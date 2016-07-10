#pragma once

#include "../Atomic.hpp"
#include "../Semaphore.hpp"
#include "../RWLock.hpp"
#include "../Thread.hpp"

#include "SchedulerMutex.hpp"
#include "SchedulerSemaphore.hpp"
#include "ExecStats.hpp"
#include "Profiling.hpp"

#include "../../Collections/Array.hpp"
#include "../../Core/Manager.hpp"

#define AX_JOB_API //no specific job API


namespace Ax { namespace Async {

	// Forward declarations
	struct SJob;
	class RJobChain;
	struct SSchedulerPriorityChain;
	struct SWorkerExecutionState;
	class LWorker;
	class LWorkerThread;
	class MScheduler;

	/// Function type for a job
	typedef void( AX_JOB_API *FnJob )( void * );
	
	/// Maximum number of simultaneous workers allowed
	static const uint32				kMaxWorkers			= sizeof( void * ) == 8 ? 64 : 32;
	/// Maximum number of chains per frame - in debug mode, going over this limit will trigger an assert
	static const uint32				kMaxChains			= 2048;
	/// Maximum number of tasks per frame - in debug mode, going over this limit will trigger an assert
	static const uint32				kMaxTasks			= 65536;

	/// Stage of the frame for the scheduler
	enum class EFramePhase
	{
		/// Not inside of a frame
		None,
		/// Just entering the frame
		Enter,
		/// Just leaving the frame
		Leave
	};

	/// Priority for a given job chain
	enum class EPriority : uint32
	{
		Lowest,
		Lower,
		Low,
		Normal,
		High,
		Higher,
		Highest
	};
	/// Number of priority levels
	static const uint32				kNumPriorityLevels = uint32( EPriority::Highest ) + 1;

	/// Individual work unit
	struct SJob
	{
		/// Function to be executed upon the task's completion
		union {
			volatile FnJob			pfnKernel;
			volatile uintptr		uKernel;
		};
		/// Data to be passed to the call to the task
		void *						pTaskData;

		/// Job that has been claimed
		static const uintptr		kClaimed = ~uintptr( 0 );
	};
	/// Collection of work
	class RJobChain
	{
	friend class LWorker;
	friend class MScheduler;
	public:
		RJobChain();
		~RJobChain();

		void AddJobs( const SJob *pJobs, uint32 cJobs );
		inline void AddJob( const SJob &Job )
		{
			AddJobs( &Job, 1 );
		}
		inline void AddJob( FnJob pfnKernel, void *pTaskData = nullptr )
		{
			const SJob Job = { { pfnKernel }, pTaskData };
			AddJobs( &Job, 1 );
		}
		template< uint32 tSize >
		inline void AddJobs( const SJob( &Jobs )[ tSize ] )
		{
			AddJobs( Jobs, tSize );
		}

		bool IsDescendant( const RJobChain &Ancestor ) const;
		void SetParent( RJobChain *pParent );
		inline void AddChild( RJobChain &Child )
		{
			Child.SetParent( this );
		}

		void SetPriority( EPriority Priority );
		inline EPriority GetPriority() const
		{
			return m_Priority;
		}

		void SetJobKernel( uint32 uIndex, FnJob pfnKernel );
		void SetJobData( uint32 uIndex, void *pTaskData );

	protected:
		void FixForSubmit();
		void ExecuteJobs( const LWorker &Worker, SWorkerExecutionState &WorkerState );

	protected:
		/// List of jobs to execute
		SJob *						m_pJobs;
		/// Number of jobs in the list
		uint32						m_cJobs;
		/// Amount of work remaining for the job chain (will be set to m_cJobs on submit)
		volatile uint32				m_cRemaining;

		/// Index of this job chain (set on initialization)
		uint32						m_uIndex;
		/// Priority of this job chain
		EPriority					m_Priority;

		/// Pointer to the ExecStats for this chain (set on submit)
		SExecStats *				m_pExecStats;

		//
		//	NOTE
		//
		//	        [ Root A ]     [ Root B ]     [ Root C ]
		//
		//	                     [ Dependency ]
		//
		//	A graph like the above (a task item depending on the results of
		//	multiple parents) is not currently possible. If this is desired then
		//	some restructuring will be necessary. In particular, we can no
		//	longer use intrusive links for management. Instead arrays of
		//	pointers would be a good alternative.
		//

		/// Parent job chain
		RJobChain *					m_pParent;
		/// Previous sibling job chain
		RJobChain *					m_pPrev;
		/// Next sibling job chain
		RJobChain *					m_pNext;
		/// First child job chain
		RJobChain *					m_pHead;
		/// Last child job chain
		RJobChain *					m_pTail;

	private:
		bool WasSubmitted() const;

		RJobChain( const RJobChain & ) = delete;
		RJobChain &operator=( const RJobChain & ) = delete;
	};

	struct SSchedulerPriorityChain
	{
		volatile uint32				cChains;
		RJobChain *					pChains				[ kMaxChains ];
	};

	/// SWorkerExecutionState: Holds information related to the execution of jobs
	struct SWorkerExecutionState
	{
		/// Information for performing lazy updates (reduce atomic contention)
		struct LazyUpdateInfo
		{
			/// Execution statistics for all of the chains
			SExecStats				DeferredStats[ kMaxChains ];
		}							LazyUpdate;

		inline SWorkerExecutionState()
		: LazyUpdate()
		{
			ResetDeferredStats();
		}

		inline void ResetDeferredStats()
		{
			ResetExecStats( LazyUpdate.DeferredStats );
		}
	};
	/// LWorker: Executes work
	class LWorker
	{
	friend class MScheduler;
	friend class LWorkerThread;
	public:
		inline uint32 GetId() const
		{
			return m_uWorkerId;
		}

		uint64 GetElapsedFrameCPUTime() const;

	protected:
		LWorker( uint32 workerId );
		~LWorker();

		bool ExecuteUntilNoWork();

	private:
		const uint32				m_uWorkerId;
		volatile EFramePhase		m_FramePhase;
		volatile bool				m_bIsWorking;
		uint64						m_HeadCPUTimestamp;
		uint64						m_TailCPUTimestamp;
		uint32						m_uCurrentChain		[ kNumPriorityLevels ];
		SWorkerExecutionState		m_ExecutionState;

		void Sleep();
	};

	/// LWorker thread
	class LWorkerThread: public virtual CThread
	{
	friend class LWorker;
	friend class MScheduler;
	public:
		inline uint32 GetWorkerId() const
		{
			return m_Worker.GetId();
		}

	protected:
		LWorkerThread( LWorker &worker );
		~LWorkerThread();

		virtual int OnRun() override;

	private:
		LWorker &					m_Worker;
	};

	/// MScheduler system: Manages worker threads and various synchronizations
	class MScheduler
	{
	friend class LWorker;
	public:
		static MScheduler &GetInstance();
		~MScheduler();

		/// Initialize to the specified number of threads
		bool Init( uint32 numThreads = 0 );
		/// Finish using the scheduler
		void Fini();

		/// Get the worker ID of the current thread (0 to numWorkers - 1)
		uint32 GetCurrentWorkerId() const;
		/// Register the current thread as the given worker ID
		void SetCurrentWorkerId( uint32 workerId );
		/// Retrieve the number of workers
		inline uint32 NumWorkers() const
		{
			return m_cWorkers;
		}

		/// Wait for all currently submitted jobs to complete
		void WaitForAllJobs();

		/// Enter a frame: Call this prior to submitting any jobs
		void EnterFrame();
		/// Leave a frame: Call this after all jobs of the frame have completed
		void LeaveFrame();

		/// Calculate execution statistics for all jobs (needed for the data
		/// pointed to by SJobChain::pExecStats to be entered)
		///
		/// @note Only call this function after LeaveFrame(), and preferably
		///       only if you actually need the execution statistics
		void CalculateExecStats();

		/// Directly submit a job chain to the scheduler
		void Submit( RJobChain *pChains, uint32 cChains = 1 );
		void Submit( RJobChain *const *ppChains, uint32 cChains );
		template< uint32 tNumChains >
		inline void Submit( RJobChain *const( &ppChains )[ tNumChains ] )
		{
			Submit( ppChains, tNumChains );
		}

	protected:
		bool IsWorking() const;

	private:
		MScheduler();

		uint32						m_cWorkers;
		LWorker *					m_pWorkers[ kMaxWorkers ];
		LWorkerThread *				m_pThreads[ kMaxWorkers ];
#if !AX_HAS_THREADLOCAL
# ifdef _WIN32
		DWORD						m_dwWorkerIdTLS;
# endif
#endif
		volatile uint32				m_cSleepingWorkers;
		CSemaphore					m_WakeSemaphore;
		volatile bool				m_bIsQuitting;
		uint64						m_HeadFrameMicrosecs;
		uint64						m_TailFrameMicrosecs;

		SSchedulerPriorityChain		m_PriorityChains	[ kNumPriorityLevels ];

		volatile uint32				m_cSubmittedChains;
		SExecStats					m_ChainStats		[ kMaxChains ];

		void WakeWorkers();
		void SubmitToPriorityChain( SSchedulerPriorityChain &PriorityChain, RJobChain &Chain );

		bool CheckNewWork( const uint32( &uCurrentChain )[ kNumPriorityLevels ] ) const;
	};
	static TManager< MScheduler >	Tasks;

}}

/// Simple scheduler wrapper
namespace Ax { namespace SchedulerCPU {

	using namespace Async;

#define AX_TASKS__ MScheduler::GetInstance()

	/// Retrieve an instance of the scheduler
	AX_FORCEINLINE MScheduler &Instance()
	{
		return AX_TASKS__;
	}

	/// Initialize the scheduler
	AX_FORCEINLINE bool Init( uint32 numThreads = 0 )
	{
		return AX_TASKS__.Init( numThreads );
	}
	/// Finish using the scheduler
	AX_FORCEINLINE void Fini()
	{
		AX_TASKS__.Fini();
	}
	/// Get the worker ID of the current thread (0 to numWorkers - 1)
	AX_FORCEINLINE uint32 GetCurrentWorkerId()
	{
		return AX_TASKS__.GetCurrentWorkerId();
	}
	/// Register the current thread as the given worker ID
	AX_FORCEINLINE void SetCurrentWorkerId( uint32 workerId )
	{
		AX_TASKS__.SetCurrentWorkerId( workerId );
	}
	/// Retrieve the number of workers
	AX_FORCEINLINE uint32 NumWorkers()
	{
		return AX_TASKS__.NumWorkers();
	}

	/// Wait for all currently submitted jobs to complete
	AX_FORCEINLINE void WaitForAllJobs()
	{
		return AX_TASKS__.WaitForAllJobs();
	}

	/// Begin the frame for the scheduler
	AX_FORCEINLINE void EnterFrame()
	{
		AX_TASKS__.EnterFrame();
	}
	/// End the frame for the scheduler
	AX_FORCEINLINE void LeaveFrame()
	{
		AX_TASKS__.LeaveFrame();
	}

	/// Calculate execution statistics for all jobs (needed for the data pointed
	/// to by SJobChain::pExecStats to be entered)
	///
	/// @note Only call this function after LeaveFrame(), and preferably only if
	///       you actually need the execution statistics
	AX_FORCEINLINE void CalculateExecStats()
	{
		AX_TASKS__.CalculateExecStats();
	}

	/// Directly submit a job chain to the scheduler
	AX_FORCEINLINE void Submit( RJobChain *pChains, uint32 cChains = 1 )
	{
		AX_TASKS__.Submit( pChains, cChains );
	}
	AX_FORCEINLINE void Submit( RJobChain *const *ppChains, uint32 cChains )
	{
		AX_TASKS__.Submit( ppChains, cChains );
	}
	template< uint32 tNumChains >
	AX_FORCEINLINE void Submit( RJobChain *const( &ppChains )[ tNumChains ] )
	{
		AX_TASKS__.Submit( ppChains, tNumChains );
	}

}}

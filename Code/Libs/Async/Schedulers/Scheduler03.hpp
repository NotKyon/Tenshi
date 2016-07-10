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

#define AX_JOB_API //no specific job API

#ifndef AX_ASYNC_PER_CHAIN_INDEX_ENABLED
# define AX_ASYNC_PER_CHAIN_INDEX_ENABLED 0
#endif
#ifndef AX_ASYNC_FRAME_STEP_JOBS_ENABLED
# define AX_ASYNC_FRAME_STEP_JOBS_ENABLED 0
#endif
#ifndef AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
# define AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED 0
#endif
#ifndef AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED
# define AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED 0 //INCOMPLETE (NOT NEEDED)
#endif
#ifndef AX_ASYNC_LOCAL_WORK_ENABLED
# define AX_ASYNC_LOCAL_WORK_ENABLED 0 //INCOMPLETE
#endif
#ifndef AX_ASYNC_PER_ARENA_LOCK_ENABLED
# define AX_ASYNC_PER_ARENA_LOCK_ENABLED 0
#endif
#ifndef AX_ASYNC_STEALING_ENABLED
# define AX_ASYNC_STEALING_ENABLED 1
#endif
#ifndef AX_ASYNC_ARENA_WORK_STEALING_ENABLED
# define AX_ASYNC_ARENA_WORK_STEALING_ENABLED AX_ASYNC_STEALING_ENABLED
#endif

namespace Ax { namespace Async {

	// Forward declarations
	struct SJob;
	struct SJobChain;
	struct SWorkerExecutionState;
#if AX_ASYNC_LOCAL_WORK_ENABLED
	struct SArena;
#endif
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

	/// Individual work unit
	struct SJob
	{
		/// Function to be executed upon the task's completion
		FnJob						pfnKernel;
		/// Data to be passed to the call to the task
		void *						pTaskData;
		/// SJob chain to be signalled for dispatching additional work
		SJobChain *					pDispatch;

		/// Validate this job
		inline void Validate() const
		{
			AX_ASSERT( pfnKernel != nullptr );
		}

		/// Execute this job
		///
		/// @return Amount of CPU-time (timestamps) it took to execute the job
		uint64 Execute( SWorkerExecutionState &WorkerState ) const;
	};
	/// Collection of work
	struct SJobChain
	{
		/// TList of jobs to execute
		SJob *						pJobs;
		/// Number of jobs in the list
		uint32						cJobs;
		/// Current number of blockers on the chain (must be nonzero for submit)
		volatile uint32				cBlockers;
#if AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED
		/// Current number of workers that have access to the chain (set on submit)
		volatile uint32				cWorkers;
#endif
		/// Pointer to the ExecStats for this chain (set on submit)
		SExecStats *				pExecStats;
#if AX_ASYNC_PER_CHAIN_INDEX_ENABLED
		AX_CACHEALIGN
		volatile uint32				uReadIndex;
#endif

		/// Validate this job chain
		inline void Validate() const
		{
			AX_ASSERT( pJobs != nullptr );
			AX_ASSERT( cJobs > 0 );
			for( uint32 i = 0; i < cJobs; ++i ) {
				AX_STATIC_SUPPRESS( 6011 )
				pJobs[ i ].Validate();
			}
		}

#if AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED
		/// Check whether the chain is in use
		inline bool InUse() const
		{
			return cWorkers > 0;
		}
#endif
	};

#if AX_ASYNC_LOCAL_WORK_ENABLED
	static const uint32				kMaxArenaTasks = 32;
	static const uint32				kNumArenas = kMaxTasks/kMaxArenaTasks;
	struct SArena
	{
		struct STask
		{
			SJob *					pJob;
			SJobChain *				pChain;
		};

#if AX_ASYNC_PER_ARENA_LOCK_ENABLED
		LWorker *					pLockedWorker;
#endif
		uint32						uTaskReadIndex;
		uint32						uTaskWriteIndex;
		STask						Tasks[ kMaxArenaTasks ];

#if AX_ASYNC_PER_ARENA_LOCK_ENABLED
		inline bool TryLockWorker( LWorker &Self )
		{
			return AtomicSetPtrEq( &pLockedWorker, &Self, ( LWorker * )0 ) == &Self;
		}
		inline void UnlockWorker( LWorker &Self )
		{
			AX_ASSERT_MSG( pLockedWorker == &Self, "Lock not held by unlocking worker" );
			AtomicSetPtr( &pLockedWorker, nullptr );
		}
#endif

		inline STask *ReadTask()
		{
			// Determine where to read from
			const uint32 uReadIndex = ( uTaskReadIndex++ )%kMaxArenaTasks;
			if( uReadIndex == uTaskWriteIndex%kMaxArenaTasks ) {
				// No task available
				return nullptr;
			}

			// Retrieve the task
			return &Tasks[ uReadIndex ];
		}
		/// Writes a set of jobs from the job chain to this arena
		///
		/// Returns the number of jobs written
		inline uint32 WriteJobsFromChain( uint32 &uOffset, uint32 &cJobs, SJobChain &Chain )
		{
			AX_ASSERT( Chain.pJobs != nullptr );
			AX_ASSERT( Chain.cJobs > 0 );

			const uint32 cMaxJobs = Chain.cJobs;
			const uint32 cChainJobs = uOffset + cJobs > cMaxJobs ? cMaxJobs - uOffset : cJobs;
			const uint32 cQueuedJobs = uTaskWriteIndex - uTaskReadIndex;
			const uint32 cFreeSlots = kMaxArenaTasks - cQueuedJobs;

			const uint32 cIterations = cFreeSlots < cChainJobs ? cFreeSlots : cChainJobs;

			for( uint32 i = 0; i < cIterations; ++i ) {
				const uint32 uJobIndex = uOffset + i;
				const uint32 uWriteIndex = uTaskWriteIndex % kMaxArenaTasks;

				Tasks[ uWriteIndex ].pJob = &Chain.pJobs[ uOffset + i ];
				Tasks[ uWriteIndex ].pChain = &Chain;

				AX_MEMORY_BARRIER();

				++uTaskWriteIndex;
			}

			uOffset += cIterations;
			cJobs -= cIterations;

			return cIterations;
		}
	};
#endif

	/// SWorkerExecutionState: Holds information related to the execution of jobs
	struct SWorkerExecutionState
	{
		/// Information for performing lazy updates (reduce atomic contention)
		struct LazyUpdateInfo
		{
			/// Last SJob::pDispatch member found
			SJobChain *				pDispatch;
			/// Number of unblock operations to apply to pDispatch
			uint32					cDispatchUnblocks;

			/// Last SJobChain processed
			uint32					uLastJobChain;
			/// Minimum amount of CPU time any job from the last SJobChain took
			uint64					LastJobChainMinJobCPUTime;
			/// Maximum amount of CPU time any job from the last SJobChain took
			uint64					LastJobChainMaxJobCPUTime;
			/// Total amount of CPU time all jobs took
			uint64					LastJobChainTotalCPUTime;

			AX_CACHEALIGN
			/// Execution statistics for all of the chains
			SExecStats				DeferredStats[ kMaxChains ];

			inline LazyUpdateInfo()
			: pDispatch( nullptr )
			, cDispatchUnblocks( 0 )
			, uLastJobChain( uint32( -1 ) )
			, LastJobChainMinJobCPUTime( uint64( -1 ) )
			, LastJobChainMaxJobCPUTime( 0 )
			, LastJobChainTotalCPUTime( 0 )
			{
			}

			inline void AddJobCPUTime( uint64 jobCPUTime )
			{
				if( LastJobChainMinJobCPUTime > jobCPUTime ) {
					LastJobChainMinJobCPUTime = jobCPUTime;
				}
				if( LastJobChainMaxJobCPUTime < jobCPUTime ) {
					LastJobChainMaxJobCPUTime = jobCPUTime;
				}
				LastJobChainTotalCPUTime += jobCPUTime;
			}

			/// Update the "dispatch" members
			void CommitDispatch();
			/// Update the "last job chain" members
			void CommitLastJobChain();
			/// Update all members
			void Commit();
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

#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
	enum class EFramePhase
	{
		None,
		Enter,
		Leave
	};
#endif

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
		LWorker( MScheduler &scheduler, uint32 workerId );
		~LWorker();

#if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		void SubmitEnterFrameJob( SJobChain &chain );
		void SubmitLeaveFrameJob( SJobChain &chain );
#endif

		bool ExecuteUntilNoWork();

	private:
		MScheduler &				m_Scheduler;
		const uint32				m_uWorkerId;
#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		volatile EFramePhase		m_FramePhase;
#endif
#if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		volatile bool				m_bIsWorking;
#endif
		uint64						m_HeadCPUTimestamp;
		uint64						m_TailCPUTimestamp;
#if AX_ASYNC_LOCAL_WORK_ENABLED
		uint32						m_uArenaIndex;
		SArena *					m_pArena;
#else
		uint32						m_uChainIndex;
# if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
		uint32						m_uLocalIndex;
# endif
#endif
		SWorkerExecutionState		m_ExecutionState;

#if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		static void AX_JOB_API ReadCPUTimestampJob( void * );
#endif

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
		void Submit( SJobChain *pChains, uint32 cChains = 1 );
		void Submit( SJobChain *const *ppChains, uint32 cChains );
		template< uint32 tNumChains >
		inline void Submit( SJobChain *const( &ppChains )[ tNumChains ] )
		{
			Submit( ppChains, tNumChains );
		}

		/// Directly allocate tasks [low-level]
		SJob *AllocTasks( uint32 count );
		/// Directly allocate chains [low-level]
		SJobChain *AllocChains( uint32 count );

	protected:
#if !AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		void IncIdleWorker();
		void DecIdleWorker();
#endif
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

		uint32						m_cFrameChains;
		uint32						m_cFrameTasks;

		AX_CACHEALIGN
		SJobChain					m_FrameChains[ kMaxChains ];
		SJob						m_FrameTasks[ kMaxTasks ];

#if AX_ASYNC_LOCAL_WORK_ENABLED
		AX_CACHEALIGN 
		SArena						m_Arenas[ kNumArenas ];

# if !AX_ASYNC_PER_ARENA_LOCK_ENABLED
		AX_CACHEALIGN
		SArena *					m_pArenas[ kNumArenas ];

		static AX_FORCEINLINE uint32 DecodeArenaIndex( uint32 uIndex )
		{
			//return uIndex*AX_POINTERS_PER_CACHELINE + uIndex/AX_POINTERS_PER_CACHELINE;
			
			//return ( ( uIndex & 0x00FF00FF )<<8 ) | ( ( uIndex & 0xFF00FF00 )>>8 );
			//return ( ( uIndex & 0xC71C71C7 )<<3 ) | ( ( uIndex & 0x38E38E38 )>>3 );
			return ( ( uIndex & 0x0F0F0F0F )<<4 ) | ( ( uIndex & 0xF0F0F0F0 )>>4 );
		}
		static AX_FORCEINLINE uint32 EncodeArenaIndex( uint32 uIndex )
		{
			return DecodeArenaIndex( uIndex );
		}
		AX_FORCEINLINE SArena *&GetArena( uint32 uIndex )
		{
			const uint32 uDecodedIndex = DecodeArenaIndex( uIndex );
			return m_pArenas[ uDecodedIndex%kNumArenas ];
		}
# else
		static AX_FORCEINLINE uint32 DecodeArenaIndex( uint32 index )
		{
			return index;
		}
		AX_FORCEINLINE SArena *GetArena( uint32 uIndex )
		{
			return &m_Arenas[ uIndex%kNumArenas ];
		}
# endif
#else
		AX_CACHEALIGN
		SJobChain *					m_pSubmittedChains[ kMaxChains ];
		volatile uint32				m_cSubmittedChains;
#endif

		AX_CACHEALIGN
		SchedulerMutex				m_SubmitWriteAccessor;
#if !AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		volatile uint32				m_cIdleWorkers;
#endif

		SExecStats					m_ChainStats[ kMaxChains ];

		void WakeWorkers();
	};
	inline MScheduler &TASKS()
	{
		return MScheduler::GetInstance();
	}
	
	/*
	===========================================================================

		INLINE IMPLEMENTATIONS

	===========================================================================
	*/

	// Execute a job
	//
	// Returns the amount of CPU time the job took to execute (RDTSC)
	AX_FORCEINLINE uint64 SJob::Execute( SWorkerExecutionState &WorkerState ) const
	{
		const uint64 baseCPUTime = GetCPUCycles();
		pfnKernel( pTaskData );
		const uint64 elapsedCPUTime = GetCPUCycles() - baseCPUTime;

		if( pDispatch != WorkerState.LazyUpdate.pDispatch ) {
			WorkerState.LazyUpdate.CommitDispatch();
		}

		if( !pDispatch ) {
			return elapsedCPUTime;
		}

		WorkerState.LazyUpdate.pDispatch = pDispatch;
		++WorkerState.LazyUpdate.cDispatchUnblocks;

		return elapsedCPUTime;
	}

	// Perform the lazy update of the dispatch
	inline void SWorkerExecutionState::LazyUpdateInfo::CommitDispatch()
	{
		if( !pDispatch ) {
			return;
		}

		SJobChain *const pToDispatch = AtomicSub( &pDispatch->cBlockers, cDispatchUnblocks ) == cDispatchUnblocks ? pDispatch : nullptr;

		cDispatchUnblocks = 0;
		pDispatch = nullptr;

		AX_MEMORY_BARRIER();

		if( pToDispatch != nullptr ) {
			MScheduler::GetInstance().Submit( pToDispatch );
		}
	}
	// Perform the lazy update of the last job chain CPU time
	inline void SWorkerExecutionState::LazyUpdateInfo::CommitLastJobChain()
	{
		AX_ASSERT( uLastJobChain < kMaxChains );

		DeferredStats[ uLastJobChain ].MinJobTime = LastJobChainMinJobCPUTime;
		DeferredStats[ uLastJobChain ].MaxJobTime = LastJobChainMaxJobCPUTime;
		DeferredStats[ uLastJobChain ].TotalJobTime = LastJobChainTotalCPUTime;
		DeferredStats[ uLastJobChain ].TotalRunTime = 0;
	}

	inline void SWorkerExecutionState::LazyUpdateInfo::Commit()
	{
		CommitDispatch();
		CommitLastJobChain();
	}

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
	AX_FORCEINLINE void Submit( SJobChain *pChains, uint32 cChains = 1 )
	{
		AX_TASKS__.Submit( pChains, cChains );
	}
	AX_FORCEINLINE void Submit( SJobChain *const *ppChains, uint32 cChains )
	{
		AX_TASKS__.Submit( ppChains, cChains );
	}
	template< uint32 tNumChains >
	AX_FORCEINLINE void Submit( SJobChain *const( &ppChains )[ tNumChains ] )
	{
		AX_TASKS__.Submit( ppChains, tNumChains );
	}

	/// Directly allocate tasks [low-level]
	AX_FORCEINLINE SJob *AllocTasks( uint32 count )
	{
		return AX_TASKS__.AllocTasks( count );
	}
	/// Directly allocate chains [low-level]
	AX_FORCEINLINE SJobChain *AllocChains( uint32 count )
	{
		return AX_TASKS__.AllocChains( count );
	}

#undef AX_TASKS__

}}

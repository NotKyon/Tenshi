#pragma once

#include "../Thread.hpp"
#include "../Atomic.hpp"
#include "../../Core/Types.hpp"
#include "../../Collections/Array.hpp"

#ifdef _WIN32
# define AX_JOB_API __fastcall
#else
# define AX_JOB_API
#endif

namespace Ax { namespace Async {

	typedef void( AX_JOB_API *kernel_t )( void * );

	class MScheduler;
	class TaskQueue;
	class TaskNode;
	class LWorkerThread;


	/*!
	 *	SJob
	 *
	 *	Individual unit of work
	 */
	struct SJob
	{
		kernel_t					pfnKernel;
		void *						pData;

		inline SJob( kernel_t func = nullptr, void *parm = nullptr )
		: pfnKernel( func )
		, pData( parm )
		{
		}
		inline SJob( const SJob &x )
		: pfnKernel( x.pfnKernel )
		, pData( x.pData )
		{
		}

		inline SJob &operator=( const SJob &x )
		{
			pfnKernel = x.pfnKernel;
			pData = x.pData;

			return *this;
		}
	};

	/*!
	 *	TaskQueue
	 *
	 *	A collection of jobs and synchronization signals and points which define
	 *	how a set of tasks should be carried out.
	 */
	class TaskQueue
	{
	friend class LWorkerThread;
	friend class MScheduler;
	public:
		// Indicates the status of the TaskQueue's life
		enum state_t
		{
			// Ready for jobs to be added to it
			kReady,
			// It's been submitted to the scheduler
			kQueued,
			// All tasks in the queue have completed execution
			kDone,
			// Mission aborted: application might be terminating or we were taking too long and became irrelevant
			kAborted
		};

		/*!
		 *	How quickly the TaskQueue should execute
		 *
		 *	Higher priorities will be completed before lower priorities
		 */
		enum class priority_t : int
		{
			kVeryLow = -2,
			kLow = -1,
			kNormal = 0,
			kHigh = 1,
			kVeryHigh = 2
		};

		/*!
		 *	Completion latency for this TaskQueue
		 */
		enum class Latency
		{
			/*! No automated limit on completion */
			Unlimited,
			/*! Task queue needs to complete on this frame */
			ThisFrame,
			/*! Task queue needs to complete on the next frame */
			NextFrame,

			/*! Default task queue latency */
			Default = Unlimited
		};

		// Statistics
		struct stats_t
		{
			// Minimum amount of time any individual job took (microseconds)
			uint64						uMinJobTime;
			// Maximum amount of time any individual job took (microseconds)
			uint64						uMaxJobTime;
			// Total amount of time spent executing jobs (microseconds)
			uint64						uTotalJobTime;
			// Total amount of time spent executing the task queue (microseconds)
			uint64						uTotalExecTime;
			// Number of times a job could not be executed because it was depending on something else
			uint32						cStalls;

			// Constructor
			inline stats_t()
			{
				Reset();
			}
			// Reset to the defaults
			inline void Reset()
			{
				uMinJobTime = ( uint64 )-1;
				uMaxJobTime = 0;
				uTotalJobTime = 0;
				uTotalExecTime = 0;
				cStalls = 0;
			}
			// Merge another set of statistics into this one
			inline void Merge( const stats_t &other )
			{
				//Lock();

				if( uMinJobTime > other.uMinJobTime ) {
					uMinJobTime = other.uMinJobTime;
				}
				if( uMaxJobTime < other.uMaxJobTime ) {
					uMaxJobTime = other.uMaxJobTime;
				}

				uTotalJobTime += other.uTotalJobTime;
				if( uTotalExecTime < other.uTotalExecTime ) {
					uTotalExecTime = other.uTotalExecTime;
				}
				cStalls += other.cStalls;

				//Unlock();
			}
			// Output debug information
			void OutputDebugInfo( const char *pszName = nullptr ) const;

		private:
			uint32						uMergeLock = 0;

			inline void Lock()
			{
				while( AtomicInc( &uMergeLock ) != 0 ) {
					AtomicDec( &uMergeLock );

					for( volatile uint32 spin = 0; spin < 512; ++spin ) {
					}
				}
			}
			inline void Unlock()
			{
				AtomicDec( &uMergeLock );
			}
		};

		/*!
		 *	Constructor
		 */
		TaskQueue();
		/*!
		 *	Add a reference
		 */
		inline void AddRef()
		{
			++m_uRefCnt;
		}
		/*!
		 *	Remove a reference
		 */
		inline void Release()
		{
			if( --m_uRefCnt == 0 ) {
				delete this;
			}
		}

		/*!
		 *	Adds a set of jobs to the queue.
		 *
		 *	All of the jobs added will depend on the currently set fence. (See
		 *	AddFence().) Additionally, they will also notify the current signal
		 *	upon having completed their execution. (See AddSignal().)
		 */
		void AddJobs( const SJob *jobs, uint32 numJobs );
		template< uint32 N >
		inline void AddJobs( const SJob( &jobs )[ N ] )
		{
			AddJobs( jobs, N );
		}
		inline void AddJob( const SJob &job )
		{
			AddJobs( &job, 1 );
		}
		inline void AddJob( kernel_t kernel, void *data = nullptr )
		{
			const SJob job = { kernel, data };
			AddJobs( &job, 1 );
		}
		/*!
		 *	Adds a signal to the queue.
		 *
		 *	When a signal is added, all of the jobs following it (up until the
		 *	end of the queue or the next signal) will trigger the signal. The
		 *	signal will only be "on" once all of the prior jobs have completed
		 *	execution.
		 *
		 *	Returns the new signal's index. (This can be zero.)
		 */
		uint32 AddSignal();
		/*!
		 *	Adds a fence to the queue.
		 *
		 *	Fences will block execution of this queue until the given signal is
		 *	set to the "on" state. All items after the fence will depend on this
		 *	fence (and therefore all prior items for the given signalIndex).
		 *
		 *	It is an error to add more jobs to the queue after a fence has been
		 *	added unless you add another signal. (Doing so will trigger an
		 *	internal assertion in debug mode.)
		 *
		 *	A fence will not be added if the given signal has no items.
		 *
		 *	If signalIndex is equal to -1 (0xFFFFFFFF) then the current signal
		 *	is used.
		 */
		void AddFence( uint32 signalIndex = ( uint32 )-1 );
		/*!
		 *	Submit this queue to the scheduler.
		 */
		void Submit( Latency latency = Latency::Default );
		/*!
		 *	Wait on this queue until it completes execution.
		 *
		 *	Returns true if the queue completed execution and false if it was
		 *	aborted.
		 */
		bool Wait();
		/*!
		 *	Abort execution of this queue.
		 */
		void Abort();

		/*!
		 *	Set the priority of the queue.
		 *
		 *	Cannot change after queue has been submitted.
		 */
		void SetPriority( priority_t priority );
		/*!
		 *	Retrieve the priority of the queue.
		 */
		priority_t GetPriority() const;

		/*!
		 *	Retrieve the current state of the queue.
		 */
		state_t GetState() const;
		/*!
		 *	Retrieve the statistics of the queue.
		 */
		const stats_t &GetStats() const;

		/*!
		 *	Display debug information about the queue
		 */
		void OutputDebugInfo( const char *pszQueueName = nullptr ) const;

		/*!
		 *	Reset the queue to its default state
		 *
		 *	NOTE: The queue must not be currently submitted
		 */
		void Reset();

	protected:
		enum executeResult_t
		{
			// Jobs were executed successfully
			kExecSuccess,
			// No jobs were able to be executed due to being blocked
			kExecBlocked,
			// This job list can now be removed from the local set
			kExecRemoved
		};

		/*!
		 *	Execute a set of tasks from this queue.
		 *
		 *	tempStats: Will be updated with information from this execution
		 *	-          session. This will not be updated in a thread-safe way.
		 *
		 *	Returns current state of the queue. This will be one of:
		 *	- kBlocked
		 *	- kDone
		 *	- kAborted
		 */
		executeResult_t ExecuteWork( stats_t &tempStats );
		/*!
		 *	Merge execution stats into the queue
		 */
		void MergeStats( const stats_t &stats );

	private:
		// Actual work to be done in the job queue
		TArray< SJob >			m_Jobs;
		// The signal index for each of the jobs in the job queue
		TArray< uint8 >			m_JobSignalIndexes;
		// Current signal levels for fences
		//
		// These are set to the number of dependencies upon submission. Each job
		// completed will decrement its corresponding signal. A "fence" will
		// block until the signal reaches zero.
		TArray< uint32 >			m_Signals;
		// Current reference count
		uint32							m_uRefCnt;
		// Current state of the job list (see state_t)
		state_t						m_State;
		// Priority of this job queue
		priority_t					m_Priority;
		// Stats for the queue
		stats_t						m_Stats;
		// Time at which this task queue was submitted (microseconds)
		uint64							m_uSubmitTime;
		// Current job index (for execution)
		uint32							m_uJobReadIndex;
		// Lock for reading a job from the list
		uint32							m_uJobReadLock;
		// Current number of accessors (worker threads)
		uint32							m_uNumAccessors;
		
		/*!
		 *	Destructor
		 */
		~TaskQueue();
		/*!
		 *	Retrieve the last job added to the list (or NULL if no job added)
		 */
		const SJob *LastJob() const;

		/*!
		 *	Special job indicating a fence
		 */
		static void AX_JOB_API Fence_f( void * );
	};

	/*!
	 *	TaskSignal
	 *
	 *	Releases a task from the task scheduler once the signal is reached.
	 */
	class TaskSignal
	{
	public:

	private:
		uint32							m_uCount;
		TaskQueue *					m_pQueue;
	};

	/*!
	 *	MScheduler
	 *
	 *	Manages the execution of various task queues
	 */
	class MScheduler
	{
	public:
		static const uint32 kMaxQueues = 1024;
		static const uint32 kMaxThreads = sizeof( void * ) == 8 ? 64 : 32;

		static MScheduler &GetInstance();

		~MScheduler();

		bool Init( uint32 numThreads = 0 );
		void Fini();

		bool SubmitQueue( TaskQueue *queue, TaskQueue::Latency latency );
		void StepFrame();

		inline uint32 GetThreadCount() const
		{
			return m_cThreads;
		}

	private:
		static const uint32 kNumBuffers = 4;

		LWorkerThread *				m_pThreads[ kMaxThreads ];
		uint32							m_cThreads;
		uint32							m_uFrame;

		AX_CACHEALIGN TaskQueue *	m_pWaiters[ kMaxQueues*kNumBuffers ];
		uint32							m_cWaiters[ kMaxQueues*kNumBuffers ];

		MScheduler();
	};

	inline MScheduler &TASKS()
	{
		return MScheduler::GetInstance();
	}

	/*!
	 *	LWorkerThread
	 *
	 *	Executes a set of task queues from its local list
	 */
	class LWorkerThread: public virtual CThread
	{
	public:
		static const uint32 kMaxQueues = MScheduler::kMaxQueues;

		LWorkerThread( MScheduler &scheduler );
		virtual ~LWorkerThread();

		void Abort();
		bool SubmitQueue( TaskQueue *queue );

	protected:
		virtual int OnRun();

	private:
		MScheduler &					m_Scheduler;
		AX_CACHEALIGN TaskQueue *	m_pQueues[ kMaxQueues ];
		uint32							m_uBaseQueue;
		uint32							m_uLastQueue;
		uint32							m_uWriteLock;

		void AcquireWriteLock();
		void ReleaseWriteLock();
	};

}}

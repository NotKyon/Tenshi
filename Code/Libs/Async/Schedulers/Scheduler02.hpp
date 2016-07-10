#pragma once

#define AX_SCHEDULER_WIP 1
#define AX_JOB_API /*no special calling convention*/

#include "../Atomic.hpp"
#include "../Barrier.hpp"
#include "../CPU.hpp"
#include "../Mutex.hpp"
#include "../QuickMutex.hpp"
#include "../RWLock.hpp"
#include "../Semaphore.hpp"
#include "../Signal.hpp"
#include "../Thread.hpp"

#include "../../Core/Assert.hpp"
#include "../../Core/Types.hpp"
#include "../../Core/Logger.hpp"
#include "../../Core/String.hpp"

#include "../../Platform/Platform.hpp"

#include "../../System/HighPerformanceClock.hpp"
#include "../../System/TimeConversion.hpp"

//
//	NOTE
//
//	- It's unnecessary to remove a queue from the scheduler's list of waiters
//	. because it will only ever be referenced from there when it's being
//	. submitted. The list of waiters is also reset at the beginning of each
//	. frame, so it won't flood.
//

/// @def	AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
/// Each local queue stores its own private index for each of the work queues.
/// This reduces contention considerably but might also contribute to an
/// unbalanced work-load. Enabled by default.
#ifndef AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
# define AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED 1
#endif

namespace Ax { namespace Async {

	typedef void( *FnJob )( void * );
	
	struct Task;
	struct TaskChain;
	struct SchedulerState;
	struct WorkerState;

	class LWorkerThread;

	/// @def AX_EXECSTATS_LOCK_ENABLED
	/// Specifies whether a lock is to be used when updating the SExecStats.
	/// Typically this shouldn't be necessary as it's done in a serialized
	/// manner at the end of the frame (from a deferred set per thread).
#ifndef AX_EXECSTATS_LOCK_ENABLED
# define AX_EXECSTATS_LOCK_ENABLED 0
#endif
	
	struct SExecStats
	{
		uint64							MinJobTime;
		uint64							MaxJobTime;
		uint64							TotalJobTime;
		uint64							TotalRunTime;
#if AX_EXECSTATS_LOCK_ENABLED
		CQuickMutex					MyLock;
#endif
		uint64							ReservedA[ 4 ];
		
		inline SExecStats()
		{
			Reset();
		}
	
		inline void Reset()
		{
#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Acquire();
#endif
			MinJobTime = uint64( -1 );
			MaxJobTime = 0;
			TotalJobTime = 0;
			TotalRunTime = 0;
#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Release();
#endif
		}
		inline void AddJobTime( uint64 microseconds )
		{
			if( !microseconds ) {
				microseconds = 1;
			}

#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Acquire();
#endif
			if( MinJobTime > microseconds ) {
				MinJobTime = microseconds;
			}
			if( MaxJobTime < microseconds ) {
				MaxJobTime = microseconds;
			}

			TotalJobTime += microseconds;
#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Release();
#endif
		}
		inline SExecStats &Merge( const SExecStats &other )
		{
#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Acquire();
#endif
			if( MinJobTime > other.MinJobTime ) {
				MinJobTime = other.MinJobTime;
			}
			if( MaxJobTime < other.MaxJobTime ) {
				MaxJobTime = other.MaxJobTime;
			}

			TotalJobTime += other.TotalJobTime;
			if( TotalRunTime < other.TotalRunTime ) {
				TotalRunTime = other.TotalRunTime;
			}
#if AX_EXECSTATS_LOCK_ENABLED
			MyLock.Release();
#endif

			return *this;
		}
		inline void OutputDebugInfo( const char *pszName = nullptr ) const
		{
			String s;
		
			s.Reserve( 512 );
			s = "SExecStats";

			if( pszName != nullptr && *pszName != '\0' ) {
				s += " <";
				s += pszName;
				s += ">\n";
			} else {
				s += ":\n";
			}

			const double minJobTimeSec = System::MicrosecondsToSeconds( MinJobTime );
			const double minJobTimeFrm = System::MicrosecondsToFrames( MinJobTime );

			const double maxJobTimeSec = System::MicrosecondsToSeconds( MaxJobTime );
			const double maxJobTimeFrm = System::MicrosecondsToFrames( MaxJobTime );

			const double totalJobTimeSec = System::MicrosecondsToSeconds( TotalJobTime );
			const double totalJobTimeFrm = System::MicrosecondsToFrames( TotalJobTime );

			const double totalRunTimeSec = System::MicrosecondsToSeconds( TotalRunTime );
			const double totalRunTimeFrm = System::MicrosecondsToFrames( TotalRunTime );

			s.AppendFormat( "  minJobTime   = %.6f sec [%.3f%%]\n", minJobTimeSec, minJobTimeFrm*100.0 );
			s.AppendFormat( "  maxJobTime   = %.6f sec [%.3f%%]\n", maxJobTimeSec, maxJobTimeFrm*100.0 );
			s.AppendFormat( "  totalJobTime = %.6f sec [%.3f%%]\n", totalJobTimeSec, totalJobTimeFrm*100.0 );
			s.AppendFormat( "  totalRunTime = %.6f sec [%.3f%%]\n", totalRunTimeSec, totalRunTimeFrm*100.0 );

			if( s.EndsWith( "\n" ) ) {
				s.Remove( -1, 1 );
			}
			BasicStatusf( "%s", s.CString() );
		}
	};

	// Individual unit of work
	struct Task
	{
		FnJob						pfnKernel;
		void *						pTaskData;
		struct TaskChain *			pDepender;
		volatile uint32 *				pDecrease;

		void Execute( WorkerState &worker, SExecStats *pExecStats = nullptr );
	};

	/// @def	AX_NUGGETCHAIN_DEPENDENCIES_ENABLED
	/// Stores a unique set of dependencies for each task chain.
	///
	/// Support for this is INCOMPLETE. Disabled by default.
#ifndef AX_NUGGETCHAIN_DEPENDENCIES_ENABLED
# define AX_NUGGETCHAIN_DEPENDENCIES_ENABLED 0
#endif
	// Collection of nuggets that can be run in parallel
	struct TaskChain
	{
		static const uint32			kMaxDependencies = 16;

		volatile uint32				cBlocked;
		volatile uint32				cAccessors;

#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
		AX_CACHEALIGN
		volatile uint32				uReadIndex;
#else
		volatile bool				bCompleted;
#endif
		uint64							uSubmitTime;

		AX_CACHEALIGN
		Task *						pTasks;
		uint32							cTasks;

		// Execution statistics: Only available after EndFrame()
		SExecStats					Stats;

#if AX_NUGGETCHAIN_DEPENDENCIES_ENABLED
		// TList of unique dependencies
		TaskChain *					pDependencies[ kMaxDependencies ];
		uint32							cDependencies;
#endif

		inline void Reset()
		{
			cBlocked = 0;
#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			uReadIndex = 0;
#else
			bCompleted = false;
#endif
			cAccessors = 0;
			pTasks = nullptr;
			cTasks = 0;
			Stats.Reset();
#if AX_NUGGETCHAIN_DEPENDENCIES_ENABLED
			cDependencies = 0;
#endif
		}
#if AX_NUGGETCHAIN_DEPENDENCIES_ENABLED
		inline void AddDependency( TaskChain *pDependency )
		{
			AX_ASSERT_NOT_NULL( pDependency );
			AX_ASSERT( cDependencies < kMaxDependencies );

			pDependencies[ cDependencies++ ] = pDependency;
		}
#endif

		inline bool InUse() const
		{
			return cAccessors != 0;
		}
		inline bool DidAbort() const
		{
#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			return uReadIndex < cTasks && cAccessors == 0;
#else
			return !bCompleted && cAccessors == 0;
#endif
		}
		inline bool DidComplete() const
		{
#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			return uReadIndex >= cTasks && cAccessors == 0;
#else
			return bCompleted && cAccessors == 0;
#endif
		}
		inline void DecrementAccessor()
		{
			if( AtomicDec( &cAccessors ) == 1 ) {
				Stats.TotalRunTime = System::Microseconds() - uSubmitTime;
			}
		}

		// Returns 'true' if the task queue completed successfully, or 'false' if aborted
		inline bool Wait() const
		{
			while( InUse() ) {
				//LocalSpin();
				CThread::Yield();
			}

#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			return uReadIndex >= cTasks;
#else
			return bCompleted;
#endif
		}

#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
		inline Task *Read( uint32 amount = 1, uint32 *pNumRead = nullptr )
		{
			AX_ASSERT( amount > 0 );
			//AX_ASSERT( cBlocked == 0 );

			const uint32 baseIndex = AtomicAdd( &uReadIndex, amount );
			if( baseIndex >= cTasks ) {
				return nullptr;
			}

			if( pNumRead != nullptr ) {
				const uint32 lastIndex = baseIndex + amount;
				*pNumRead = cTasks < lastIndex ? cTasks - baseIndex : amount;
			}

			return &pTasks[ baseIndex ];
		}
#endif
	};
	
	/// Maximum number of simultaneous workers allowed
	static const uint32				kMaxWorkers			= sizeof( void * ) == 8 ? 64 : 32;
	/// Maximum number of queues per frame - in debug mode, going over this limit will trigger an assert
	static const uint32				kMaxQueues			= 2048;
	/// Maximum number of tasks per frame - in debug mode, going over this limit will trigger an assert
	static const uint32				kMaxTasks			= 65536;

	struct SchedulerState
	{
		WorkerState *				pWorkers[ kMaxWorkers ];
		uint32							cWorkers;
		bool						bCollectStats;

		// Memory backing for the individual jobs
		Task						Tasks[ kMaxTasks ];
		volatile uint32				cTasks;

		// Memory backing for each queue of jobs
		TaskChain					Queues[ kMaxQueues ];
		volatile uint32				cQueues;

		// TList of general signals (not specific to a TaskChain)
		AX_CACHEALIGN
		volatile uint32				uTaskCompletion[ kMaxTasks ];
		volatile uint32				cTaskCompletion;

		AX_CACHEALIGN
		CQuickMutex					SubmitLock;

		inline SchedulerState()
		: cWorkers( 0 )
		, bCollectStats( !!AX_DEBUG_ENABLED )
		, cTasks( 0 )
		, cQueues( 0 )
		, cTaskCompletion( 0 )
		, SubmitLock()
		{
			bCollectStats = true; //TEMPORARY!!!
		}

		inline Task *AllocTasks( uint32 amount )
		{
			AX_ASSERT( amount > 0 );

			const uint32 baseIndex = AtomicAdd( &cTasks, amount );
			AX_ASSERT( baseIndex + amount <= kMaxTasks );

			return &Tasks[ baseIndex ];
		}
		inline TaskChain *AllocChains( uint32 amount )
		{
			AX_ASSERT( amount > 0 );

			const uint32 baseIndex = AtomicAdd( &cQueues, amount );
			AX_ASSERT( baseIndex + amount <= kMaxTasks );

			return &Queues[ baseIndex ];
		}
		inline volatile uint32 *AllocCompletion()
		{
			const uint32 index = AtomicInc( &cTaskCompletion );
			AX_ASSERT( index < cTaskCompletion );

			return &uTaskCompletion[ index ];
		}

		inline void BeginFrame()
		{
			cTasks = 0;
			cQueues = 0;
			cTaskCompletion = 0;
		}
		void EndFrame();

		void SubmitNoLock( TaskChain *const *ppQueues, uint32 cQueues );
		inline bool TrySubmit( TaskChain *const *ppQueues, uint32 cQueues )
		{
			if( !SubmitLock.TryAcquire() ) {
				return false;
			}

			SubmitNoLock( ppQueues, cQueues );
			SubmitLock.Release();

			return true;
		}
		inline void Submit( TaskChain *const *ppQueues, uint32 cQueues )
		{
			while( !TrySubmit( ppQueues, cQueues ) ) {
				LocalSpin();
			}
		}
	};
	/// @def	AX_TRANSFER_QUEUES_ENABLED
	/// Transfer queues are a deferred (per-thread) set of queues that are to be
	/// submitted through the scheduler.
	///
	/// They were intended to be used to reduce contention for the main queue.
	/// However, they impeded parallelism significantly. Therefore, they are
	/// disabled by default.
#ifndef AX_TRANSFER_QUEUES_ENABLED
# define AX_TRANSFER_QUEUES_ENABLED 0
#endif
	struct WorkerState
	{
		// Local queues from which work is directly executed
		TaskChain *					pLocalQueues[ kMaxQueues ];
		uint32							cLocalQueues;
		CQuickMutex					LocalLock;
		LWorkerThread *				pThread;

		uint32							uQueueReadIndex[ kMaxQueues ];

		AX_CACHEALIGN

		// Current reader index
		uint32							uLocalQueueIndex;

		// Last dependency (for lazy updates)
		TaskChain *					pLastDepender;
		uint32							cLastDependerUnblocks;

		// Whether to quit/abort
		volatile bool				bQuit;

		AX_CACHEALIGN
		SExecStats					DeferredStats[ kMaxQueues ];

#if AX_TRANSFER_QUEUES_ENABLED
		// Queues waiting to be submitted
		AX_CACHEALIGN
		TaskChain *					pTransferQueues[ kMaxQueues ];
		uint32							cTransferQueues;
		CQuickMutex					TransferLock;
#endif

		inline WorkerState()
		: cLocalQueues( 0 )
		, LocalLock()
		, pThread( nullptr )
		, uLocalQueueIndex( 0 )
		, pLastDepender( nullptr )
		, cLastDependerUnblocks( 0 )
		, bQuit( false )
#if AX_TRANSFER_QUEUES_ENABLED
		, cTransferQueues( 0 )
		, TransferLock()
#endif
		{
			for( uint32 i = 0; i < kMaxQueues; ++i ) {
				DeferredStats[ i ].Reset();
			}

#if AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			memset( &uQueueReadIndex, 0, sizeof( uQueueReadIndex ) );
#endif
		}
		~WorkerState();

		// Decrement 'cAccessors' for each unfinished queue
		void CleanUnfinishedWork()
		{
			// We must be in the process of quitting for this to work
			AX_ASSERT( bQuit == true );

			// We're no longer accessing these queues
			for( uint32 i = uLocalQueueIndex; i < cLocalQueues; ++i ) {
				pLocalQueues[ i ]->DecrementAccessor();
			}
		}

		// Merge the deferred SExecStats to the appropriate queues
		inline void MergeDeferredStatsNoLock()
		{
			for( uint32 i = 0; i < cLocalQueues; ++i ) {
				pLocalQueues[ i ]->Stats.Merge( DeferredStats[ i ] );
			}

			for( uint32 i = 0; i < cLocalQueues; ++i ) {
				DeferredStats[ i ].Reset();
			}
		}
		
#if AX_TRANSFER_QUEUES_ENABLED
		// Immediately add work to the transfer queue without regard to safety
		inline void AddTransferQueueNoLock( TaskChain *pQueue )
		{
			pTransferQueues[ cTransferQueues++ ] = pQueue;
		}
		// Add work to the transfer queue
		inline void AddTransferQueue( TaskChain *pQueue )
		{
			TransferLock.Acquire();
			AddTransferQueueNoLock( pQueue );
			TransferLock.Release();
		}
#endif
		
		// Perform a lazy update for the last dependency
		void CommitLastDepender();

#if AX_TRANSFER_QUEUES_ENABLED
		// Immediately transfer the queues to the scheduler; no synchronization
		inline void FlushTransferQueuesNoLock( SchedulerState &scheduler )
		{
			// Do the transfer
			scheduler.SubmitNoLock( pTransferQueues, cTransferQueues );

			// Nothing left to copy
			cTransferQueues = 0;
		}
		// Attempt to transfer work to the 
		inline bool TryFlushTransferQueues( SchedulerState &scheduler )
		{
			if( !cTransferQueues || !scheduler.SubmitLock.TryAcquire() ) {
				return false;
			}

			FlushTransferQueuesNoLock( scheduler );
			scheduler.SubmitLock.Release();

			return true;
		}
		// Attempt to transfer work from any scheduler (prioritizing self)
		inline bool TryFlushAnyTransferQueues( SchedulerState &scheduler )
		{
			// Get permission to write to the ready queue
			if( !scheduler.SubmitLock.TryAcquire() ) {
				return false;
			}

			// Write our own data if it's present
			if( cTransferQueues > 0 ) {
				FlushTransferQueuesNoLock( scheduler );
				scheduler.SubmitLock.Release();

				return true;
			}

			// <see loop after the following>
			WorkerState *pLockedWorkers[ kMaxWorkers ];
			uint32 cLockedWorkers = 0;

			// Enumerate each worker
			for( uint32 i = 0; i < scheduler.cWorkers; ++i ) {
				// Skip self and workers without transfer queues
				WorkerState *const pWorker = scheduler.pWorkers[ i ];
				if( pWorker == this || !pWorker->cTransferQueues ) {
					continue;
				}

				// Attempt to acquire a lock
				if( !pWorker->TransferLock.TryAcquire() ) {
					// If the lock fails, then add to the "try again" queue then
					// try the next worker
					pLockedWorkers[ cLockedWorkers++ ] = pWorker;
					continue;
				}

				// We have the lock, so perform the transfer
				pWorker->FlushTransferQueuesNoLock( scheduler );
				// MScheduler is now free
				scheduler.SubmitLock.Release();

				// We're done with the worker, and this function
				pWorker->TransferLock.Release();
				return true;
			}

			// Try the workers that we failed to lock
			for( uint32 i = 0; i < cLockedWorkers; ++i ) {
				// If we fail to acquire a lock on this _again_ then move on
				if( !pLockedWorkers[ i ]->TransferLock.TryAcquire() ) {
					continue;
				}

				// We have the lock; perform the transfer
				pLockedWorkers[ i ]->FlushTransferQueuesNoLock( scheduler );
				// MScheduler is now free
				scheduler.SubmitLock.Release();

				// We're done with the worker, and this function
				pLockedWorkers[ i ]->TransferLock.Release();
				return true;
			}

			// Absolutely nothing could be added! Wasted time
			scheduler.SubmitLock.Release();
			return false;
		}
#endif

		// Call this in spin loops for miscellaneous maintenance
		inline void Idle( SchedulerState &scheduler )
		{
			// Update the last dependency (this might provide extra work)
			CommitLastDepender();

			// Add more work to the list of available queues
#if AX_TRANSFER_QUEUES_ENABLED
# if 0
			if( TryFlushAnyTransferQueues( scheduler ) ) {
				return;
			}
# else
			if( TryFlushTransferQueues( scheduler ) ) {
				return;
			}
# endif
#endif
		}

		// Execute work from the local queues
		void Execute( SchedulerState &scheduler, uint32 workerId );
	};
	
	inline void Task::Execute( WorkerState &worker, SExecStats *pExecStats )
	{
		// Execute the data and update corresponding information
		const uint64 baseTimeMicrosec = pExecStats != nullptr ? System::Microseconds() : 0;
		pfnKernel( pTaskData );
		if( pExecStats != nullptr ) {
			pExecStats->AddJobTime( System::Microseconds() - baseTimeMicrosec );
		}

		// Need to decrement the decrease pointer immediately
		if( pDecrease != nullptr ) {
			AtomicDec( pDecrease );
		}

		// The remainder of this code deals with dependencies
		if( !pDepender ) {
			return;
		}
		
		// Avoid atomic operations by retaining information about the last dependency
		if( worker.pLastDepender != nullptr && worker.pLastDepender != pDepender ) {
			worker.CommitLastDepender();
		}

		worker.pLastDepender = pDepender;
		++worker.cLastDependerUnblocks;
	}
	
	inline void SchedulerState::EndFrame()
	{
		// Wait for each allocated queue to complete
		//
		//	NOTE: This is fine for queues that haven't been submitted as well
		//	-     since they will not have any accessors and therefore won't
		//	-     register as "in use."
		//
		for( uint32 i = 0; i < cQueues; ++i ) {
			Queues[ i ].Wait();
		}

		// The rest of this function handles stats collection
		if( !bCollectStats ) {
			return;
		}

		// Merge stats in now
		for( uint32 i = 0; i < cWorkers; ++i ) {
			pWorkers[ i ]->MergeDeferredStatsNoLock();
		}
	}
	inline void FixQueueAccessors( TaskChain *pChain, uint32 cAccessors )
	{
		AX_ASSERT_NOT_NULL( pChain );
		AX_ASSERT( cAccessors > 0 );

		pChain->cAccessors = cAccessors;

		for( uint32 i = 0; i < pChain->cTasks; ++i ) {
			if( pChain->pTasks[ i ].pDepender != nullptr && pChain->pTasks[ i ].pDepender->cAccessors != cAccessors ) {
				FixQueueAccessors( pChain->pTasks[ i ].pDepender, cAccessors );
			}
		}
	}
	inline void SchedulerState::SubmitNoLock( TaskChain *const *ppQueues, uint32 cQueues )
	{
		WorkerState *pLockedWorkers[ kMaxWorkers ];
		uint32 cLockedWorkers = 0;

		const uint64 submitTime = System::Microseconds();
		// Each queue will have 'cWorkers'-accessors
		for( uint32 i = 0; i < cQueues; ++i ) {
			FixQueueAccessors( ppQueues[ i ], cWorkers );
			ppQueues[ i ]->uSubmitTime = submitTime;
		}

		// Try to add work to the local queues of each worker
		for( uint32 i = 0; i < cWorkers; ++i ) {
			// If we can't acquire the lock now then add to the local list of retries
			if( !pWorkers[ i ]->LocalLock.TryAcquire() ) {
				pLockedWorkers[ cLockedWorkers++ ] = pWorkers[ i ];
				continue;
			}

			// Copy the queue over
			memcpy( &pWorkers[ i ]->pLocalQueues[ pWorkers[ i ]->cLocalQueues ], ppQueues, cQueues*sizeof( *ppQueues ) );
			pWorkers[ i ]->cLocalQueues += cQueues;

			// Done with this worker
			pWorkers[ i ]->LocalLock.Release();
		}

		// Retry for everything we weren't able to update right away
		while( cLockedWorkers > 0 ) {
			// Enumerate the list of locked workers
			uint32 i = 0;
			while( i < cLockedWorkers ) {
				WorkerState *const pWorker = pLockedWorkers[ i ];

				// If a lock can't be acquired, then try the next worker
				if( !pWorker->LocalLock.TryAcquire() ) {
					++i;
					continue;
				}

				// Lock acquired; copy the queue
				memcpy( &pWorker->pLocalQueues[ pWorker->cLocalQueues ], ppQueues, cQueues*sizeof( *ppQueues ) );
				pWorker->cLocalQueues += cQueues;

				// Release the lock
				pWorker->LocalLock.Release();

				// Remove this worker from the list
				for( uint32 j = i + 1; j < cLockedWorkers; ++j ) {
					pLockedWorkers[ j - 1 ] = pLockedWorkers[ j ];
				}
				--cLockedWorkers;
			}

			// Avoid stressing the bus too much for the same locks
			if( cLockedWorkers > 0 && cLockedWorkers <= 2 ) {
				LocalSpin( 2048/cLockedWorkers );
			}
		}
	}
	inline void WorkerState::Execute( SchedulerState &scheduler, uint32 workerId )
	{
		const bool bCollectStats = scheduler.bCollectStats;
		SExecStats *pExecStats = bCollectStats ? &DeferredStats[ uLocalQueueIndex ] : nullptr;
		if( pExecStats != nullptr && uLocalQueueIndex < cLocalQueues ) {
			pExecStats->Reset();
		}
#if AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
		if( uLocalQueueIndex < cLocalQueues ) {
			uQueueReadIndex[ uLocalQueueIndex ] = 0;
		}
#endif

		// Check for work
		while( uLocalQueueIndex < cLocalQueues ) {
			// Return immediately if we're quitting
			if( bQuit ) {
				return;
			}

#if !AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
			// Grab a job
			Task *const pTask = pLocalQueues[ uLocalQueueIndex ]->Read();
#else
			const uint32 jobIndex = ( uQueueReadIndex[ uLocalQueueIndex ]++ )*scheduler.cWorkers + workerId;
			Task *const pTask = jobIndex < pLocalQueues[ uLocalQueueIndex ]->cTasks ? &pLocalQueues[ uLocalQueueIndex ]->pTasks[ jobIndex ] : nullptr;
#endif

			// No more in this queue; move on
			if( !pTask ) {
				// We're no longer accessing this queue
				pLocalQueues[ uLocalQueueIndex ]->DecrementAccessor();

				// Go to the next queue
				++uLocalQueueIndex;
#if AX_WORKER_LOCAL_QUEUE_INDEX_ENABLED
				uQueueReadIndex[ uLocalQueueIndex ] = 0;
#endif
				if( bCollectStats ) {
					pExecStats = &DeferredStats[ uLocalQueueIndex ];
					pExecStats->Reset();
				}

				continue;
			}

			// Execute the job
			pTask->Execute( *this, pExecStats );
		}

		// Nothing was done, so do normal maintenance stuff
		Idle( scheduler );
	}

	//
	//	Originally, the TaskQueue (and similar interfaces) were meant to be
	//	emulated in this version of the scheduler. Some effort was made but it
	//	was determined that this would take longer than it was worth, so that
	//	sub-project has been postponed indefinitely.
	//
	//	The following range of code is the latest state of those efforts.
	//

#if 0
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
		struct SJob
		{
			FnJob					pfnKernel;
			void *					pData;
		};
		typedef SExecStats stats_t;

		// Indicates the status of the TaskQueue's life
		enum state_t
		{
			// Ready for jobs to be added to it
			kReady,
			// It's been submitted to the scheduler
			kQueued,
			// All tasks in the queue have completed execution
			kDone
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
		inline void AddJob( FnJob kernel, void *data = nullptr )
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
		void Submit();
		/*!
		 *	Wait on this queue until it completes execution.
		 *
		 *	Returns true if the queue completed execution and false if it was
		 *	aborted.
		 */
		bool Wait();

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
		// Task queue (allocated upon submission)
		TArray< TaskChain * >	m_TaskChains;
		// Chain at index 'i' of m_TaskChains depends on chain at index 'i' of m_ChainDependencies
		TArray< TaskChain * >	m_ChainDependencies;
		// Completion signal (allocated upon submission)
		volatile uint32 *				m_pCompletion;
		
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

		/*!
		 *	Convert to the internal representation
		 */
		void Convert();
	};


	/*
	===========================================================================

		TASK QUEUE IMPLEMENTATION

	===========================================================================
	*/

	inline TaskQueue::TaskQueue()
	{
	}

	inline void TaskQueue::AddJobs( const SJob *jobs, uint32 numJobs )
	{
		// Adding jobs can only be done prior to submission
		AX_ASSERT( m_State == kReady );

		// Only do this if it's worthwhile
		AX_ASSERT( numJobs > 0 );
		const int numItems = ( int )numJobs;

		// Add a signal if the last job was a fence for the current signal
		if( m_Jobs.Num() > 0 && LastJob()->pfnKernel == &Fence_f ) {
			if( ( int )( uintptr )LastJob()->pData == m_Signals.Num() - 1 ) {
				( void )AddSignal();
			}
		}

		// Append the job to the list
		AX_VERIFY( m_Jobs.Append( numItems, jobs ) );

		// Make sure we're within range on the signals (pedantic paranoia)
		AX_ASSERT( m_Signals.Num() <= 256 );
		const uint8 currentSignalIndex = m_Signals.IsEmpty() ? 0 : ( uint8 )( m_Signals.Num() - 1 );

		// Update the corresponding slot in the signal indexes list
		AX_VERIFY( m_JobSignalIndexes.Resize( m_JobSignalIndexes.Num() + numItems, currentSignalIndex ) );

		// If we haven't added the initial signal yet, then add it
		if( !currentSignalIndex && m_Signals.IsEmpty() ) {
			( void )AddSignal();
		}

		// Increment the signal
		m_Signals[ currentSignalIndex ] += numJobs;
	}
	inline uint32 TaskQueue::AddSignal()
	{
		// Adding signals can only be done prior to submission
		AX_ASSERT( m_State == kReady );

		// Get the index for the signal that we're going to add
		const uint32 newSignalIndex = ( uint32 )m_Signals.Num();

		// We can't add the signal if we're past our limit
		AX_ASSERT( newSignalIndex <= 255 );

		// Append the signal
		AX_VERIFY( m_Signals.Append( 0 ) );

		// Done here
		return newSignalIndex;
	}
	inline void TaskQueue::AddFence( uint32 signalIndex )
	{
		// Adding fences can only be done prior to submission
		AX_ASSERT( m_State == kReady );

		if( signalIndex == ( uint32 )-1 ) {
			signalIndex = m_Signals.IsEmpty() ? 0 : uint32( m_Signals.Num() - 1 );
		}

		// Ensure the given signal exists
		//
		// If this is the initial signal then we're adding a fence without
		// having jobs, which should not be done anyway.
		AX_ASSERT( signalIndex < ( uint32 )m_Signals.Num() );

		// Ensure the given signal has jobs attached (otherwise we'll introduce
		// unnecessary overhead)
		AX_ASSERT( m_Signals[ signalIndex ] > 0 );

		// Add a fence here
		const SJob fence = { &Fence_f, ( void * )signalIndex };
		AX_VERIFY( m_Jobs.Append( fence ) );
		AX_VERIFY( m_JobSignalIndexes.Append( 0xFF ) );
	}
	inline void TaskQueue::Submit()
	{
		// Only allow submission of lists which are "ready"
		AX_ASSERT( m_State == kReady );
		// There must be jobs in the list (otherwise no point submitting)
		AX_ASSERT( m_Jobs.Num() > 0 );
		// Verify that m_Jobs and m_JobSignalIndexes match up
		AX_ASSERT( m_Jobs.Num() == m_JobSignalIndexes.Num() );

		// If the last job isn't a fence then add one
		if( LastJob()->pfnKernel != &Fence_f ) {
			// The signal used by the last job will be the last signal added.
			//
			// The last job is not a fence so this signal must exist and be
			// greater than zero.
			AddFence( uint32( m_Signals.Num() - 1 ) );
		}

		// Set our state to submitted now
		m_State = kQueued;

		// Convert the queue then submit the converted version
		Convert();

		TaskChain *pChains[ 256 ];
		uint32 cChains = 0;
		
		AX_ASSERT( m_TaskChains.Num() <= 256 );

		for( uintptr i = 0; i < m_TaskChains.Num(); ++i ) {
			if( m_ChainDependencies[ i ] != nullptr ) {
				continue;
			}

			pChains[ cChains++ ] = m_TaskChains[ i ];
		}

		TASKS().SubmitChains( pChains, cChains );
	}
	inline void TaskQueue::Convert()
	{
		auto &sched = TASKS();

		const uint32 cJobs = m_Jobs.Num();

		// Allocate the completion signal
		m_pCompletion = sched.AllocCompletion();

		// Allocate one chain per signal
		const uint32 cChains = m_Signals.Num();
		TaskChain *const pChains = sched.AllocChains( cChains );
		AX_ASSERT_NOT_NULL( pChains );

		m_TaskChains.Reserve( cChains );
		for( uint32 i = 0; i < cChains; ++i ) {
			m_TaskChains.Append( &pChains[ i ] );

			pChains[ i ].cBlocked = 0;

			pChains[ i ].cTasks = m_Signals[ i ];
			pChains[ i ].pTasks = sched.AllocTasks( pChains[ i ].cTasks );
			AX_ASSERT_NOT_NULL( pChains[ i ].pTasks );

			pChains[ i ].uReadIndex = 0;
			pChains[ i ].Stats.Reset();
		}

		TaskChain *pLastFence = nullptr;
		uint8 FenceSignalIndex = 0xFF;
		const uint32 baseJob = 0;

		// Manage dependencies per chain
		for( uint32 i = 0; i < cJobs; ++i ) {
			const auto &job = m_Jobs[ i ];

			// Fences define dependencies
			if( job.pfnKernel == &Fence_f ) {
				FenceSignalIndex = ( uint8 )( uintptr )job.pData;
				AX_ASSERT( uintptr( FenceSignalIndex ) < m_Signals.Num() );

				pLastFence = &pChains[ FenceSignalIndex ];
				continue;
			}

			// 

			// Get the chain this job belongs to
			const uint8 jobSignalIndex = m_JobSignalIndexes[ i ];
			AX_ASSERT( uintptr( jobSignalIndex ) < cChains );

			TaskChain *const pChain = &pChains[ jobSignalIndex ];

			// Put the job in the chain
			pChain->pTasks[ 

			// pChain has a dependency on this job's completion
			pChain->cBlocked
		}



		// Create a chain from each fence
		for( job = 0; job < numJobs; ++job ) {
			// If it's a normal job then ignore it for now
			//
			// NOTE: The jobs array always ends with a fence, so this is safe
			// -     even for the last set of jobs in the array
			if( m_Jobs[ job ].pfnKernel != &Fence_f ) {
				continue;
			}

			// Zero-length should be ignored
			const uint32 cTasks = job - baseJob;
			if( !cTasks ) {
				baseJob = job + 1;
				continue;
			}

			// Allocate the chain
			TaskChain *const pChain = sched.AllocChains( 1 );
			AX_ASSERT_NOT_NULL( pChain );

			// Allocate the nuggets within the chain
			pChain->pTasks = sched.AllocTasks( cTasks );
			AX_ASSERT_NOT_NULL( pChain->pTasks );

			// Add the chain to the internal list
			AX_VERIFY( m_TaskChains.Append( pChain ) );
			AX_VERIFY( m_ChainDependencies.Append( nullptr ) );

			// Initialize the nuggets
			for( uint32 i = 0; i < cTasks; ++i ) {
				pChain->pTasks[ i ].pfnKernel = m_Jobs[ baseJob + i ].pfnKernel;
				pChain->pTasks[ i ].pTaskData = m_Jobs[ baseJob + i ].pData;
				pChain->pTasks[ i ].pDecrease = m_pCompletion;
				pChain->pTasks[ i ].pDepender = nullptr;
			}

			// Set the new base
			baseJob = job + 1;
		}

		// Set dependency pointers for each chain
		for( uint32 i = 0; i < m_TaskChains.Num(); ++i ) {
		}
	}
#if 0
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
		// Task queue (allocated upon submission)
		TArray< TaskChain * >	m_TaskChains;
		// Chain at index 'i' of m_TaskChains depends on chain at index 'i' of m_ChainDependencies
		TArray< TaskChain * >	m_ChainDependencies;
#endif
	inline bool TaskQueue::Wait()
	{
	}

	inline TaskQueue::state_t TaskQueue::GetState() const
	{
	}
	inline const TaskQueue::stats_t &TaskQueue::GetStats() const
	{
	}

	inline void TaskQueue::OutputDebugInfo( const char *pszQueueName ) const
	{
	}

	inline void TaskQueue::Reset()
	{
	}
#endif

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

		void BeginFrame();
		void EndFrame();

		inline uint32 GetThreadCount() const
		{
			return m_State.cWorkers;
		}

		void WaitForChains( TaskChain *const *ppChains, uint32 cChains );
		template< uint32 tSize >
		inline void WaitForChains( TaskChain *const( &ppChains ) )
		{
			WaitForChains( ppChains, tSize );
		}
		inline void WaitForChain( TaskChain *pChain )
		{
			WaitForChains( &pChain, 1 );
		}

		inline Task *AllocTasks( uint32 count )
		{
			return m_State.AllocTasks( count );
		}
		inline TaskChain *AllocChains( uint32 count )
		{
			return m_State.AllocChains( count );
		}
		inline volatile uint32 *AllocCompletion()
		{
			return m_State.AllocCompletion();
		}

		inline bool SubmitChain( TaskChain *pChain )
		{
			return SubmitChains( &pChain, 1 );
		}
		bool SubmitChains( TaskChain *const *ppChains, uintptr cChains );
		template< uintptr tSize >
		inline bool SubmitChains( TaskChain *const( &ppChains )[ tSize ] )
		{
			return SubmitChains( ppChains, tSize );
		}

	private:
		SchedulerState				m_State;

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

		LWorkerThread( SchedulerState &scheduler, uint32 workerId );
		virtual ~LWorkerThread();

		void Abort();

	protected:
		virtual int OnRun();

	private:
		SchedulerState &			m_Scheduler;
		WorkerState &				m_Worker;
		const uint32					m_WorkerId;
	};

	inline WorkerState::~WorkerState()
	{
		bQuit = true;
		delete pThread;
	}
	inline void WorkerState::CommitLastDepender()
	{
		// If there is no dependency then we're done here
		if( !pLastDepender ) {
			return;
		}

		// Reduce the number of blockers on the dependency; if we're down
		// to just the last set then transfer
		if( AtomicSub( &pLastDepender->cBlocked, cLastDependerUnblocks ) == cLastDependerUnblocks ) {
			//AddTransferQueue( pLastDepender );
			TASKS().SubmitChain( pLastDepender );
		}

		// There is no longer a last dependency
		pLastDepender = nullptr;
		cLastDependerUnblocks = 0;
	}

}}

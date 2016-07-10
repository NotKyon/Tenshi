#include "Scheduler01.hpp"
#include "../Atomic.hpp"
#include "../CPU.hpp"
#include "../../Core/Assert.hpp"
#include "../../System/HighPerformanceClock.hpp"
#include "../../System/Timer.hpp"
#include "../../System/TimeConversion.hpp"
#include "../../Core/Logger.hpp"
#include "../../Core/String.hpp"

namespace Ax { namespace Async {


	void AX_JOB_API TaskQueue::Fence_f( void * )
	{
	}

	TaskQueue::TaskQueue()
	: m_Jobs()
	, m_JobSignalIndexes()
	, m_Signals()
	, m_uRefCnt( 1 )
	, m_State( kReady )
	, m_Priority( priority_t::kNormal )
	, m_Stats()
	, m_uSubmitTime( 0 )
	, m_uJobReadIndex( 0 )
	, m_uJobReadLock( 0 )
	, m_uNumAccessors( 0 )
	{
	}
	TaskQueue::~TaskQueue()
	{
	}

	void TaskQueue::AddJobs( const SJob *jobs, uint32 numJobs )
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
	uint32 TaskQueue::AddSignal()
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
	void TaskQueue::AddFence( uint32 signalIndex )
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
	void TaskQueue::Submit( Latency latency )
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
		m_uSubmitTime = System::Microseconds();

		// Submit ourselves to the scheduler (which will dispatch us to each thread)
		( void )TASKS().SubmitQueue( this, latency );
	}
	bool TaskQueue::Wait()
	{
		// We need to be submitted in order to be waited upon
		AX_ASSERT( m_State != kReady );

		// Wait until we're marked as done (or aborted)
		while( m_State < kDone || m_uNumAccessors > 0 ) {
			// Don't take up CPU time while we wait though
			CThread::Yield();
		}

		// We completed successfully if we were set to "done" (rather than "aborted")
		return m_State == kDone;
	}
	void TaskQueue::Abort()
	{
		// In order to abort we need to be submitted
		AX_ASSERT( m_State != kReady );

		// We're aborting so don't let any worker threads pull work from here
		AtomicAdd( &m_uJobReadIndex, ( uint32 )m_Jobs.Num() );

		// Wait until all of the worker threads give up on this queue
		while( *( const volatile uint32 * )&m_uNumAccessors > 0 ) {
			CThread::Yield();
		}

		// We're now successfully aborted
		m_State = kAborted;
	}
	void TaskQueue::SetPriority( priority_t priority )
	{
		m_Priority = priority;
	}
	TaskQueue::priority_t TaskQueue::GetPriority() const
	{
		return m_Priority;
	}
	TaskQueue::state_t TaskQueue::GetState() const
	{
		return m_State;
	}
	const TaskQueue::stats_t &TaskQueue::GetStats() const
	{
		return m_Stats;
	}
	void TaskQueue::Reset()
	{
		// Queue must not be currently executing
		AX_ASSERT( m_State != kQueued );
		AX_ASSERT( m_uNumAccessors == 0 );

		// Reset all settings
		m_Jobs.Clear();
		m_JobSignalIndexes.Clear();
		m_Signals.Clear();
		m_State = kReady;
		m_Priority = priority_t::kNormal;
		m_Stats.Reset();
		m_uSubmitTime = 0;
		m_uJobReadIndex = 0;
		m_uJobReadLock = 0;
		m_uNumAccessors = 0;
	}
	void TaskQueue::OutputDebugInfo( const char *pszName ) const
	{
		m_Stats.OutputDebugInfo( pszName );
	}
	void TaskQueue::stats_t::OutputDebugInfo( const char *pszName ) const
	{
		String s;
		
		s.Reserve( 512 );
		s = "TaskQueue stats";

		if( pszName != nullptr && *pszName != '\0' ) {
			s += " <";
			s += pszName;
			s += ">";
		}
		s += ":\n";

		const double minJobTimeSec = System::MicrosecondsToSeconds( uMinJobTime );
		const double minJobTimeFrm = System::MicrosecondsToFrames( uMinJobTime );

		const double maxJobTimeSec = System::MicrosecondsToSeconds( uMaxJobTime );
		const double maxJobTimeFrm = System::MicrosecondsToFrames( uMaxJobTime );

		const double totalJobTimeSec = System::MicrosecondsToSeconds( uTotalJobTime );
		const double totalJobTimeFrm = System::MicrosecondsToFrames( uTotalJobTime );

		const double totalRunTimeSec = System::MicrosecondsToSeconds( uTotalExecTime );
		const double totalRunTimeFrm = System::MicrosecondsToFrames( uTotalExecTime );

		s.AppendFormat( "  minJobTime   = %.6f sec [%3.5f%%]\n", minJobTimeSec, minJobTimeFrm*100.0 );
		s.AppendFormat( "  maxJobTime   = %.6f sec [%3.5f%%]\n", maxJobTimeSec, maxJobTimeFrm*100.0 );
		s.AppendFormat( "  totalJobTime = %.6f sec [%3.5f%%]\n", totalJobTimeSec, totalJobTimeFrm*100.0 );
		s.AppendFormat( "  totalRunTime = %.6f sec [%3.5f%%]\n", totalRunTimeSec, totalRunTimeFrm*100.0 );
		s.AppendFormat( "  numStalls    = %u\n", cStalls );

		if( s.EndsWith( "\n" ) ) {
			s.Remove( -1, 1 );
		}
		BasicStatusf( "%s", s.CString() );
	}

	TaskQueue::executeResult_t TaskQueue::ExecuteWork( stats_t &stats )
	{
		// Attempt to acquire access to the queue
		if( AtomicInc( &m_uJobReadLock ) != 0 ) {
			// We were blocked by another thread accessing this queue
			AtomicDec( &m_uJobReadLock );
			++stats.cStalls;
			return kExecBlocked;
		}

		// Are we out of jobs?
		if( m_uJobReadIndex == ( uint32 )m_Jobs.Num() ) {
			// Then we should be removed
			AtomicDec( &m_uJobReadLock );
			m_State = kDone;
			return kExecRemoved;
		}

		// Grab the next job from the list
		uint32 jobIndex = m_uJobReadIndex++;
		const SJob *job = m_Jobs.Pointer( jobIndex );

		// Check for a fence (special case)
		if( job->pfnKernel == &Fence_f ) {
			// Determine which signal we depend upon
			const uint32 signalIndex = ( uint32 )( uintptr )job->pData;

			// Check for an untriggered signal
			if( m_Signals[ signalIndex ] > 0 ) {
				// There are still jobs depending on this so we're blocked
				--m_uJobReadIndex;
				AtomicDec( &m_uJobReadLock );
				++stats.cStalls;
				return kExecBlocked;
			}

			// If we're out of jobs then the list has finished
			if( m_uJobReadIndex == ( uint32 )m_Jobs.Num() ) {
				// Which means this list needs to be removed from whichever threads are holding it
				AtomicDec( &m_uJobReadLock );
				m_State = kDone;
				m_Stats.uTotalExecTime = System::Microseconds() - m_uSubmitTime;
				return kExecRemoved;
			}

			// Grab the next job (it won't be a fence)
			jobIndex = m_uJobReadIndex++;
			job = m_Jobs.Pointer( jobIndex );
		}

		// Others can now read from this queue
		AtomicDec( &m_uJobReadLock );

		// The kernel must be valid
		AX_ASSERT_NOT_NULL( job->pfnKernel );
		AX_ASSERT( job->pfnKernel != &Fence_f );

		// Execute the job
		System::CTimer jobWorkTimer;
		job->pfnKernel( job->pData );
		AtomicDec( &m_Signals[ m_JobSignalIndexes[ jobIndex ] ] );
		const uint64 jobWorkTime = jobWorkTimer.GetElapsed();

		// Update the stats
		stats.uTotalJobTime += jobWorkTime;
		if( stats.uMinJobTime > jobWorkTime ) {
			stats.uMinJobTime = jobWorkTime;
		}
		if( stats.uMaxJobTime < jobWorkTime ) {
			stats.uMaxJobTime = jobWorkTime;
		}

		// Done
		return kExecSuccess;
	}
	void TaskQueue::MergeStats( const stats_t &stats )
	{
		m_Stats.Merge( stats );
	}

	const SJob *TaskQueue::LastJob() const
	{
		const size_t n = m_Jobs.Num();
		if( !n ) {
			return nullptr;
		}

		return m_Jobs.Pointer( n - 1 );
	}

	MScheduler &MScheduler::GetInstance()
	{
		static MScheduler instance;
		return instance;
	}

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable:4351 )
#endif
	MScheduler::MScheduler()
	: m_pThreads()
	, m_cThreads( 0 )
	, m_uFrame( 0 )
	, m_pWaiters()
	, m_cWaiters()
	{
		for( uint32 i = 0; i < kNumBuffers; ++i ) {
			m_cWaiters[ i ] = 0;
		}
	}
#ifdef _MSC_VER
# pragma warning( pop )
#endif
	MScheduler::~MScheduler()
	{
		Fini();
	}

	bool MScheduler::Init( uint32 numThreads )
	{
		// Query the core count
		static const uint32 cPhyCores = GetCPUCoreCount();
		static const uint32 cLogCores = GetCPUThreadCount();
		//static const uint32 cCoreDist = cPhyCores != cLogCores ? cLogCores : cPhyCores + cPhyCores/2;
		static const uint32 cCoreDist = cLogCores;

		// Ensure we're not currently running
		AX_ASSERT( m_cThreads == 0 );

		// Figure out how many threads is desired
		const uint32 cDesiredThreads = numThreads == 0 ? cCoreDist : numThreads;
		// Clamp the number of threads to the maximum
		const uint32 cThreads = cDesiredThreads > kMaxThreads ? kMaxThreads : cDesiredThreads;

		// Create each thread
		for( m_cThreads = 0; m_cThreads < cThreads; ++m_cThreads ) {
			// Allocate
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4316) // object allocated on the heap may not be aligned
#endif
			m_pThreads[ m_cThreads ] = new LWorkerThread( *this );
#ifdef _MSC_VER
# pragma warning(pop)
#endif
			// Fail if not allocated
			if( !AX_VERIFY_NOT_NULL( m_pThreads[ m_cThreads ] ) ) {
				// Destruct in reverse order (while decreasing the count)
				while( m_cThreads > 0 ) {
					delete m_pThreads[ --m_cThreads ];
					m_pThreads[ m_cThreads ] = nullptr;
				}
				// Mission failed
				return false;
			}
		}

		// Start-up each thread (now that we have all of them allocated)
		for( uint32 i = 0; i < m_cThreads; ++i ) {
			m_pThreads[ i ]->Start();
		}

		// Done!
		return true;
	}
	void MScheduler::Fini()
	{
		// If we don't have any threads allocated then just return
		if( !m_cThreads ) {
			return;
		}

		// Tell each thread to stop
		for( uint32 i = 0; i < m_cThreads; ++i ) {
			m_pThreads[ i ]->SignalStop();
		}

		// Wait for a moment
		CThread::Yield();

		// Delete each thread in reverse order
		while( m_cThreads > 0 ) {
			// Calculate the index
			const uint32 i = m_cThreads - 1;

			// Delete the thread
			delete m_pThreads[ i ];
			m_pThreads[ i ] = nullptr;

			// Set the new count (it's equal to the index)
			m_cThreads = i;
		}
	}

	bool MScheduler::SubmitQueue( TaskQueue *queue, TaskQueue::Latency latency )
	{
		// Must be valid
		AX_ASSERT_NOT_NULL( queue );

		// We need to be active to accept a queue
		if( !m_cThreads ) {
			// Just execute the queue all at once here
			TaskQueue::stats_t stats;
			while( queue->ExecuteWork( stats ) != TaskQueue::kExecRemoved ) {
			}
			queue->MergeStats( stats );

			// Done
			return true;
		}

		// Initially successful
		bool success = true;

		// Add the queue to each of the worker threads
		for( uint32 i = 0; i < m_cThreads; ++i ) {
			success &= m_pThreads[ i ]->SubmitQueue( queue );
		}

		const uint32 thisFrame = ( m_uFrame + 0 )%kNumBuffers;
		const uint32 nextFrame = ( m_uFrame + 1 )%kNumBuffers;
		const uint32 thisOffset = thisFrame*kMaxQueues;
		const uint32 nextOffset = nextFrame*kMaxQueues;

		// Add to the list of waiters
		switch( latency ) {
		// No latency restrictions on unlimited, so nothing to do
		case TaskQueue::Latency::Unlimited:
			break;

		// Must complete by end of this frame, so add to immediate queue
		case TaskQueue::Latency::ThisFrame:
			AX_ASSERT( m_cWaiters[ thisFrame ] < kMaxQueues );

			queue->AddRef();
			m_pWaiters[ thisOffset + AtomicInc( &m_cWaiters[ thisFrame ] ) ] = queue;
			break;

		// Must complete by end of next frame, so add to delayed queue
		case TaskQueue::Latency::NextFrame:
			AX_ASSERT( m_cWaiters[ nextFrame ] < kMaxQueues );

			queue->AddRef();
			m_pWaiters[ nextOffset + AtomicInc( &m_cWaiters[ nextFrame ] ) ] = queue;
			break;
		}
		
		// Done
		return success;
	}
	void MScheduler::StepFrame()
	{
		//
		//	NOTE: This code needs several buffers to work properly
		//	-     kNumBuffers should be 4 for correct functionality
		//

		const uint32 frame = AtomicInc( &m_uFrame )%kNumBuffers;
		const uint32 offset = frame*kMaxQueues;

		// Wait for each task queue for this frame
		while( m_cWaiters[ frame ] > 0 ) {
			// Grab the index we're allowed to modify
			const uint32 i = offset + AtomicDec( &m_cWaiters[ frame ] ) - 1;

			// Invalid indexes aren't allowed
			if( i < offset || i >= kMaxQueues*kNumBuffers ) {
				break;
			}

			// Paranoia. Evaluate later.
			//
			// A queue submission to this frame might be present
			uint32 spinCount = 1024; //Selected by fair dice roll
			while( !m_pWaiters[ i ] && spinCount > 0 ) {
				CThread::Yield();
				--spinCount;
			}

			if( spinCount == 0 ) {
				// Don't want to access a null pointer
				continue;
			}

			// Actually wait for the waiter
			m_pWaiters[ i ]->Wait();
			m_pWaiters[ i ]->Release();
			m_pWaiters[ i ] = nullptr;
		}
	}
	
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable:4351 )
#endif
	LWorkerThread::LWorkerThread( MScheduler &scheduler )
	: m_Scheduler( scheduler )
	, m_pQueues()
	, m_uBaseQueue( 0 )
	, m_uLastQueue( 0 )
	, m_uWriteLock( 0 )
	{
	}
#ifdef _MSC_VER
# pragma warning( pop )
#endif
	LWorkerThread::~LWorkerThread()
	{
	}
	
	void LWorkerThread::Abort()
	{
		SignalStop();
	}
	bool LWorkerThread::SubmitQueue( TaskQueue *queue )
	{
		AX_ASSERT_NOT_NULL( queue );
		queue->AddRef();

		AcquireWriteLock();
		if( IsQuitting() ) {
			ReleaseWriteLock();
			queue->Release();
			return false;
		}

		// Wait for space in the buffer to become available
		while( m_uLastQueue - m_uBaseQueue >= kMaxQueues ) {
			// Breathe
			CThread::Yield();

			// Check if we need to exit
			if( IsQuitting() ) {
				// Release the write lock and return
				ReleaseWriteLock();
				queue->Release();
				return false;
			}
		}

		// Write to the buffer
		const uint32 index = m_uLastQueue%kMaxQueues;
		m_pQueues[ index ] = queue;
		++m_uLastQueue; // NOTE: This must only be incremented AFTER the buffer has been written to
		ReleaseWriteLock();

		// Success!
		return true;
	}

	inline void Spin( uint32 spinCount = 1024 ) {
		for( volatile uint32 spin = 0; spin < spinCount; ++spin ) {
		}
	}

	int LWorkerThread::OnRun()
	{
		TaskQueue::stats_t stats[ kMaxQueues ];
		TaskQueue *lastBlockedQueue = nullptr;
		TaskQueue *queue = nullptr;
		uint32 queueIndex = 0;

		// Keep looping until we need to exit
		while( !IsQuitting() ) {
			// Check for work
			if( m_uBaseQueue == m_uLastQueue ) {
				//Spin();
				continue;
			}

			// Make note of the actual base/last indexes
			const uint32 baseIndex = m_uBaseQueue%kMaxQueues;
			const uint32 lastIndex = m_uLastQueue%kMaxQueues;

			// Check whether we need to fetch another queue
			if( queue == nullptr ) {
				// Determine the range we're working with
				const uint32 start = baseIndex < lastIndex ? baseIndex : lastIndex;
				const uint32 end = baseIndex > lastIndex ? baseIndex : lastIndex;

				// Search for the highest priority queue
				queue = m_pQueues[ start ];
				queueIndex = start;
				for( uint32 i = start + 1; i < end; ++i ) {
					// Check for a queue of a higher priority
					//
					// NOTE: If the current queue is the last blocked queue then it's ideal to reject it since we
					//       want to acquire a different queue at this point, even if it has a lower priority
					if( !( queue->GetPriority() < m_pQueues[ i ]->GetPriority() ) && queue != lastBlockedQueue ) {
						continue;
					}

					// This is the higher priority queue (convenience variable)
					TaskQueue *const higherPriorityQueue = m_pQueues[ i ];

#if 1
					// Swap places to sort from highest priority to lowest priority (reduce branch mispredictions)
					m_pQueues[ i ] = queue;
					m_pQueues[ start ] = higherPriorityQueue;
#else
					// Record the new index
					queueIndex = i;
#endif

					// Set the current queue candidate
					queue = higherPriorityQueue;

					// Leave the loop if we find the highest priority
					if( queue->GetPriority() >= TaskQueue::priority_t::kVeryHigh ) {
						break;
					}
				}
				
				// Increase the number of accessors on the new queue
				AtomicInc( &queue->m_uNumAccessors );

#if 0
				// Wait a little bit if this queue was the last one blocked (might be only one left)
				if( queue == lastBlockedQueue ) {
					Spin( 2048 );
				}
#endif

				// We don't care about the last blocked queue now
				lastBlockedQueue = nullptr;
			}

			// Execute work from the queue
			const TaskQueue::executeResult_t result = queue->ExecuteWork( stats[ queueIndex ] );

			// Should the queue now be removed?
			if( result == TaskQueue::kExecRemoved ) {
				// Apply stats to the queue then reset the internal stats
				queue->MergeStats( stats[ queueIndex ] );
				stats[ queueIndex ].Reset();

				// If this queue is not at the base of the range then swap and increment (to remove)
				if( queueIndex != baseIndex ) {
					// NOTE: A lock isn't necessary here because that really only affects m_uLastQueue
					m_pQueues[ queueIndex ] = m_pQueues[ baseIndex ];
				}
				++m_uBaseQueue;

				// If this was the last blocked queue then reset
				if( lastBlockedQueue == queue ) {
					lastBlockedQueue = nullptr;
				}

				// This queue is no longer in use so indicate we need a new one
				AtomicDec( &queue->m_uNumAccessors );
				queue->Release();
				queue = nullptr;

				// Loop again
				continue;
			}

			// Were we blocked?
			if( result == TaskQueue::kExecBlocked ) {
				// Check for a repeat incident
				if( lastBlockedQueue == queue ) {
					// This queue is blocking a lot so try a different one
					AtomicDec( &queue->m_uNumAccessors );
					queue = nullptr;
					//CThread::Yield();

					// Loop again
					continue;
				}

				// Set the last blocked queue to this queue
				lastBlockedQueue = queue;
			}
		}

		// We're no longer working on this queue
		if( queue != nullptr ) {
			AtomicDec( &queue->m_uNumAccessors );
		}

		// If we needed to exit while there were still queues to be worked upon
		// then those queues are now being aborted
		AcquireWriteLock();
		while( m_uBaseQueue != m_uLastQueue ) {
			m_pQueues[ m_uBaseQueue ]->Abort();
			m_pQueues[ m_uBaseQueue ]->Release();
			++m_uBaseQueue;
		}
		ReleaseWriteLock();

		// Done
		return EXIT_SUCCESS;
	}

	void LWorkerThread::AcquireWriteLock()
	{
		while( AtomicInc( &m_uWriteLock ) != 0 ) {
			AtomicDec( &m_uWriteLock );
			Spin( 4096 );
			//CThread::Yield();
		}
	}
	void LWorkerThread::ReleaseWriteLock()
	{
		AtomicDec( &m_uWriteLock );
	}

}}

#include "Scheduler03.hpp"
#include "Notify.hpp"
#include "../../System/TimeConversion.hpp"
#include "../../Core/Logger.hpp"

#if !AX_PROFILING_ENABLED && AX_ASYNC_FRAME_STEP_JOBS_ENABLED
# undef AX_ASYNC_FRAME_STEP_JOBS_ENABLED
# define AX_ASYNC_FRAME_STEP_JOBS_ENABLED 0
#endif

#if AX_ASYNC_LOCAL_WORK_ENABLED
# error Ax: Async: Local work is not supported (AX_ASYNC_LOCAL_WORK_ENABLED should be defined to 0)
#endif

namespace Ax { namespace Async {

#if AX_HAS_THREADLOCAL
	namespace Detail
	{

		AX_THREADLOCAL uint32		g_uWorkerId;

	}
#endif

	// Zero out an array
	template< typename tElement, uint32 tSize >
	static AX_FORCEINLINE void Zero( tElement( &arr )[ tSize ] )
	{
		memset( &arr, 0, sizeof( arr ) );
	}

	// Perform a linear allocation from an array
	template< typename tElement, uint32 tSize >
	static AX_FORCEINLINE tElement *LinearAlloc( tElement( &arr )[ tSize ], uint32 &current, uint32 count, const char *msg = "Allocation failed" )
	{
		AX_ASSERT( count > 0 );

		const uint32 base = AtomicAdd( &current, count );
		if( base + count > tSize ) {
			AX_ASSERT_MSG( false, msg );

			AtomicSub( &current, count );
			return nullptr;
		}

		return &arr[ base ];
	}

	/*
	===========================================================================

		WORKER

	===========================================================================
	*/

#if AX_PROFILING_ENABLED
	static const char *const g_pszWorkerNames[ 64 ] = {
		"CPU-1", "CPU-2", "CPU-3", "CPU-4", "CPU-5", "CPU-6", "CPU-7", "CPU-8",
		"CPU-9", "CPU10", "CPU11", "CPU12", "CPU13", "CPU14", "CPU15", "CPU16",
		"CPU17", "CPU18", "CPU19", "CPU20", "CPU21", "CPU22", "CPU23", "CPU24",
		"CPU25", "CPU26", "CPU27", "CPU28", "CPU29", "CPU30", "CPU31", "CPU32",

		"CPU33", "CPU34", "CPU35", "CPU36", "CPU37", "CPU38", "CPU39", "CPU40",
		"CPU41", "CPU42", "CPU43", "CPU44", "CPU45", "CPU46", "CPU47", "CPU48",
		"CPU49", "CPU50", "CPU51", "CPU52", "CPU53", "CPU54", "CPU55", "CPU56",
		"CPU57", "CPU58", "CPU59", "CPU60", "CPU61", "CPU62", "CPU63", "CPU64"
	};
#endif
	static const char *const g_pszThreadNames[ 64 ] = {
		"Worker 1" , "Worker 2" , "Worker 3" , "Worker 4" , "Worker 5" , "Worker 6" , "Worker 7" , "Worker 8" ,
		"Worker 9" , "Worker 10", "Worker 11", "Worker 12", "Worker 13", "Worker 14", "Worker 15", "Worker 16",
		"Worker 17", "Worker 18", "Worker 19", "Worker 20", "Worker 21", "Worker 22", "Worker 23", "Worker 24",
		"Worker 25", "Worker 26", "Worker 27", "Worker 28", "Worker 29", "Worker 30", "Worker 31", "Worker 32",

		"Worker 33", "Worker 34", "Worker 35", "Worker 36", "Worker 37", "Worker 38", "Worker 39", "Worker 40",
		"Worker 41", "Worker 42", "Worker 43", "Worker 44", "Worker 45", "Worker 46", "Worker 47", "Worker 48",
		"Worker 49", "Worker 50", "Worker 51", "Worker 52", "Worker 53", "Worker 54", "Worker 55", "Worker 56",
		"Worker 57", "Worker 58", "Worker 59", "Worker 60", "Worker 61", "Worker 62", "Worker 63", "Worker 64"
	};
	
	LWorker::LWorker( MScheduler &scheduler, uint32 workerId )
	: m_Scheduler( scheduler )
	, m_uWorkerId( workerId )
#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
	, m_FramePhase( EFramePhase::None )
#endif
#if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
	, m_bIsWorking( false )
#endif
	, m_HeadCPUTimestamp( 0 )
	, m_TailCPUTimestamp( 0 )
	, m_ExecutionState()
#if !AX_ASYNC_LOCAL_WORK_ENABLED
	, m_uChainIndex( 0 )
# if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
	, m_uLocalIndex( 0 )
# endif
#endif
	{
	}
	LWorker::~LWorker()
	{
	}

	uint64 LWorker::GetElapsedFrameCPUTime() const
	{
		return m_TailCPUTimestamp - m_HeadCPUTimestamp;
	}

#if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
	void LWorker::SubmitEnterFrameJob( SJobChain &chain )
	{
		AX_ASSERT_NOT_NULL( chain.pJobs );

		chain.pJobs[ chain.cJobs ].pfnKernel = &ReadCPUTimestampJob;
		chain.pJobs[ chain.cJobs ].pTaskData = &m_HeadCPUTimestamp;
		chain.pJobs[ chain.cJobs ].pDispatch = nullptr;

		++chain.cJobs;
	}
	void LWorker::SubmitLeaveFrameJob( SJobChain &chain )
	{
		AX_ASSERT_NOT_NULL( chain.pJobs );

		chain.pJobs[ chain.cJobs ].pfnKernel = &ReadCPUTimestampJob;
		chain.pJobs[ chain.cJobs ].pTaskData = &m_TailCPUTimestamp;
		chain.pJobs[ chain.cJobs ].pDispatch = nullptr;

		++chain.cJobs;
	}
#endif
	
#if AX_ASYNC_LOCAL_WORK_ENABLED
	bool LWorker::ExecuteUntilNoWork()
	{
		bool bDidWork = false;

# if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		m_bIsWorking = true;
		AX_MEMORY_BARRIER();
# else
		m_Scheduler.DecIdleWorker();
# endif
		for(;;) {
			// Index of the current arena (prior to decoding)
			const uint32 uLocalArenaIndex = m_uArenaIndex++;

# if AX_ASYNC_PER_ARENA_LOCK_ENABLED
# else
# endif
		}
# if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		m_bIsWorking = false;
		AX_MEMORY_BARRIER();
# else
		m_Scheduler.IncIdleWorker();
# endif

		return bDidWork;
	}
#else
	bool LWorker::ExecuteUntilNoWork()
	{
#if AX_PROFILING_ENABLED
		CProfileSampler Sampler;
#endif
		CAutoNotifyExecuteUntilNoWork Notification_ExecuteUntilNoWork_;

#if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
		const uint32 stride = m_Scheduler.m_cWorkers;
		const uint32 base = m_uWorkerId;

		//const uint32 originalChainIndex = m_uChainIndex;
		//const uint32 originalLocalIndex = m_uLocalIndex;
#endif
		
		const bool bDoWork = m_uChainIndex < m_Scheduler.m_cSubmittedChains;

#if AX_PROFILING_ENABLED
		if( bDoWork ) {
			Sampler.Enter( "ExecuteUntilNoWork - %s", g_pszWorkerNames[ m_uWorkerId ] );
		}
#endif

#if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		m_bIsWorking = true;
		AX_MEMORY_BARRIER();
#else
		m_Scheduler.DecIdleWorker();
#endif

		CNotifyFetchJob Notify_FetchJob_;
		CNotifyExecuteJob Notify_ExecJob_;
		CNotifyUpdateExecStats Notify_UpdateStats_;
		CNotifyDispatchDeferredWork Notify_LazyDispatch_;
		CNotifyUpdateJobChain Notify_UpdateChain_;

		while( m_uChainIndex < m_Scheduler.m_cSubmittedChains ) {
			SJobChain *const pChain = m_Scheduler.m_pSubmittedChains[ m_uChainIndex ];

			m_ExecutionState.LazyUpdate.uLastJobChain = m_uChainIndex;
			m_ExecutionState.LazyUpdate.LastJobChainMinJobCPUTime = uint64( -1 );
			m_ExecutionState.LazyUpdate.LastJobChainMaxJobCPUTime = 0;
			m_ExecutionState.LazyUpdate.LastJobChainTotalCPUTime = 0;

			AX_MEMORY_BARRIER();

			for(;;) {
				Notify_FetchJob_.Enter();
#if AX_ASYNC_PER_CHAIN_INDEX_ENABLED
				const uint32 index = AtomicInc( &pChain->uReadIndex );
#else
				const uint32 index = base + m_uLocalIndex*stride;
#endif
				Notify_FetchJob_.Leave();

				if( index >= pChain->cJobs ) {
					NotifyJobNotFetched();
					break;
				}

				NotifyJobFetched();

				Notify_ExecJob_.Enter();
				const uint64 jobCPUTime = pChain->pJobs[ index ].Execute( m_ExecutionState );
				Notify_ExecJob_.Leave();
#if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
				++m_uLocalIndex;
#endif

#if AX_PROFILING_ENABLED
				Notify_UpdateStats_.Enter();
				m_ExecutionState.LazyUpdate.AddJobCPUTime( jobCPUTime );
				Notify_UpdateStats_.Leave();
#else
				( void )jobCPUTime;
#endif
			}

			++m_uChainIndex;
#if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
			m_uLocalIndex = 0;
#endif

			Notify_LazyDispatch_.Enter();
			m_ExecutionState.LazyUpdate.CommitDispatch();
			Notify_LazyDispatch_.Leave();

			Notify_UpdateChain_.Enter();
			m_ExecutionState.LazyUpdate.CommitLastJobChain();
			Notify_UpdateChain_.Leave();
		}

#if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		m_bIsWorking = false;
		AX_MEMORY_BARRIER();
#else
		m_Scheduler.IncIdleWorker();
#endif

		//return originalChainIndex != m_uChainIndex || originalLocalIndex != m_uLocalIndex;
		return bDoWork;
	}
#endif

#if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
	void AX_JOB_API LWorker::ReadCPUTimestampJob( void *pData )
	{
		uint64 *pDstTimestamp = reinterpret_cast< uint64 * >( pData );
		AX_ASSERT_NOT_NULL( pDstTimestamp );

		*pDstTimestamp = GetCPUCycles();
	}
#endif

	void LWorker::Sleep()
	{
		AtomicInc( &m_Scheduler.m_cSleepingWorkers );
		m_Scheduler.m_WakeSemaphore.Wait();
		AtomicDec( &m_Scheduler.m_cSleepingWorkers );
	}



	/*
	===========================================================================

		WORKER THREAD

	===========================================================================
	*/

	// How long to do active polling for work before putting the thread to sleep
	static const uint32 kSleepThresholdMillisecs = 1000;

	LWorkerThread::LWorkerThread( LWorker &worker )
	: CThread()
	, m_Worker( worker )
	{
	}
	LWorkerThread::~LWorkerThread()
	{
	}
	int LWorkerThread::OnRun()
	{
		// LWorker ID needs to be set (0 is main thread)
		AX_ASSERT( m_Worker.m_uWorkerId > 0 );
		// LWorker ID must be valid
		AX_ASSERT( m_Worker.m_uWorkerId < 64 );

		// Apply the thread-local worker ID
		MScheduler::GetInstance().SetCurrentWorkerId( m_Worker.m_uWorkerId );
		// Set the name of this thread for debugging purposes
		SetName( g_pszThreadNames[ m_Worker.m_uWorkerId ] );

		// Backoff delay settings for querying work availability
		static const uint32 kMinCheckWorkSpins = 64;
		static const uint32 kMaxCheckWorkSpins = kBackoffMaxSpinCount;
		uint32 cCheckWorkSpins = kMinCheckWorkSpins;

		CNotifyFrame Notify_Frame_;

		// Main loop
		uint32 lastCheckedMillisecs = System::Milliseconds_LowLatency();
		while( !IsQuitting() ) {
#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
			if( m_Worker.m_FramePhase == EFramePhase::Enter ) {
				m_Worker.m_HeadCPUTimestamp = GetCPUCycles();
				m_Worker.m_FramePhase = EFramePhase::None;
				AX_MEMORY_BARRIER();
				Notify_Frame_.Enter();
			} else if( m_Worker.m_FramePhase == EFramePhase::Leave ) {
				m_Worker.m_TailCPUTimestamp = GetCPUCycles();
				m_Worker.m_FramePhase = EFramePhase::None;
				AX_MEMORY_BARRIER();
				Notify_Frame_.Leave();
			}
#endif

			// Determine whether it's time to sleep
			const uint32 currentMillisecs = System::Milliseconds_LowLatency();
			const uint32 elapsedMillisecs = currentMillisecs - lastCheckedMillisecs;
			if( elapsedMillisecs >= kSleepThresholdMillisecs ) {
				// There's no need to do an active loop if we haven't received work in this long...
				m_Worker.Sleep();
				lastCheckedMillisecs = System::Milliseconds_LowLatency();
				continue;
			}

			// Perform work
			if( m_Worker.ExecuteUntilNoWork() ) {
				// We performed work, so reset the sleep counter
				lastCheckedMillisecs = System::Milliseconds_LowLatency();

				// Gradually reduce the backoff delay since we have work available again
				if( cCheckWorkSpins >= kMinCheckWorkSpins*2 ) {
					cCheckWorkSpins /= 2;
				}

				// Loop again
				continue;
			} else {
				// Reduce contention by waiting for a little while before checking again
				Backoff( cCheckWorkSpins, kMaxCheckWorkSpins );
				NotifyIdleSpin();
			}
		} // Keep working until the thread is told to quit

		// Done
		return EXIT_SUCCESS;
	}


	
	/*
	===========================================================================

		SCHEDULER

	===========================================================================
	*/

	MScheduler &MScheduler::GetInstance()
	{
		static MScheduler instance;
		return instance;
	}

	MScheduler::MScheduler()
	: m_cWorkers( 0 )
#if !AX_HAS_THREADLOCAL && defined( _WIN32 )
	, m_dwWorkerIdTLS( TlsAlloc() )
#endif
	, m_cSleepingWorkers( 0 )
	, m_WakeSemaphore()
	, m_bIsQuitting( false )
	, m_cFrameChains( 0 )
	, m_cFrameTasks( 0 )
#if !AX_ASYNC_LOCAL_WORK_ENABLED
	, m_cSubmittedChains( 0 )
#endif
	, m_SubmitWriteAccessor()
	{
#if !AX_HAS_THREADLOCAL && defined( _WIN32 )
		if( !AX_VERIFY( m_dwWorkerIdTLS != TLS_OUT_OF_INDEXES ) ) {
			exit( EXIT_FAILURE );
		}
#endif

		Zero( m_pWorkers );
		Zero( m_FrameChains );
		Zero( m_FrameTasks );
#if AX_ASYNC_LOCAL_WORK_ENABLED
		Zero( m_Arenas );
# if !AX_ASYNC_PER_ARENA_LOCK_ENABLED
		for( uint32 i = 0; i < kNumArenas; ++i ) {
			m_pArenas[ i ] = &m_Arenas[ i ];
		}
# endif
#else
		Zero( m_pSubmittedChains );
#endif
	}
	MScheduler::~MScheduler()
	{
		Fini();

#if !AX_HAS_THREADLOCAL && defined( _WIN32 )
		TlsFree( m_dwWorkerIdTLS );
#endif
	}

	bool MScheduler::Init( uint32 numThreads )
	{
		static const uint32 cProcessors = GetCPUThreadCount();
		AX_ASSERT( cProcessors > 0 );

		const uint32 cDesiredThreads = numThreads > 0 ? numThreads : cProcessors;
		const uint32 cThreads = cDesiredThreads >= kMaxWorkers ? kMaxWorkers : cDesiredThreads;

		for( uint32 i = 0; i < cThreads; ++i ) {
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable:4316 ) //object allocated on the heap may not be aligned 64
#endif
			m_pWorkers[ i ] = new LWorker( *this, i );
#ifdef _MSC_VER
# pragma warning( pop )
#endif
			if( !AX_VERIFY_NOT_NULL( m_pWorkers[ i ] ) ) {
				while( i > 0 ) {
					delete m_pWorkers[ i - 1 ];
					--i;
				}

				return false;
			}
		}

#ifdef _WIN32
		// Peg the main thread to one specific processor core for QueryPerformanceCounter() improvement
		SetThreadAffinityMask( GetCurrentThread(), 1<<0 );
#endif

		m_pThreads[ 0 ] = nullptr;
		for( uint32 i = 1; i < cThreads; ++i ) {
			m_pThreads[ i ] = new LWorkerThread( *m_pWorkers[ i ] );
			if( !AX_VERIFY_NOT_NULL( m_pThreads[ i ] ) ) {
				while( i > 0 ) {
					delete m_pThreads[ i - 1 ];
					--i;
				}

				for( i = 0; i < cThreads; ++i ) {
					delete m_pWorkers[ i ];
				}

				return false;
			}

#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			const HANDLE hThread = m_pThreads[ i ]->GetNative_MSWin();
			SetThreadAffinityMask( hThread, DWORD_PTR( 1 )<<i );
#endif
		}

		m_cWorkers = cThreads;

		m_cSleepingWorkers = 0;
		m_bIsQuitting = false;
		m_cFrameChains = 0;
		m_cFrameTasks = 0;
#if !AX_ASYNC_LOCAL_WORK_ENABLED
		m_cSubmittedChains = 0;
#endif
#if !AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		m_cIdleWorkers = m_cWorkers;
#endif

		AX_MEMORY_BARRIER(); //must be set before threads start (this is just paranoia)

		for( uint32 i = 1; i < cThreads; ++i ) {
			m_pThreads[ i ]->Start();
		}

		return true;
	}
	void MScheduler::Fini()
	{
		if( !m_cWorkers ) {
			return;
		}

		// Do not allow further work to be submitted
		m_bIsQuitting = true;

#if !AX_ASYNC_LOCAL_WORK_ENABLED
		// Cut the current work load short
		m_SubmitWriteAccessor.Lock();
		m_cSubmittedChains = 0;
		m_SubmitWriteAccessor.Unlock();
#endif

		// Tell every thread that we're quitting
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			if( !m_pThreads[ i ] ) {
				continue;
			}

			m_pThreads[ i ]->SignalStop();
		}

		// Wake up all sleeping threads to have them exit gracefully
		if( m_cSleepingWorkers > 0 ) {
			m_WakeSemaphore.Signal( m_cSleepingWorkers );
		}

		// Wait for all jobs to complete (no threads processing work anymore)
		WaitForAllJobs();

		// Close each of the threads
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			if( !m_pThreads[ i ] ) {
				continue;
			}

			m_pThreads[ i ]->Stop();

			delete m_pThreads[ i ];
			m_pThreads[ i ] = nullptr;
		}

		// No more workers
		m_cWorkers = 0;
	}

	uint32 MScheduler::GetCurrentWorkerId() const
	{
#if AX_HAS_THREADLOCAL
		return Detail::g_uWorkerId;
#elif defined( _WIN32 )
		return ( uint32 )( uintptr )TlsGetValue( m_dwWorkerIdTLS );
#else
# error TODO: Find current worker identifier by inspecting the list for the current thread handle
#endif
	}
	void MScheduler::SetCurrentWorkerId( uint32 workerId )
	{
#if AX_HAS_THREADLOCAL
		Detail::g_uWorkerId = workerId;
#elif defined( _WIN32 )
		TlsSetValue( m_dwWorkerIdTLS, ( LPVOID )( uintptr )workerId );
#else
# error Function not implemented
#endif
	}

	void MScheduler::WaitForAllJobs()
	{
		// determine which worker thread we are (if we are one)
		const uint32 workerId = GetCurrentWorkerId();
		LWorker *const pWorker = workerId < m_cWorkers ? m_pWorkers[ workerId ] : nullptr;

		CAutoNotifyWaitWork Notification_WaitWork_;

		// work completion checking
		static const uint32 kMinCheckWorkingSpins = 16;
		static const uint32 kMaxCheckWorkingSpins = kBackoffMaxSpinCount;
		uint32 cCheckWorkingSpins = kMinCheckWorkingSpins;

		// if we are a worker then work until we have nothing else
		if( pWorker != nullptr ) {
			AX_ASSERT_MSG( workerId == 0, "Only the main thread or a non-worker thread can wait for jobs to complete" );

			// do work until all workers have no work to do
			do {
				//
				//	NOTE: We want to "work until no work" as other workers may
				//	-     be finishing the last of theirs to release
				//	-     dependencies into the queue.
				//
				if( !pWorker->ExecuteUntilNoWork() ) {
					NotifyIdleSpin();
					Backoff( cCheckWorkingSpins, kMaxCheckWorkingSpins );
				} else if( cCheckWorkingSpins > kMinCheckWorkingSpins*2 ) {
					cCheckWorkingSpins = cCheckWorkingSpins/2;
				}
			} while( IsWorking() );
		} else {
			// wait until all workers become idle as well -- do not sleep here
			while( IsWorking() ) {
				Backoff( cCheckWorkingSpins, kMaxCheckWorkingSpins );
			}
		}
	}
	bool MScheduler::IsWorking() const
	{
#if AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			if( m_pWorkers[ i ]->m_bIsWorking ) {
				return true;
			}
		}

		return false;
#else
		return m_cIdleWorkers < m_cWorkers;
#endif
	}

	//
	//	NOTE: When collecting profiling information we need to ignore the first
	//	-     and last events in the frame. (They should just be the required
	//	-     'frame synchronization' jobs for determining how long everything
	//	-     took.) Only one profiling event should be generated per worker.
	//

	void MScheduler::EnterFrame()
	{
		m_cFrameChains = 0;
		m_cFrameTasks = 0;

#if AX_ASYNC_LOCAL_WORK_ENABLED
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_uArenaIndex = i; //start checking here
		}
#else
		m_cSubmittedChains = 0;
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_uChainIndex = 0;
# if !AX_ASYNC_PER_CHAIN_INDEX_ENABLED
			m_pWorkers[ i ]->m_uLocalIndex = 0;
# endif
		}
#endif

		AX_MEMORY_BARRIER();

#if AX_PROFILING_ENABLED
		//
		//	Don't want to consider profiling overhead (to the maximum extent
		//	possible) as we're interested primarily in the performance of the
		//	actual content being scheduled.
		//

		ResetAllProfilePools();

# if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		SJobChain *const pFrameChain = AllocChains( 1 );
		AX_ASSERT_NOT_NULL( pFrameChain );

		pFrameChain->pJobs = AllocTasks( m_cWorkers );
		AX_ASSERT_NOT_NULL( pFrameChain->pJobs );

		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->SubmitEnterFrameJob( *pFrameChain );
		}

		m_HeadFrameMicrosecs = System::Microseconds();
		Submit( pFrameChain );
		//WaitForAllJobs();
# else
		m_HeadFrameMicrosecs = System::Microseconds();

#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		m_pWorkers[ 0 ]->m_HeadCPUTimestamp = GetCPUCycles();
		Detail_v1::Notify( Detail_v1::ENotify::Frame, Detail_v1::ENotifyPhase::Enter );
#endif

		for( uint32 i = 1; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_FramePhase = EFramePhase::Enter;
		}


		AX_MEMORY_BARRIER();
# endif
#else
		m_HeadFrameMicrosecs = System::Microseconds();
#endif
	}
	void MScheduler::LeaveFrame()
	{
#if AX_PROFILING_ENABLED
# if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		for( uint32 i = 1; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_FramePhase = EFramePhase::Leave;
		}

		AX_MEMORY_BARRIER();
# endif
#endif

#if !AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		m_pWorkers[ 0 ]->m_TailCPUTimestamp = GetCPUCycles();
		Detail_v1::Notify( Detail_v1::ENotify::Frame, Detail_v1::ENotifyPhase::Leave );
#endif

		m_TailFrameMicrosecs = System::Microseconds();

#if AX_PROFILING_ENABLED
# if AX_ASYNC_FRAME_STEP_JOBS_ENABLED
		SJobChain *const pFrameChain = AllocChains( 1 );
		AX_ASSERT_NOT_NULL( pFrameChain );

		pFrameChain->pJobs = AllocTasks( m_cWorkers );
		AX_ASSERT_NOT_NULL( pFrameChain->pJobs );

		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->SubmitLeaveFrameJob( *pFrameChain );
		}

		Submit( pFrameChain );
		WaitForAllJobs();
# endif
#endif

		//BasicDebugf( "MScheduler seconds = %.5f", System::MicrosecondsToSeconds( m_TailFrameMicrosecs - m_HeadFrameMicrosecs ) );
		//BasicDebugf( "Head = %u Tail = %u", ( unsigned int )m_HeadFrameMicrosecs, ( unsigned int )m_TailFrameMicrosecs );
	}

#if !AX_ASYNC_PER_WORKER_IDLE_FLAG_ENABLED
	void MScheduler::IncIdleWorker()
	{
		AtomicInc( &m_cIdleWorkers );
	}
	void MScheduler::DecIdleWorker()
	{
		AtomicDec( &m_cIdleWorkers );
	}
#endif

#if AX_PROFILING_ENABLED
	static inline uint64 CPUTimeToMicroseconds( uint64 cpuTime, uint64 microsecondsPerFrame, uint64 cpuTimePerFrame )
	{
		const double frameTime = double( cpuTime )/double( cpuTimePerFrame );
		const double microseconds = double( microsecondsPerFrame )*frameTime;
		return uint64( microseconds );
	}
#endif

	void MScheduler::CalculateExecStats()
	{
#if AX_PROFILING_ENABLED
		// should be consistent
		AX_ASSERT( m_HeadFrameMicrosecs <= m_TailFrameMicrosecs );
		// one for EnterFrame() and one for LeaveFrame()
		AX_ASSERT( m_cFrameChains >= 2 );

		const uint64 elapsedMicrosecs = m_TailFrameMicrosecs - m_HeadFrameMicrosecs;

		// Collect all of the frame times for the workers
		uint64 workerMicrosecs[ kMaxWorkers ];
		uint64 workerCPUTime[ kMaxWorkers ];
		for( uint32 i = 0; i < m_cWorkers; ++i ) {
			AX_ASSERT_NOT_NULL( m_pWorkers[ i ] );
			const uint64 elapsedCPUTime = m_pWorkers[ i ]->GetElapsedFrameCPUTime();
			workerCPUTime[ i ] = elapsedCPUTime;

			//
			//	TODO: Check the accuracy of these estimations
			//

			// Calculate the frame time in microseconds for the worker
			if( elapsedCPUTime >= 1ULL<<32 ) {
				const uint64 loCPUTime = elapsedCPUTime & 0xFFFFFFFF;
				const uint64 hiCPUTime = ( elapsedCPUTime >> 32 ) & 0xFFFFFFFF;

				const uint64 loMicrosecs = loCPUTime*1000000/elapsedMicrosecs;
				const uint64 hiMicrosecs = hiCPUTime*1000000/elapsedMicrosecs;

				workerMicrosecs[ i ] = ( hiMicrosecs<<32 ) | ( loMicrosecs );
			} else {
				workerMicrosecs[ i ] = elapsedCPUTime*1000000/elapsedMicrosecs;
			}
		}

		// Merge all of the stats
		for( uint32 i = 0; i < m_cFrameChains; ++i ) {
			m_ChainStats[ i ].Reset();

			// For each worker, convert the CPU time for their SExecStats to microseconds, then merge
			for( uint32 j = 0; j < m_cWorkers; ++j ) {
				const auto &cpuTimeStats = m_pWorkers[ j ]->m_ExecutionState.LazyUpdate.DeferredStats[ i ];
				SExecStats microsecStats;

				microsecStats.MinJobTime   = CPUTimeToMicroseconds( cpuTimeStats.MinJobTime  , elapsedMicrosecs, workerCPUTime[ j ] );
				microsecStats.MaxJobTime   = CPUTimeToMicroseconds( cpuTimeStats.MaxJobTime  , elapsedMicrosecs, workerCPUTime[ j ] );
				microsecStats.TotalJobTime = CPUTimeToMicroseconds( cpuTimeStats.TotalJobTime, elapsedMicrosecs, workerCPUTime[ j ] );
				microsecStats.TotalRunTime = CPUTimeToMicroseconds( cpuTimeStats.TotalRunTime, elapsedMicrosecs, workerCPUTime[ j ] );

				m_ChainStats[ i ].Merge( microsecStats );
			}
		}
#endif
	}

#if AX_ASYNC_LOCAL_WORK_ENABLED
	void MScheduler::Submit( SJobChain *pChains, uint32 cChains )
	{
		//
		//	TODO: Improve arena iteration for multiple chains
		//	-     Should treat the array of chains like one large job queue
		//

		AX_ASSERT_NOT_NULL( pChains );
		AX_ASSERT( cChains > 0 );

		// Current thread's worker ID
		const uint32 uWorkerId = GetCurrentWorkerId();
		AX_ASSERT( uWorkerId < m_cWorkers );

		// Pointer to the current thread's worker
		LWorker *pWorker = m_pWorkers[ uWorkerId ];
		AX_ASSERT_NOT_NULL( pWorker );

		static const uint32 kMinTaskThreshold = 4;

		// Maximum number of tasks to be queued per arena (not respecting the minimum threshold)
		const uint32 cDesiredTaskThreshold = kMaxArenaTasks/m_cWorkers;
		// Maximum number of tasks to be queued per arena
		const uint32 cTaskThreshold = cDesiredTaskThreshold < kMinTaskThreshold ? kMinTaskThreshold : cDesiredTaskThreshold;

		// Enumerate each chain
		for( uint32 i = 0; i < cChains; ++i ) {
			// The current chain
			SJobChain &Chain = pChains[ i ];

			// Desired number of tasks per arena (might be more than what an arena can actually hold)
			const uint32 cUnboundedDesiredWork = Chain.cJobs/m_cWorkers + Chain.cJobs%m_cWorkers;
			// Desired number of tasks per arena (respecting the per-arena limits)
			const uint32 cDesiredWork = cUnboundedDesiredWork < kMaxArenaTasks ? cUnboundedDesiredWork : kMaxArenaTasks;
			// Number of tasks per arena (respecting the threshold)
			const uint32 cWorkPerArena = cDesiredWork < cTaskThreshold ? cTaskThreshold : cDesiredWork;
			//const uint32 cArenas = cWorkPerArena/kMaxArenaTasks + ( cWorkPerArena%kMaxArenaTasks != 0 ? 1 : 0 );

			// Current job index within the chain
			uint32 uJobOffset = 0;
			// Number of jobs waiting to be queued from the chain
			uint32 cJobs = Chain.cJobs;

			//
			//	TODO: Check for the case where all arenas are completely filled
			//	-     If there are to many jobs then perform them
			//


			// Current arena index of the current worker
			const uint32 uBaseArena = pWorker->m_uArenaIndex;

			// Arena iterator
			uint32 j;

			// If we have a queue then add work to our queue first
			if( pWorker->m_pArena != nullptr ) {
				pWorker->m_pArena->WriteJobsFromChain( uJobOffset, cJobs, Chain );
				j = uBaseArena + 1;
			} else {
				j = uBaseArena;
			}

			// Add jobs while there are jobs to add
			while( cJobs > 0 ) {
#if AX_ASYNC_PER_ARENA_LOCK_ENABLED
				// Pointer to the current arena
				SArena *const pArena = &m_Arenas[ j%kNumArenas ];

				// Attempt to acquire exclusive access to the arena
				if( !pArena->TryLockWorker( pWorker ) ) {
					// Failed, so go to the next arena
					++j;
					continue;
				}
#else
				// Real index to the arena
				const uint32 uArenaIndex = MScheduler::DecodeArenaIndex( j%kNumArenas );

				// Pointer to the current arena
				SArena *const pArena = m_pArenas[ uArenaIndex ];

				// Attempt to acquire exclusive access to the arena
				if( !pArena || AtomicSetPtrEq( &m_pArenas[ uArenaIndex ], ( SArena * )0, pArena ) != ( SArena * )0 ) {
					// Failed, so go to the next arena
					++j;
					continue;
				}
#endif

				// Actually submit work to the arena
				pWorker->m_pArena->WriteJobsFromChain( uJobOffset, cJobs, Chain );

				// Release the lock to the arena
#if AX_ASYNC_PER_ARENA_LOCK_ENABLED
				pArena->UnlockWorker( pWorker );
#else
				AtomicSetPtr( &m_pArenas[ uArenaIndex ], pArena );
#endif

				// Go to the next arena
				++j;
			}
		}
	}
	void MScheduler::Submit( SJobChain *const *ppChains, uint32 cChains )
	{
		AX_ASSERT_NOT_NULL( ppChains );
		AX_ASSERT( cChains > 0 );

		for( uint32 i = 0; i < cChains; ++i ) {
			Submit( ppChains[ i ], 1 );
		}
	}
#else
	void MScheduler::Submit( SJobChain *pChains, uint32 cChains )
	{
		AX_ASSERT_NOT_NULL( pChains );
		AX_ASSERT( cChains > 0 );

		CAutoNotifySubmitWork Notification_SubmitWork_;

		// Make sure we won't go over the limit
		AX_ASSERT( m_cSubmittedChains + cChains < kMaxChains );

		// Don't do anything if we're supposed to be quitting
		AX_IF_UNLIKELY( m_bIsQuitting ) {
			return;
		}

		// Only one writer at a time
		m_SubmitWriteAccessor.Lock();
		for( uint32 i = 0; i < cChains; ++i ) {
			// Do processing (and verification) on the chain being submitted
			AX_ASSERT( pChains[ i ].cBlockers == 0 );
#if AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED
			pChains[ i ].cWorkers = m_cWorkers;
#endif
			pChains[ i ].pExecStats = &m_ChainStats[ m_cSubmittedChains ];
#if AX_ASYNC_PER_CHAIN_INDEX_ENABLED
			pChains[ i ].uReadIndex = 0;
#endif

			// Perform the actual submission
			m_pSubmittedChains[ m_cSubmittedChains ] = &pChains[ i ];
			AX_MEMORY_BARRIER();
			++m_cSubmittedChains;
		}
		m_SubmitWriteAccessor.Unlock();

		// There is now work available so if any workers are sleeping they need to be awakened
		WakeWorkers();
	}
	void MScheduler::Submit( SJobChain *const *ppChains, uint32 cChains )
	{
		AX_ASSERT_NOT_NULL( ppChains );
		AX_ASSERT( cChains > 0 );

#if AX_DEBUG_ENABLED
		// Verify the chains being submitted
		for( uint32 i = 0; i < cChains; ++i ) {
			AX_ASSERT_NOT_NULL( ppChains[ i ] );
			AX_ASSERT( ppChains[ i ]->cBlockers == 0 );
		}
#endif

		// Make sure we won't go over the limit
		AX_ASSERT( m_cSubmittedChains + cChains < kMaxChains );

		// Don't do anything if we're supposed to be quitting
		AX_IF_UNLIKELY( m_bIsQuitting ) {
			return;
		}

		// Only one writer at a time
		m_SubmitWriteAccessor.Lock();
		for( uint32 i = 0; i < cChains; ++i ) {
			// Do processing on the chain being submitted
#if AX_ASYNC_PER_CHAIN_WORKER_COUNT_ENABLED
			ppChains[ i ]->cWorkers = m_cWorkers;
#endif
			ppChains[ i ]->pExecStats = &m_ChainStats[ m_cSubmittedChains ];
#if AX_ASYNC_PER_CHAIN_INDEX_ENABLED
			ppChains[ i ]->uReadIndex = 0;
#endif

			// Perform the actual submission
			m_pSubmittedChains[ m_cSubmittedChains ] = ppChains[ i ];
			AX_MEMORY_BARRIER();
			++m_cSubmittedChains;
		}
		m_SubmitWriteAccessor.Unlock();

		// There is now work available so if any workers are sleeping they need to be awakened
		WakeWorkers();
	}
#endif

	SJob *MScheduler::AllocTasks( uint32 count )
	{
		SJob *const pJobs = LinearAlloc( m_FrameTasks, m_cFrameTasks, count, "Too many tasks allocated for this frame" );
		memset( ( void * )pJobs, 0, sizeof( SJob )*count );
		return pJobs;
	}
	SJobChain *MScheduler::AllocChains( uint32 count )
	{
		SJobChain *const pChains = LinearAlloc( m_FrameChains, m_cFrameChains, count, "Too many task chains allocated for this frame" );
		memset( ( void * )pChains, 0, sizeof( SJobChain )*count );
		return pChains;
	}

	void MScheduler::WakeWorkers()
	{
		const uint32 cSleepers = m_cSleepingWorkers;

		AX_IF_LIKELY( !cSleepers ) {
			return;
		}

		m_WakeSemaphore.Signal( m_cSleepingWorkers );
	}

}}

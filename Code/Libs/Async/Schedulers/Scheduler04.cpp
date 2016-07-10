#include "Scheduler04.hpp"

// See RJobChain::SetParent()
#ifndef AX_ASYNC_DESCENDANT_PARENTS_ENABLED
# define AX_ASYNC_DESCENDANT_PARENTS_ENABLED 0
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



	/*
	===========================================================================

		JOB CHAIN

	===========================================================================
	*/

	RJobChain::RJobChain()
	: m_pJobs( nullptr )
	, m_cJobs( 0 )
	, m_cRemaining( 0 )
	, m_uIndex( 0 )
	, m_Priority( EPriority::Normal )
	, m_pExecStats( nullptr )
	{
	}
	RJobChain::~RJobChain()
	{
	}
	void RJobChain::AddJobs( const SJob *pJobs, uint32 cJobs )
	{
		AX_ASSERT_NOT_NULL( pJobs );
		AX_ASSERT( cJobs > 0 );
#if AX_DEBUG_ENABLED
		for( uint32 i = 0; i < cJobs; ++i ) {
			AX_ASSERT_NOT_NULL( pJobs[ i ].pfnKernel );
		}
#endif
		AX_ASSERT_MSG( !WasSubmitted(), "Job chain has already been submitted" );

		uint32 cRealJobs = m_cJobs;
		while( cRealJobs > 0 ) {
			if( m_pJobs[ cRealJobs - 1 ].pfnKernel != nullptr ) {
				break;
			}

			--cRealJobs;
		}

		const uint32 cNeededJobs = cRealJobs + cJobs;
		if( cNeededJobs > m_cJobs ) {
			static const uint32 kAlign = 64;
			const uint32 cAlignedJobs = cNeededJobs - cNeededJobs%kAlign + kAlign;

			SJob *pNewJobs = Alloc< SJob >( cAlignedJobs );
			if( !AX_VERIFY_NOT_NULL( pNewJobs ) ) {
				exit( EXIT_FAILURE );
			}

			if( cRealJobs > 0 ) {
				memcpy( ( void * )pNewJobs, ( const void * )m_pJobs, cRealJobs*sizeof( SJob ) );
			}

			const uint32 uOffset = cNeededJobs;
			const uint32 cBlanks = cAlignedJobs - uOffset;
			if( cBlanks > 0 ) {
				memset( &pNewJobs[ uOffset ], 0, cBlanks*sizeof( SJob ) );
			}

			Dealloc( m_pJobs );
			m_pJobs = pNewJobs;
			m_cJobs = cAlignedJobs;
		}

		memcpy( &m_pJobs[ cRealJobs ], pJobs, cJobs*sizeof( SJob ) );
	}

	bool RJobChain::IsDescendant( const RJobChain &Ancestor ) const
	{
		AX_ASSERT( this != &Ancestor );

		const RJobChain *const pAncestor = &Ancestor;
		const RJobChain *pTest = m_pParent;
		while( pTest != nullptr ) {
			if( pTest == pAncestor ) {
				return true;
			}
		}

		return false;
	}
	void RJobChain::SetParent( RJobChain *pParent )
	{
		AX_ASSERT( pParent != this );
		AX_ASSERT_MSG( !WasSubmitted(), "Job chain has already been submitted" );

		//
		//	We want job chain creation to be quick, so the following code is
		//	somewhat obscured to be mindful of shared branches to help the
		//	compiler better optimize this section.
		//

		if( m_pParent != nullptr ) {
			if( m_pPrev != nullptr ) {
				m_pPrev->m_pNext = m_pNext;
			} else {
				m_pParent->m_pHead = m_pNext;
			}

			if( m_pNext != nullptr ) {
				m_pNext->m_pPrev = m_pPrev;
			} else {
				m_pParent->m_pTail = m_pPrev;
			}
		} else {
			if( m_pPrev != nullptr ) {
				m_pPrev->m_pNext = m_pNext;
			}

			if( m_pNext != nullptr ) {
				m_pNext->m_pPrev = m_pPrev;
			}
		}

		if( pParent != nullptr ) {
			// Unless it's needed, the case where a new parent is a descendant
			// of the item being set should not be handled. (This is probably a
			// bug anyway.)
#if AX_ASYNC_DESCENDANT_PARENTS_ENABLED
			if( pParent->IsDescendant( *this ) ) {
				pParent->SetParent( m_pParent );
			}
#else
			AX_ASSERT( !pParent->IsDescendant( *this ) );
#endif

			m_pPrev = pParent->m_pTail;
			m_pNext = nullptr;

			if( m_pPrev != nullptr ) {
				m_pPrev->m_pNext = this;
			} else {
				pParent->m_pHead = this;
			}
			pParent->m_pTail = this;
		} else {
			m_pPrev = nullptr;
			m_pNext = nullptr;
		}

		m_pParent = pParent;
	}
		
	void RJobChain::SetPriority( EPriority Priority )
	{
		AX_ASSERT( uint32( Priority ) < kNumPriorityLevels );
		m_Priority = Priority;
	}

	void RJobChain::SetJobKernel( uint32 uIndex, FnJob pfnKernel )
	{
		AX_ASSERT( uIndex < m_cJobs );
		m_pJobs[ uIndex ].pfnKernel = pfnKernel;
	}
	void RJobChain::SetJobData( uint32 uIndex, void *pTaskData )
	{
		AX_ASSERT( uIndex < m_cJobs );
		m_pJobs[ uIndex ].pTaskData = pTaskData;
	}

	void RJobChain::FixForSubmit()
	{
		AX_ASSERT_NOT_NULL( m_pJobs );
		AX_ASSERT( m_cJobs > 0 );
		AX_ASSERT( m_pPrev == nullptr || m_pParent != nullptr ); //if prev is set then we are a child
		AX_ASSERT( m_pNext == nullptr || m_pParent != nullptr ); //if next is set then we are a child
		AX_ASSERT( m_pHead == nullptr || m_pHead->m_pParent == this ); //if we have a child we must own it
		AX_ASSERT( m_pTail == nullptr || m_pTail->m_pParent == this ); //no kidnapping!

		while( m_cJobs > 0 ) {
			if( m_pJobs[ m_cJobs - 1 ].pfnKernel != nullptr ) {
				break;
			}

			--m_cJobs;
		}

		m_cRemaining = m_cJobs;
		AX_ASSERT( m_cRemaining > 0 );

		for( RJobChain *pChild = m_pHead; pChild != nullptr; pChild = pChild->m_pNext ) {
			pChild->FixForSubmit();
		}
	}
	void RJobChain::ExecuteJobs( const LWorker &Worker, SWorkerExecutionState &WorkerState )
	{
		//
		//	We want to reduce contention on cache-lines while keeping read
		//	speeds high. The naive approach to reducing cache-line contention is
		//	to separate the reads by a certain stride such that a new cache-line
		//	is read in each time. This prevents false sharing to a degree, but
		//	at the expense of read performance.
		//
		//	The approach taken here is to implicitly separate workers into areas
		//	within the job chain. Contention is still possible but in a well-
		//	balanced work-load it's not a problem as the contention level will
		//	be much lower than usual.
		//
		//	Two for-loops are used here, rather than a modulus operator, to help
		//	the compiler keep output optimized.
		//
		//	--------------------------------------------------------------------
		//
		//	Potential other methods include:
		//
		//	 * Separate into "guarded" areas upon submission based on the number
		//	 - of workers and some minimum boundary. Workers will prefer to stay
		//	 - within their own areas until all other work in the chain is
		//	 - exhausted.
		//
		//	--------------------------------------------------------------------
		//
		//	A job is marked as "fetched" if its uKernel member is set to
		//	SJob::kClaimed.
		//

		static_assert( sizeof( SJob ) < AX_CACHE_SIZE, "SJob is not small enough or AX_CACHE_SIZE is not reasonable" );
		static const uint32 kJobsPerCacheline = AX_CACHE_SIZE/sizeof( SJob );
		static const uint32 kJobsPerFence = kJobsPerCacheline*2;

		if( !m_cRemaining ) {
			return;
		}

		const uint32 uChainIndex = m_uIndex;
		const uint32 cJobs = m_cJobs;
		const uint32 uOffset = Worker.GetId()*kJobsPerFence;

		uint64 PrevTimestamp;

		uint64 MaxJobCPUTime = 0;
		uint64 MinJobCPUTime = ~uint64( 0 );
		uint64 TotalCPUTime = 0;
		uint32 cExecuted = 0;

#define EXECJOB__()\
			volatile FnJob pfnKernel = m_pJobs[ i ].pfnKernel;\
			if( pfnKernel == ( FnJob )SJob::kClaimed ) {\
				continue;\
			}\
			\
			pfnKernel = ( FnJob )AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR\
			(\
				&m_pJobs[ i ].pfnKernel,\
				( FnJob )SJob::kClaimed,\
				pfnKernel\
			);\
			AX_MEMORY_BARRIER();\
			if( ( void * )pfnKernel == ( void * )SJob::kClaimed || !pfnKernel ) {\
				continue;\
			}\
			\
			PrevTimestamp = GetCPUCycles();\
			pfnKernel( m_pJobs[ i ].pTaskData );\
			const uint64 ElapsedTime = GetCPUCycles() - PrevTimestamp;\
			if( MaxJobCPUTime < ElapsedTime ) {\
				MaxJobCPUTime = ElapsedTime;\
			}\
			if( MinJobCPUTime > ElapsedTime ) {\
				MinJobCPUTime = ElapsedTime;\
			}\
			TotalCPUTime += ElapsedTime;\
			++cExecuted

		for( uint32 i = uOffset; i < cJobs; ++i ) {
			EXECJOB__();
		}
		for( uint32 i = 0; i < uOffset; ++i ) {
			EXECJOB__();
		}

#undef EXECJOB__

		auto &Stats = WorkerState.LazyUpdate.DeferredStats[ uChainIndex ];
		if( Stats.MaxJobTime < MaxJobCPUTime ) {
			Stats.MaxJobTime = MaxJobCPUTime;
		}
		if( Stats.MinJobTime > MaxJobCPUTime ) {
			Stats.MinJobTime = MinJobCPUTime;
		}
		Stats.TotalJobTime += TotalCPUTime;
		Stats.TotalRunTime += TotalCPUTime;

		if( AX_ATOMIC_FETCH_SUB_REL32( &m_cRemaining, cExecuted ) == cExecuted ) {
			static const uint32 kMaxDeps = 8;
			MScheduler &Sched = MScheduler::GetInstance();

			RJobChain *pDeps[ kMaxDeps ];
			RJobChain *pDep;
			uint32 uIndex;

			uIndex = 0;
			pDep = m_pHead;
			while( pDep != nullptr ) {
				pDeps[ uIndex++ ] = pDep;
				pDep = pDep->m_pNext;

				if( uIndex == kMaxDeps ) {
					Sched.Submit( pDeps );
					uIndex = 0;
				}
			}

			if( uIndex > 0 ) {
				Sched.Submit( pDeps, uIndex );
			}
		}
	}

	bool RJobChain::WasSubmitted() const
	{
		return m_cRemaining != 0 || m_pExecStats != nullptr;
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
	
	LWorker::LWorker( uint32 workerId )
	: m_uWorkerId( workerId )
	, m_FramePhase( EFramePhase::None )
	, m_bIsWorking( false )
	, m_HeadCPUTimestamp( 0 )
	, m_TailCPUTimestamp( 0 )
	, m_ExecutionState()
	{
		Zero( m_uCurrentChain );
	}
	LWorker::~LWorker()
	{
	}

	uint64 LWorker::GetElapsedFrameCPUTime() const
	{
		return m_TailCPUTimestamp - m_HeadCPUTimestamp;
	}
	
	bool LWorker::ExecuteUntilNoWork()
	{
#if AX_PROFILING_ENABLED
		CProfileSampler Sampler;
#endif

		MScheduler &Sched = MScheduler::GetInstance();
		
		const bool bDoWork = Sched.CheckNewWork( m_uCurrentChain );
		if( !bDoWork ) {
			return false;
		}

		m_bIsWorking = true;
		AX_MEMORY_BARRIER();

#if AX_PROFILING_ENABLED
		Sampler.Enter( "ExecuteUntilNoWork - %s", g_pszWorkerNames[ m_uWorkerId ] );
#endif

		//
		//	[ A | B ] (2)
		//	
		//		#1: ( 0 + 0 )%2 = 0 (A)
		//		#2: ( 0 + 1 )%2 = 1 (B)
		//
		//		#1: ( 1 + 0 )%2 = 1 (B)
		//		#2: ( 1 + 1 )%2 = 0 (A)
		//
		//	[ A | B | C ] (3)
		//
		//		#1: ( 1 + 0 )%3 = 1 (B)
		//		#2: ( 1 + 1 )%3 = 2 (C)
		//
		//		#1: ( 2 + 0 )%3 = 2 (C)
		//		#2: ( 2 + 1 )%3 = 0 (A)
		//
		//	[ A | B | C | D ] (4)
		//
		//		#1: ( 2 + 0 )%4 = 2 (C)
		//		#2: ( 2 + 1 )%4 = 3 (D)
		//
		//		#2: ( 3 + 1 )%4 = 0 (A)
		//
		//	[ A | B | C | D | E ] (5)
		//
		//		#1: ( 2 + 0 )%5 = 2 (C)
		//		#2: ( 3 + 1 )%5 = 4 (E)
		//
		//		#1: ( 3 + 0 )%5 = 3 (D)
		//
		//		#1: ( 4 + 0 )%5 = 4 (E)
		//		#2: ( 4 + 1 )%5 = 0 (A)
		//

		for( uint32 uPriority = kNumPriorityLevels; uPriority > 0; --uPriority ) {
			// Current priority chain
			SSchedulerPriorityChain &PriorityChain = Sched.m_PriorityChains[ uPriority - 1 ];
			// Number of chains in the current priority level
			const uint32 cChains = PriorityChain.cChains;
			// Reference to the current chain index for this priority level
			uint32 &uChain = m_uCurrentChain[ uPriority - 1 ];

			// Run through the chains of the current priority level
			while( uChain < cChains ) {
				// Index of the chain to test (try to avoid contention by skipping chains)
				const uint32 i = ( uChain + m_uWorkerId ) % cChains;
				++uChain;

				PriorityChain.pChains[ i ]->ExecuteJobs( *this, m_ExecutionState );
			}

			// Check for new work of a higher priority level
			for( uint32 uCheck = kNumPriorityLevels; uCheck > uPriority; --uCheck ) {
				const uint32 i = uCheck - 1;
				if( m_uCurrentChain[ i ] < Sched.m_PriorityChains[ i ].cChains ) {
					// for-loop will decrement, so need + 1
					uPriority = uCheck + 1;
					break;
				}
			}
		}

		m_bIsWorking = false;
		AX_MEMORY_BARRIER();

		return bDoWork;
	}

	void LWorker::Sleep()
	{
		AtomicInc( &Tasks->m_cSleepingWorkers );
		Tasks->m_WakeSemaphore.Wait();
		AtomicDec( &Tasks->m_cSleepingWorkers );
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
		Tasks->SetCurrentWorkerId( m_Worker.m_uWorkerId );
		// Set the name of this thread for debugging purposes
		SetName( g_pszThreadNames[ m_Worker.m_uWorkerId ] );

		// Backoff delay settings for querying work availability
		static const uint32 kMinCheckWorkSpins = 64;
		static const uint32 kMaxCheckWorkSpins = kBackoffMaxSpinCount;
		uint32 cCheckWorkSpins = kMinCheckWorkSpins;

		bool bCanWork = false;

		// Main loop
		uint32 lastCheckedMillisecs = System::Milliseconds_LowLatency();
		while( !IsQuitting() ) {
			if( m_Worker.m_FramePhase == EFramePhase::Enter ) {
				m_Worker.m_HeadCPUTimestamp = GetCPUCycles();
				m_Worker.m_FramePhase = EFramePhase::None;
				AX_MEMORY_BARRIER();
				bCanWork = true;
				lastCheckedMillisecs = System::Milliseconds_LowLatency();
			} else if( m_Worker.m_FramePhase == EFramePhase::Leave ) {
				m_Worker.m_TailCPUTimestamp = GetCPUCycles();
				m_Worker.m_FramePhase = EFramePhase::None;
				AX_MEMORY_BARRIER();
				bCanWork = false;
				lastCheckedMillisecs = System::Milliseconds_LowLatency();
			}

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
			if( bCanWork && m_Worker.ExecuteUntilNoWork() ) {
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
	, m_HeadFrameMicrosecs( 0 )
	, m_TailFrameMicrosecs( 0 )
	, m_cSubmittedChains( 0 )
	{
#if !AX_HAS_THREADLOCAL && defined( _WIN32 )
		if( !AX_VERIFY( m_dwWorkerIdTLS != TLS_OUT_OF_INDEXES ) ) {
			exit( EXIT_FAILURE );
		}
#endif

		Zero( m_pWorkers );
		Zero( m_pThreads );
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
			m_pWorkers[ i ] = new LWorker( i );
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
		static const uint32 kMaxProcessors = kMaxWorkers;
		const uint32 uProcessorNumber = GetCurrentProcessorNumber();
		// Peg the main thread to one specific processor core for QueryPerformanceCounter() improvement
		SetThreadAffinityMask( GetCurrentThread(), DWORD_PTR( 1 )<<uProcessorNumber );
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
# if 0
			SetThreadAffinityMask( hThread, DWORD_PTR( 1 )<<( ( i + uProcessorNumber )%kMaxProcessors ) );
# else
			SetThreadIdealProcessor( hThread, ( i + uProcessorNumber )%kMaxProcessors );
# endif
#endif
		}

		m_cWorkers = cThreads;

		m_cSleepingWorkers = 0;
		m_bIsQuitting = false;
		m_cSubmittedChains = 0;

		Zero( m_PriorityChains );

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
		const uint32 uWorkerId = GetCurrentWorkerId();
		LWorker *const pWorker = uWorkerId < m_cWorkers ? m_pWorkers[ uWorkerId ] : nullptr;

		// work completion checking
		static const uint32 kMinCheckWorkingSpins = 16;
		static const uint32 kMaxCheckWorkingSpins = kBackoffMaxSpinCount;
		uint32 cCheckWorkingSpins = kMinCheckWorkingSpins;

		// if we are a worker then work until we have nothing else
		if( pWorker != nullptr ) {
			AX_ASSERT_MSG( uWorkerId == 0, "Only the main thread or a non-worker thread can wait for jobs to complete" );

			// do work until all workers have no work to do
			do {
				//
				//	NOTE: We want to "work until no work" as other workers may
				//	-     be finishing the last of theirs to release
				//	-     dependencies into the queue.
				//
				if( !pWorker->ExecuteUntilNoWork() ) {
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

	void MScheduler::EnterFrame()
	{
		m_cSubmittedChains = 0;
		for( uint32 i = 0; i < kNumPriorityLevels; ++i ) {
			const uint32 n = m_PriorityChains[ i ].cChains*uint32( sizeof( RJobChain * ) );
			if( !n ) {
				continue;
			}

			m_PriorityChains[ i ].cChains = 0;
			memset( &m_PriorityChains[ i ].pChains[ 0 ], 0, n );

			for( uint32 j = 0; j < m_cWorkers; ++j ) {
				m_pWorkers[ j ]->m_uCurrentChain[ i ] = 0;
			}
		}

		AX_MEMORY_BARRIER();

#if AX_PROFILING_ENABLED
		//
		//	Don't want to consider profiling overhead (to the maximum extent
		//	possible) as we're interested primarily in the performance of the
		//	actual content being scheduled.
		//

		ResetAllProfilePools();
#endif

		m_HeadFrameMicrosecs = System::Microseconds();

		for( uint32 i = 1; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_FramePhase = EFramePhase::Enter;
		}

		AX_MEMORY_BARRIER();
	}
	void MScheduler::LeaveFrame()
	{
		for( uint32 i = 1; i < m_cWorkers; ++i ) {
			m_pWorkers[ i ]->m_FramePhase = EFramePhase::Leave;
		}

		AX_MEMORY_BARRIER();
		WakeWorkers();

		uint32 uWorker = 1;
		while( uWorker < m_cWorkers ) {
			while( m_pWorkers[ uWorker ]->m_FramePhase != EFramePhase::None ) {
				LocalSpin();
			}

			++uWorker;
		}
		
		m_pWorkers[ 0 ]->m_TailCPUTimestamp = GetCPUCycles();
		m_TailFrameMicrosecs = System::Microseconds();
		
#if AX_PROFILING_ENABLED
		CalculateExecStats();
#endif

		//BasicDebugf( "MScheduler seconds = %.5f", System::MicrosecondsToSeconds( m_TailFrameMicrosecs - m_HeadFrameMicrosecs ) );
		//BasicDebugf( "Head = %u Tail = %u", ( unsigned int )m_HeadFrameMicrosecs, ( unsigned int )m_TailFrameMicrosecs );
	}

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
		const uint32 cWorkers = m_cWorkers;
		const uint32 cChains = m_cSubmittedChains;
		const uint64 elapsedMicrosecs = m_TailFrameMicrosecs - m_HeadFrameMicrosecs;

		// Collect all of the frame times for the workers
		uint64 workerMicrosecs[ kMaxWorkers ];
		uint64 workerCPUTime[ kMaxWorkers ];
		for( uint32 i = 0; i < cWorkers; ++i ) {
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
		for( uint32 i = 0; i < cChains; ++i ) {
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

	static bool IsValidPriority( EPriority Priority )
	{
#if 0
		switch( Priority )
		{
		case EPriority::Lowest:
		case EPriority::Lower:
		case EPriority::Low:
		case EPriority::Normal:
		case EPriority::High:
		case EPriority::Higher:
		case EPriority::Highest:
			return true;
		}

		return false;
#else
		return uint32( Priority ) < kNumPriorityLevels;
#endif
	}

	void MScheduler::Submit( RJobChain *pChains, uint32 cChains )
	{
		static const uint32 kMaxChainsPerSubmit = 16;
		RJobChain *pNewChains[ kMaxChainsPerSubmit ];

		uint32 cNewChains = 0;
		for( uint32 i = 0; i <= cChains; ++i ) {
			if( cNewChains == kMaxChainsPerSubmit || i == cChains ) {
				Submit( pNewChains, cNewChains );
				cNewChains = 0;

				if( i == cChains ) {
					break;
				}
			}

			pNewChains[ cNewChains++ ] = &pChains[ i ];
		}
	}
	void MScheduler::Submit( RJobChain *const *ppChains, uint32 cChains )
	{
		AX_ASSERT_NOT_NULL( ppChains );

		if( !cChains ) {
			return;
		}

		const uint32 uBase = AtomicAdd( &m_cSubmittedChains, cChains );
		if( !AX_VERIFY_MSG( uBase + cChains <= kMaxChains, "Too many chains" ) ) {
#if AX_DEBUG_ENABLED && defined( _MSC_VER )
			__debugbreak();
#endif
			return;
		}

		for( uint32 i = 0; i < cChains; ++i ) {
			AX_ASSERT_NOT_NULL( ppChains[ i ] );
			AX_ASSERT( IsValidPriority( ppChains[ i ]->m_Priority ) );

			ppChains[ i ]->FixForSubmit();
			ppChains[ i ]->m_uIndex = uBase + i;
			ppChains[ i ]->m_pExecStats = &m_ChainStats[ uBase + i ];
		}

		for( uint32 i = 0; i < cChains; ++i ) {
			const uint32 uPriorityIndex = uint32( ppChains[ i ]->m_Priority );
			SubmitToPriorityChain( m_PriorityChains[ uPriorityIndex ], *ppChains[ i ] );
		}
	}

	bool MScheduler::IsWorking() const
	{
		const uint32 cWorkers = m_cWorkers;
		for( uint32 i = 1; i < cWorkers; ++i ) {
			if( m_pWorkers[ i ]->m_bIsWorking ) {
				return true;
			}
		}

		AX_ASSERT( cWorkers > 0 );
		if( m_pWorkers[ 0 ]->m_bIsWorking ) {
			return true;
		}

		return false;
	}
	
	void MScheduler::WakeWorkers()
	{
		const uint32 cSleepers = m_cSleepingWorkers;

		AX_IF_LIKELY( !cSleepers ) {
			return;
		}

		m_WakeSemaphore.Signal( m_cSleepingWorkers );
	}
	void MScheduler::SubmitToPriorityChain( SSchedulerPriorityChain &PriorityChain, RJobChain &Chain )
	{
		//
		//	The idea here is that if you can change the next pointer from a NULL
		//	value to a non-NULL value then you've written to that slot and can
		//	increment the current count (which points to the next free slot).
		//
		//	If it fails to write a non-NULL value then something else has
		//	written to that slot, which will cause the current chain to
		//	increment, so we should try again.
		//
		//		NOTE: The above assumes that each priority chain sets its array
		//		-     to NULL before any jobs can be submitted. If that is not
		//		-     done then there's a possibility that this will loop
		//		-     indefinitely.
		//

		RJobChain *const pChain = &Chain;

		if( !AX_VERIFY_MSG( PriorityChain.cChains < kMaxChains, "Too many chains submitted" ) ) {
			return;
		}
		
		while( AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( &PriorityChain.pChains[ PriorityChain.cChains ], pChain, nullptr ) != nullptr ) {
			LocalSpin();

			AX_IF_UNLIKELY( m_bIsQuitting ) {
				return;
			}
			AX_IF_UNLIKELY( !AX_VERIFY_MSG( PriorityChain.cChains < kMaxChains, "Too many chains submitted" ) ) {
				return;
			}
		}

		AX_ATOMIC_FETCH_ADD_FULL32( &PriorityChain.cChains, 1 );
	}

	bool MScheduler::CheckNewWork( const uint32( &uCurrentChain )[ kNumPriorityLevels ] ) const
	{
		for( uint32 uIndex = kNumPriorityLevels; uIndex > 0; --uIndex ) {
			const uint32 i = uIndex - 1;
			if( uCurrentChain[ i ] < m_PriorityChains[ i ].cChains ) {
				return true;
			}
		}

		return false;
	}

}}

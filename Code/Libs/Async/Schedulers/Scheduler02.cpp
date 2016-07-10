#include "Scheduler02.hpp"

namespace Ax { namespace Async {

	/*
	===========================================================================

		SCHEDULER IMPLEMENTATION

	===========================================================================
	*/
	
	MScheduler &MScheduler::GetInstance()
	{
		static MScheduler instance;
		return instance;
	}

	MScheduler::MScheduler()
	: m_State()
	{
	}
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
		if( m_State.cWorkers > 0 ) {
			return true;
		}

		// Figure out how many threads is desired
		const uint32 cDesiredThreads = numThreads == 0 ? cCoreDist : numThreads;
		// Clamp the number of threads to the maximum
		const uint32 cThreads = cDesiredThreads > kMaxThreads ? kMaxThreads : cDesiredThreads;
		uint32 uThread;
		
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4316) // object allocated on the heap may not be aligned
#endif

		// Create each thread
		for( uThread = 0; uThread < cThreads; ++uThread ) {
			// Allocate
			m_State.pWorkers[ uThread ] = new WorkerState();
			if( !AX_VERIFY_NOT_NULL( m_State.pWorkers[ uThread ] ) ) {
				break;
			}
			m_State.pWorkers[ uThread ]->pThread = new LWorkerThread( m_State, uThread );
			if( !AX_VERIFY_NOT_NULL( m_State.pWorkers[ uThread ]->pThread ) ) {
				break;
			}
		}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

		// Clean-up on failure
		if( uThread != cThreads ) {
			// Destruct in reverse order
			while( uThread > 0 ) {
				delete m_State.pWorkers[ uThread ];
				m_State.pWorkers[ uThread ] = nullptr;

				--uThread;
			}

			// Mission failed
			return false;
		}

		// Book keeping
		m_State.cWorkers = cThreads;

		// Safe to start each thread now
		for( uThread = 0; uThread < cThreads; ++uThread ) {
			AX_ASSERT_NOT_NULL( m_State.pWorkers[ uThread ] );
			AX_ASSERT_NOT_NULL( m_State.pWorkers[ uThread ]->pThread );

			m_State.pWorkers[ uThread ]->pThread->Start();
		}

		// Done!
		return true;
	}
	void MScheduler::Fini()
	{
		// If we don't have any threads allocated then just return
		if( !m_State.cWorkers ) {
			return;
		}

		// Tell each thread to stop
		for( uint32 i = 0; i < m_State.cWorkers; ++i ) {
			m_State.pWorkers[ i ]->bQuit = true;
			m_State.pWorkers[ i ]->pThread->SignalStop();
		}

		// Wait for a moment
		CThread::Yield();

		// Delete each thread in reverse order
		while( m_State.cWorkers > 0 ) {
			// Calculate the index
			const uint32 i = m_State.cWorkers - 1;

			// Delete the thread
			delete m_State.pWorkers[ i ];
			m_State.pWorkers[ i ] = nullptr;

			// Set the new count (it's equal to the index)
			m_State.cWorkers = i;
		}
	}
	void MScheduler::BeginFrame()
	{
		m_State.BeginFrame();
	}
	void MScheduler::EndFrame()
	{
		m_State.EndFrame();
	}
	bool MScheduler::SubmitChains( TaskChain *const *ppChains, uintptr cChains )
	{
		AX_ASSERT( cChains <= kMaxQueues );
		m_State.Submit( ppChains, uint32( cChains ) );
		return true;
	}


	/*
	===========================================================================

		WORKER THREAD IMPLEMENTATION

	===========================================================================
	*/

	LWorkerThread::LWorkerThread( SchedulerState &scheduler, uint32 workerId )
	: m_Scheduler( scheduler )
	, m_Worker( *scheduler.pWorkers[ workerId ] )
	, m_WorkerId( workerId )
	{
	}
	LWorkerThread::~LWorkerThread()
	{
	}

	void LWorkerThread::Abort()
	{
		m_Worker.bQuit = true;
	}

	int LWorkerThread::OnRun()
	{
		while( !m_Worker.bQuit ) {
			m_Worker.Execute( m_Scheduler, m_WorkerId );
		}

		m_Worker.CleanUnfinishedWork();
		return EXIT_SUCCESS;
	}

}}

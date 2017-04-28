#include "Thread.hpp"
#include "../Core/Logger.hpp"
#include "../System/TimeConversion.hpp"

#if AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
# include <sys/types.h>
# include <unistd.h>
#endif

Ax::Async::CThread::CThread()
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
: m_hThread( NULL )
, m_uThreadId( 0 )
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
: m_Thread( (pthread_t)0 )
, m_ThreadId( 0 )
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::CThread ): Unhandled thread model
#endif
, m_bTerminate( false )
, m_iExitValue( EXIT_SUCCESS )
#if AX_DEBUG_ENABLED
, m_Name()
#endif
{
}
Ax::Async::CThread::~CThread()
{
	Stop();
}

#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS && AX_DEBUG_ENABLED
static const DWORD MS_VC_EXCEPTION = 0x406D1388;
# pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD							dwType;				// Must be 0x1000.
   LPCSTR							szName;				// Pointer to name (in user addr space).
   DWORD							dwThreadID;			// CThread ID (-1=caller thread).
   DWORD							dwFlags;			// Reserved for future use, must be zero.
} THREADNAME_INFO;
# pragma pack(pop)

static void SetThreadName( DWORD threadId, const char *name )
{
# if defined( _MSC_VER ) || defined( __INTEL_COMPILER )
	THREADNAME_INFO info;

	info.dwType = 0x1000;
	info.szName = name;
	info.dwThreadID = threadId;
	info.dwFlags = 0;

	__try {
		RaiseException( MS_VC_EXCEPTION, 0, sizeof( info )/sizeof( ULONG_PTR ), ( ULONG_PTR * )&info );
	AX_STATIC_SUPPRESS( 6320 )
	AX_STATIC_SUPPRESS( 6322 )
	} __except( EXCEPTION_EXECUTE_HANDLER ) {
	}
# else
	//
	//	TODO: Does GDB respect the MSVC thread naming? Does GDB have its own
	//	-     thread name system? Is there anything we can do for other
	//	-     debuggers?
	//
	( void )threadId;
	( void )name;
# endif
}
#endif

void Ax::Async::CThread::SetName( const char *name )
{
#if AX_DEBUG_ENABLED
	AX_VERIFY( m_Name.Assign( name ) == true );

# if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	if( m_uThreadId != 0 ) {
		SetThreadName( m_uThreadId, m_Name.CString() );
	}
# endif
#else
	( void )name;
#endif
}
const char *Ax::Async::CThread::GetName() const
{
#if AX_DEBUG_ENABLED
	return m_Name;
#else
	return "";
#endif
}

Ax::Async::CThread::EPriority Ax::Async::CThread::SetPriority( EPriority prio )
{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	AX_ASSERT( m_uThreadId != 0 );
	AX_ASSERT( m_hThread != NULL );
#endif

	const EPriority oldPrio = GetPriority();

#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	int realPrio;

	switch( prio ) {
	case EPriority::Idle:		realPrio = THREAD_PRIORITY_IDLE;			break;
	case EPriority::VeryLow:	realPrio = THREAD_PRIORITY_LOWEST;			break;
	case EPriority::Low:		realPrio = THREAD_PRIORITY_BELOW_NORMAL;	break;
	case EPriority::Normal:		realPrio = THREAD_PRIORITY_NORMAL;			break;
	case EPriority::High:		realPrio = THREAD_PRIORITY_ABOVE_NORMAL;	break;
	case EPriority::VeryHigh:	realPrio = THREAD_PRIORITY_HIGHEST;			break;
	case EPriority::Critical:	realPrio = THREAD_PRIORITY_TIME_CRITICAL;	break;
	default:
		realPrio = 0;
		AX_ASSERT_MSG( false, "Invalid priority parameter" );
		break;
	}

	if( !SetThreadPriority( m_hThread, realPrio ) ) {
# if AX_DEBUG_ENABLED
		if( !m_Name.IsEmpty() ) {
			AX_ERROR_LOG += "Failed to set thread \"" + m_Name + "\" priority";
		} else {
# endif
			AX_ERROR_LOG += "Failed to set thread priority";
# if AX_DEBUG_ENABLED
		}
# endif
	}
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	((void)prio);
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::SetPriority ): Unhandled thread model
#endif

	return oldPrio;
}
Ax::Async::CThread::EPriority Ax::Async::CThread::GetPriority() const
{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	AX_ASSERT( m_uThreadId != 0 );
	AX_ASSERT( m_hThread != NULL );

	const int prio = GetThreadPriority( m_hThread );
	if( prio == THREAD_PRIORITY_ERROR_RETURN ) {
# if AX_DEBUG_ENABLED
		if( !m_Name.IsEmpty() ) {
			AX_ERROR_LOG += "Failed to get thread \"" + m_Name + "\" priority";
		} else {
# endif
			AX_ERROR_LOG += "Failed to get thread priority";
# if AX_DEBUG_ENABLED
		}
# endif

		return EPriority::Normal;
	}

	switch( prio ) {
	case THREAD_PRIORITY_IDLE:			return EPriority::Idle;
	case THREAD_PRIORITY_LOWEST:		return EPriority::VeryLow;
	case THREAD_PRIORITY_BELOW_NORMAL:	return EPriority::Low;
	case THREAD_PRIORITY_NORMAL:		return EPriority::Normal;
	case THREAD_PRIORITY_ABOVE_NORMAL:	return EPriority::High;
	case THREAD_PRIORITY_HIGHEST:		return EPriority::VeryHigh;
	case THREAD_PRIORITY_TIME_CRITICAL:	return EPriority::Critical;
	default:
		if( prio < THREAD_PRIORITY_LOWEST ) {
			return EPriority::VeryLow;
		}

		if( prio > THREAD_PRIORITY_HIGHEST ) {
			return EPriority::VeryHigh;
		}

		AX_ASSERT_MSG( false, "This code should not be reachable" );
		break;
	}

	return EPriority::Normal;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	return EPriority::Normal;
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::GetPriority ): Unhandled thread model
#endif
}

bool Ax::Async::CThread::Start()
{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	if( m_uThreadId != 0 ) {
		return true;
	}

	m_bTerminate = false;
	m_iExitValue = EXIT_SUCCESS;

	m_hThread = CreateThread( nullptr, 0, &Main_f, ( void * )this, 0, &m_uThreadId );
	if( !m_hThread ) {
# if AX_DEBUG_ENABLED
		if( !m_Name.IsEmpty() ) {
			AX_ERROR_LOG += ( "Failed to create thread \"" + m_Name + "\"" );
		} else {
# endif
			AX_ERROR_LOG += "Failed to create thread";
# if AX_DEBUG_ENABLED
		}
# endif

		return false;
	}

# if AX_DEBUG_ENABLED
	if( !m_Name.IsEmpty() ) {
		SetThreadName( m_uThreadId, m_Name.CString() );
	}
# endif

	return true;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	if( m_Thread != ( pthread_t )0 ) {
		return true;
	}

	m_bTerminate = false;
	m_iExitValue = EXIT_SUCCESS;
	m_ThreadId = 0;

	if( pthread_create( &m_Thread, ( const pthread_attr_t * )0, &Main_f, ( void * )this ) != 0 ) {
# if AX_DEBUG_ENABLED
		if( !m_Name.IsEmpty() ) {
			AX_ERROR_LOG += ( "Failed to create thread \"" + m_Name + "\"" );
		} else {
# endif
			AX_ERROR_LOG += "Failed to create thread";
# if AX_DEBUG_ENABLED
		}
# endif

		return false;
	}

	return true;
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::Start ): Thread model not handled
#endif
}
Ax::threadResult_t AX_THREAD_CALL Ax::Async::CThread::Main_f( void *parm )
{
	CThread *const p = ( CThread * )parm;
	AX_ASSERT_NOT_NULL( p );

#if AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	p->m_ThreadId = getpid();
#endif

	p->m_iExitValue = p->OnRun();
	return threadResult_t( size_t( p->m_iExitValue ) );
}

void Ax::Async::CThread::SignalStop()
{
	m_bTerminate = true;
}
void Ax::Async::CThread::Stop()
{
	static const int maxThreadWaitTimeMS = 500;

#if AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	((void)maxThreadWaitTimeMS);

	if( !m_Thread ) {
		return;
	}
#endif

#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	if( !m_uThreadId ) {
		return;
	}
#endif

	m_bTerminate = true;
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	if( WaitForSingleObject( m_hThread, maxThreadWaitTimeMS ) != WAIT_OBJECT_0 ) {
# if AX_DEBUG_ENABLED
		if( !m_Name.IsEmpty() ) {
			AX_WARNING_LOG += "Thread \"" + m_Name + "\" did not exit quickly; forcing quit";
		} else {
# endif
			AX_WARNING_LOG += "Thread did not exit quickly; forcing quit";
# if AX_DEBUG_ENABLED
		}
# endif

		m_uThreadId = 0;

		CloseHandle( m_hThread );
		m_hThread = NULL;
	}
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	( void )pthread_cancel( m_Thread );
	( void )pthread_join( m_Thread, ( void ** )0 );
	m_Thread = ( pthread_t )0;
#endif
}
bool Ax::Async::CThread::IsQuitting() const
{
	return m_bTerminate;
}
bool Ax::Async::CThread::IsRunning() const
{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	return m_hThread != NULL && WaitForSingleObject( m_hThread, 0 ) != WAIT_OBJECT_0;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	int status;

	if( !m_ThreadId || waitpid( m_ThreadId, &status, WNOHANG | WUNTRACED ) == -1 ) {
		return 0;
	}

	if( WIFEXITED( status ) || WIFSIGNALED( status ) ) {
		return 0;
	}

	return 1;
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::IsRunning ): Thread model not handled
#endif
}
int Ax::Async::CThread::GetExitCode() const
{
	return m_iExitValue;
}
Ax::Async::SThreadTimes &Ax::Async::CThread::GetTimes( SThreadTimes &OutTimes ) const
{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
	static uint64 MainThreadCreationTime = 0;
	FILETIME CreationTime, ExitTime, KernelTime, UserTime;

	if( !MainThreadCreationTime ) {
		AX_EXPECT( GetProcessTimes( GetCurrentProcess(), &CreationTime, &ExitTime, &KernelTime, &UserTime ) == TRUE );

		MainThreadCreationTime = ( uint64( CreationTime.dwHighDateTime )<<32 ) | uint64( CreationTime.dwLowDateTime );
	}

	if( !GetThreadTimes( m_hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime ) ) {
		OutTimes.EnterTime = 0;
		OutTimes.LeaveTime = 0;
		OutTimes.KernelTime = 0;
		OutTimes.UserTime = 0;

		return OutTimes;
	}

	if( IsRunning() ) {
		ExitTime.dwHighDateTime = 0;
		ExitTime.dwLowDateTime = 0;
	}

	const uint64 EnterTime = ( uint64( CreationTime.dwHighDateTime )<<32 ) | uint64( CreationTime.dwLowDateTime );
	const uint64 LeaveTime = ( uint64( ExitTime.dwHighDateTime )<<32 ) | uint64( ExitTime.dwLowDateTime );

	const uint64 LocalEnterTime = EnterTime - MainThreadCreationTime;
	const uint64 LocalLeaveTime = LeaveTime - MainThreadCreationTime;
	const uint64 LocalKernelTime = ( uint64( KernelTime.dwHighDateTime )<<32 ) | uint64( KernelTime.dwLowDateTime );
	const uint64 LocalUserTime = ( uint64( UserTime.dwHighDateTime )<<32 ) | uint64( UserTime.dwLowDateTime );

	OutTimes.EnterTime = System::HundredNanosecondsToMicroseconds( LocalEnterTime );
	OutTimes.LeaveTime = System::HundredNanosecondsToMicroseconds( LocalLeaveTime );
	OutTimes.KernelTime = System::HundredNanosecondsToMicroseconds( LocalKernelTime );
	OutTimes.UserTime = System::HundredNanosecondsToMicroseconds( LocalUserTime );

	return OutTimes;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
	OutTimes.EnterTime = 0;
	OutTimes.LeaveTime = 0;
	OutTimes.KernelTime = 0;
	OutTimes.UserTime = 0;
	return OutTimes;
#else
# error AX_THREAD_MODEL( Ax::Async::CThread::GetTimes ): Thread model not handled
#endif
}

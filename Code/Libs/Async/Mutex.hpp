#pragma once

#include "Detail/Threading.hpp"

namespace Ax { namespace Async {

	// Mutual-exclusion (CMutex) synchronization primitive
	class CMutex
	{
	public:
									// Constructor
		inline						CMutex				()
									: m_Mutex()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										( void )InitializeCriticalSectionAndSpinCount( &m_Mutex, 4096 );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										static pthread_mutexattr_t attr;
										static bool didInit = false;

										if( !didInit ) {
											pthread_mutexattr_init( &attr );
											pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

											didInit = true;
										}

										pthread_mutex_init( &m_Mutex, &attr );
#else
# error AX_THREAD_MODEL( CMutex::CMutex() ): Unhandled thread model
#endif
									}
									// Destructor
		inline						~CMutex				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										DeleteCriticalSection( &m_Mutex );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_mutex_destroy( &m_Mutex );
#else
# error AX_THREAD_MODEL( CMutex::~CMutex() ): Unhandled thread model
#endif
									}
		
		AX_STATIC_SUPPRESS( 26135 )
									// Acquire a lock, waiting indefinitely
		inline void					Lock				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										EnterCriticalSection( &m_Mutex );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_mutex_lock( &m_Mutex );
#else
# error AX_THREAD_MODEL( CMutex::Lock() ): Unhandled thread model
#endif
									}
		AX_STATIC_SUPPRESS( 26135 )
									// Attempt to acquire a lock, returning immediately
		inline bool					TryLock				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										return TryEnterCriticalSection( &m_Mutex ) != false;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										return pthread_mutex_trylock( &m_Mutex ) == false;
#else
# error AX_THREAD_MODEL( CMutex::TryLock() ): Unhandled thread model
#endif
									}
		AX_STATIC_SUPPRESS( 26135 )
									// Release an acquired lock
		inline void					Unlock				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										LeaveCriticalSection( &m_Mutex );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_mutex_unlock( &m_Mutex );
#else
# error AX_THREAD_MODEL( CMutex::Unlock() ): Unhandled thread model
#endif
									}

	private:
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		CRITICAL_SECTION			m_Mutex;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
		pthread_mutex_t				m_Mutex;
#else
# error AX_THREAD_MODEL( CMutex ): Unhandled thread model
#endif
	};

	// Abstract scope-based locking class
	template< class MutexType >
	class LockGuard
	{
	public:
		typedef MutexType			mutex_type;

									// Acquire a lock (for automatic release on destruct)
		inline						LockGuard			( MutexType &m )
									: m_Mutex( m )
									{
										m_Mutex.Lock();
									}
									// Release the acquired lock
		inline						~LockGuard			()
									{
										m_Mutex.Unlock();
									}

									LockGuard			( const LockGuard & ) AX_DELETE_FUNC;
		LockGuard &					operator=			( const LockGuard & ) AX_DELETE_FUNC;

	private:
		mutex_type &				m_Mutex;
	};
	
	// Locks a CMutex on construct and unlocks on destruct
	typedef LockGuard< CMutex >		MutexGuard;

}}

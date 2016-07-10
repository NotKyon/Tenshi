#pragma once

#include "Detail/Threading.hpp"

namespace Ax { namespace Async {

	class CSignal
	{
	public:
									// Constructor
		inline						CSignal				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										m_hEvent = CreateEventW( 0, TRUE, FALSE, 0 );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_mutex_init( &m_Mutex, 0 );
										pthread_cond_init( &m_Condition, 0 );
#else
# error AX_THREAD_MODEL( CSignal::CSignal ): Unhandled thread model
#endif
									}
									// Destructor
		inline						~CSignal				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										if( !m_hEvent ) {
											return;
										}

										CloseHandle( m_hEvent );
										m_hEvent = nullptr;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_cond_destroy( &m_Condition );
										pthread_mutex_destroy( &m_Mutex );
#else
# error AX_THREAD_MODEL( CSignal::~CSignal ): Unhandled thread model
#endif
									}

									// Trigger the signal (mark it as "on" / "active")
		inline void					Trigger				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										SetEvent( m_hEvent );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_cond_signal( &m_Condition );
#else
# error AX_THREAD_MODEL( CSignal::Trigger ): Unhandled thread model
#endif
									}
									// Reset the signal (mark it as "off" / "deactivated")
		inline void					Reset				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										ResetEvent( m_hEvent );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										//
										//	TODO: Implement this
										//

										AX_ASSERT_MSG( false, "Unimplemented" )
#else
# error AX_THREAD_MODEL( CSignal::Reset ): Unhandled thread model
#endif
									}

									// Wait for this signal to be triggered ("on" / "active")
		inline void					Wait				()
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										WaitForSingleObject( m_hEvent, INFINITE );
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										pthread_mutex_lock( &m_Mutex );
										pthread_cond_wait( &m_Condition, &m_Mutex );
										pthread_mutex_unlock( &m_Mutex );
#else
# error AX_THREAD_MODEL( CSignal::Wait ): Unhandled thread model
#endif
									}
									// Wait for this signal to be triggered for the specified number of milliseconds
		inline bool					TimedWait			( uint32 milliseconds )
									{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
										return WaitForSingleObject( m_hEvent, milliseconds ) == WAIT_OBJECT_0;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
										struct timespec ts = {
											milliseconds/1000, ( milliseconds%1000 )*1000000
										};
										bool r = false;

										pthread_mutex_lock( &m_Mutex );
										r = pthread_cond_timedwait( &m_Condition, &m_Mutex, &ts ) == 0;
										pthread_mutex_unlock( &m_Mutex );

										return r;
#else
# error AX_THREAD_MODEL( CSignal::TimedWait ): Unhandled thread model
#endif
									}
									// Check to see whether this signal is triggered
		inline bool					TryWait				()
									{
										return TimedWait( 0 );
									}

	private:
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		HANDLE						m_hEvent;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
		pthread_mutex_t				m_Mutex;
		pthread_cond_t				m_Condition;
#else
# error AX_THREAD_MODEL( CSignal ): Unhandled thread model
#endif
	};

}}

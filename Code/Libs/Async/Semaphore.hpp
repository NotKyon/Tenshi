#pragma once

#include "Detail/Threading.hpp"
#include "../Core/Types.hpp"

#ifdef __APPLE__
# include <dispatch/dispatch.h>
#endif

namespace Ax { namespace Async {

	class CSemaphore
	{
	public:
		static const uint32			kMaxCount = 0x7FFFFFFFUL;

		// Constructor
		inline CSemaphore( uint32 baseCount = 0, uint32 maxCount = kMaxCount )
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		: m_hSemaphore( CreateSemaphoreW( NULL, ( LONG )baseCount, ( LONG )maxCount, NULL ) )
#elif defined(__APPLE__)
		: m_Semaphore( dispatch_semaphore_create( (long)baseCount ) )
#endif
		{
#if defined(__APPLE__)
			((void)maxCount);
#endif
		}
		// Destructor
		inline ~CSemaphore()
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			CloseHandle( m_hSemaphore );
#elif defined(__APPLE__)
			dispatch_release( m_Semaphore );
#endif
		}
		// Signal the semaphore (increases current count by the amount given)
		inline bool Signal( uint32 count = 1, uint32 *prevCount = nullptr )
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			return ReleaseSemaphore( m_hSemaphore, ( LONG )count, ( LONG * )prevCount ) == TRUE;
#elif defined(__APPLE__)
			if( prevCount != nullptr ) {
				return false;
			}

			while( count > 0 ) {
				dispatch_semaphore_signal( m_Semaphore );
			}
			return true;
#endif
		}
		// Wait for the semaphore to be signalled (decreases current count by one upon returning)
		inline void Wait()
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			WaitForSingleObject( m_hSemaphore, INFINITE );
#elif defined(__APPLE__)
			dispatch_semaphore_wait( m_Semaphore, DISPATCH_TIME_FOREVER );
#endif
		}
		// Wait for the semaphore to be signalled for a specific amount of time (decreases current count by one upon returning)
		inline bool TimedWait( uint32 milliseconds )
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			return WaitForSingleObject( m_hSemaphore, milliseconds ) == WAIT_OBJECT_0;
#elif defined(__APPLE__)
			return dispatch_semaphore_wait( m_Semaphore, dispatch_time( DISPATCH_TIME_NOW, ((int64)milliseconds)*1000000 ) ) == 0;
#endif
		}
		// Check to see whether this signal is triggered
		inline bool TryWait()
		{
			return TimedWait( 0 );
		}

	private:
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		HANDLE						m_hSemaphore;
#elif defined(__APPLE__)
		dispatch_semaphore_t        m_Semaphore;
#else
# error AX_THREAD_MODEL( CSemaphore ): Unhandled thread model
#endif
	};

}}

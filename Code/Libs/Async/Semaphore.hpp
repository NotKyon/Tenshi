#pragma once

#include "Detail/Threading.hpp"
#include "../Core/Types.hpp"

namespace Ax { namespace Async {

	class CSemaphore
	{
	public:
		static const uint32			kMaxCount = 0x7FFFFFFFUL;

		// Constructor
		inline CSemaphore( uint32 baseCount = 0, uint32 maxCount = kMaxCount )
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		: m_hSemaphore( CreateSemaphoreW( NULL, ( LONG )baseCount, ( LONG )maxCount, NULL ) )
#endif
		{
		}
		// Destructor
		inline ~CSemaphore()
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			CloseHandle( m_hSemaphore );
#endif
		}
		// Signal the semaphore (increases current count by the amount given)
		inline bool Signal( uint32 count = 1, uint32 *prevCount = nullptr )
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			return ReleaseSemaphore( m_hSemaphore, ( LONG )count, ( LONG * )prevCount ) == TRUE;
#endif
		}
		// Wait for the semaphore to be signalled (decreases current count by one upon returning)
		inline void Wait()
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			WaitForSingleObject( m_hSemaphore, INFINITE );
#endif
		}
		// Wait for the semaphore to be signalled for a specific amount of time (decreases current count by one upon returning)
		inline bool TimedWait( uint32 milliseconds )
		{
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
			return WaitForSingleObject( m_hSemaphore, milliseconds ) == WAIT_OBJECT_0;
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
#else
# error AX_THREAD_MODEL( CSemaphore ): Unhandled thread model
#endif
	};

}}

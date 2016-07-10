#pragma once

#include "Atomic.hpp"
#include "CPU.hpp"

#ifndef AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED
# define AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED 1
#endif

#if AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED
# include "Thread.hpp"
#endif

#include "../Core/Assert.hpp"
#include "../Core/Types.hpp"

#include "../System/HighPerformanceClock.hpp"

namespace Ax { namespace Async {

#if AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED
	static const uint32 kSemaphoreSleepResistanceMilliseconds = 128;
#endif

	/// Simple implementation of a semaphore which avoids OS scheduling
	class CQuickSemaphore
	{
	public:
		static const uint32 kMaxCount = 0x7FFFFFFFUL;

		/// Constructor
		inline CQuickSemaphore( uint32 baseCount = 0, uint32 maxCount = kMaxCount )
		: m_uCurrent( baseCount )
		, m_uMaximum( maxCount )
		{
			AX_ASSERT( baseCount <= maxCount );
		}
		/// Destructor
		inline ~CQuickSemaphore()
		{
		}
		/// Signal the semaphore (increases current count by the amount given)
		inline bool Signal( uint32 count = 1, uint32 *pPrevCount = nullptr )
		{
			uint32 cSpinCounter = 1;

			const uint32 maximumValue = m_uMaximum;
			for(;;) {
				const uint32 currentValue = m_uCurrent;
				const uint32 updatedValue = currentValue + count;
				if( updatedValue > maximumValue ) {
					return false;
				}

				if( AtomicSetEq( &m_uCurrent, updatedValue, currentValue ) == updatedValue ) {
					if( pPrevCount != nullptr ) {
						*pPrevCount = currentValue;
					}

					return true;
				}

				Backoff( cSpinCounter );
			}
		}
		/// Wait for the semaphore to be signalled (decreases current count by one upon returning)
		inline void Wait()
		{
#if AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED
			const uint32 baseMilliseconds = ReadMilliseconds();
			while( !TryWait() ) {
				const uint32 elapsed = ReadMilliseconds() - baseMilliseconds;
				if( elapsed >= kSemaphoreSleepResistanceMilliseconds ) {
					CThread::Yield();
				}

				LocalSpin();
			}
#else
			uint32 cSpinCounter = 1;

			while( !TryWait() ) {
				Backoff( cSpinCounter );
			}
#endif
		}
		/// Wait for the semaphore to be signalled for a specific amount of time (decreases current count by one upon returning)
		inline bool TimedWait( uint32 milliseconds )
		{
			uint32 cSpinCounter = 1;
			const uint32 baseMilliseconds = ReadMilliseconds();
			while( !TryWait() ) {
				const uint32 elapsed = ReadMilliseconds() - baseMilliseconds;
				if( elapsed >= milliseconds ) {
					return false;
				}

#if AX_ASYNC_QUICKSEMAPHORE_SLEEP_ENABLED
				if( elapsed > kSemaphoreSleepResistanceMilliseconds ) {
					CThread::Yield();
				}
#endif
				Backoff( cSpinCounter );
			}

			return true;
		}
		/// Check to see whether this signal is triggered (this will decrement upon successful return)
		inline bool TryWait()
		{
			const uint32 currentValue = m_uCurrent;
			if( !currentValue ) {
				return false;
			}

			const uint32 updatedValue = currentValue - 1;
			return AtomicSetEq( &m_uCurrent, updatedValue, currentValue ) == updatedValue;
		}

	private:
		volatile uint32				m_uCurrent;
		uint32						m_uMaximum;

		static inline uint32 ReadMilliseconds()
		{
			return System::Milliseconds_LowLatency();
		}
	};

}}

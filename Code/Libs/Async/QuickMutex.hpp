#pragma once

#include "Atomic.hpp"
#include "CPU.hpp"					// for LocalSpin() and kSpinCount

namespace Ax { namespace Async {

	/// Simple mutex implementation with a low-contention spin-lock
	class CQuickMutex
	{
	public:
		inline CQuickMutex()
		: m_uLock( 0 )
		{
		}
		inline ~CQuickMutex()
		{
		}

		/// Attempts to acquire the lock (as with TryAcquire()) but does not
		/// decrement if it failed
		inline bool TryAcquireUnsafe()
		{
			return AtomicInc( &m_uLock ) == 0;
		}
		/// Attempts to acquire the lock without waiting
		inline bool TryAcquire()
		{
			if( AtomicInc( &m_uLock ) != 0 ) {
				AtomicDec( &m_uLock );
				return false;
			}

			return true;
		}
		/// Acquires the lock
		inline void Acquire( uint32 spinCount = kSpinCount )
		{
			// Wait for access to the lock
			while( !TryAcquireUnsafe() ) {
				// Don't overload the lock
				Backoff( spinCount );

				// Otherwise we need to decrement
				Release();
			}
		}
		/// Releases the lock
		inline void Release()
		{
			AtomicDec( &m_uLock );
		}
		/// Retrieves the current value of the lock
		inline uint32 GetValue() const
		{
			return m_uLock;
		}

		
		// For compatibility with LockGuard<>

		inline void Lock()
		{
			Acquire();
		}
		inline void Unlock()
		{
			Release();
		}
		
	private:
		volatile uint32				m_uLock;

		CQuickMutex( const CQuickMutex & ) = delete;
		CQuickMutex &operator=( const CQuickMutex & ) = delete;
	};

}}

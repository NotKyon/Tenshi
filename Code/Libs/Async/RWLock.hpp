#pragma once

#include "Detail/Threading.hpp"
#include "Atomic.hpp"
#include "Thread.hpp"
#include "Mutex.hpp"

namespace Ax { namespace Async {

	// Single-writer multiple-reader synchronization primitive
	class CRWLock
	{
	public:
									// Maximum number of readers at any given time
		static const uintcpu		kMaxReaders			= 64;

									// Constructor
		inline						CRWLock				()
									: m_uReaderCount( 0 )
									, m_uWriterFetch( 0 )
									{
									}
									// Destructor
		inline						~CRWLock			()
									{
									}

									// Acquire read access
		inline void					ReadLock			()
									{
										for(;;) {
											AX_MEMORY_BARRIER();

											// try to gain access
											const uintcpu readers = AtomicInc( &m_uReaderCount ) + 1;
											if( readers >= kMaxReaders - 1 ) {
												AtomicDec( &m_uReaderCount );

												// don't lock the bus
												for( volatile uint32 i = 0; i < 2048; ++i ) {
												}

												continue;
											}

											// we gained access
											return;
										}
									}
									// Release read access
		inline void					ReadUnlock			()
									{
										AtomicDec( &m_uReaderCount );
									}
									// Acquire write access
		inline void					WriteLock			()
									{
										AX_MEMORY_BARRIER();

										// only one writer can acquire access at a time, to avoid deadlock
										AcquireWritePrivilege();
										AtomicAdd( &m_uReaderCount, kMaxReaders );
										while( m_uReaderCount > kMaxReaders ) {
										}
										ReleaseWritePrivilege();
									}
									// Release write access
		inline void					WriteUnlock			()
									{
										AtomicSub( &m_uReaderCount, kMaxReaders );
									}

	private:
		volatile uintcpu			m_uReaderCount;
		volatile uintcpu			m_uWriterFetch;

		inline void AcquireWritePrivilege()
		{
			while( AtomicInc( &m_uWriterFetch ) != 0 ) {
				AtomicDec( &m_uWriterFetch );
			}
		}
		inline void ReleaseWritePrivilege()
		{
			AtomicDec( &m_uWriterFetch );
		}
	};

	// Scope-based read-locker
	class ReadLockGuard
	{
	public:
		typedef CRWLock				mutex_type;

									// Acquire a lock (for automatic release on destruct)
		inline						ReadLockGuard		( CRWLock &m )
									: m_Mutex( m )
									{
										m_Mutex.ReadLock();
									}
									// Release the acquired lock
		inline						~ReadLockGuard		()
									{
										m_Mutex.ReadUnlock();
									}

									ReadLockGuard		( const ReadLockGuard & ) = delete;
		ReadLockGuard &				operator=			( const ReadLockGuard & ) = delete;

	private:
		mutex_type &				m_Mutex;
	};

	// Scope-based write-locker
	class WriteLockGuard
	{
	public:
		typedef CRWLock				mutex_type;

									// Acquire a lock (for automatic release on destruct)
		inline						WriteLockGuard		( CRWLock &m )
									: m_Mutex( m )
									{
										m_Mutex.WriteLock();
									}
									// Release the acquired lock
		inline						~WriteLockGuard		()
									{
										m_Mutex.WriteUnlock();
									}

									WriteLockGuard		( const WriteLockGuard & ) = delete;
		WriteLockGuard &			operator=			( const WriteLockGuard & ) = delete;

	private:
		mutex_type &				m_Mutex;
	};

}}

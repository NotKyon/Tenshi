#pragma once

#include "Detail/Threading.hpp"
#include "Atomic.hpp"
#include "Signal.hpp"

namespace Ax { namespace Async {

	// CSignal-like synchronization primitive that is triggered when its count
	// reaches the expected value.
	class CBarrier
	{
	public:
									// Constructor
		inline						CBarrier			( uintcpu exp = 0 )
									: m_uCurrent( 0 )
									, m_uTarget( exp )
									, m_Event()
									{
									}
									// Destructor
		inline						~CBarrier			()
									{
									}

									// Increment the current level, triggering the signal if it reaches the expected
									// level
		inline void					Raise				()
									{
										if( AtomicInc( &m_uCurrent ) >= m_uTarget - 1 ) {
											m_Event.Trigger();
										}
									}
									// Decrement the current level, resetting the signal if it goes below the expected
									// level
		inline void					Lower				()
									{
										if( AtomicDec( &m_uCurrent ) < m_uTarget ) {
											m_Event.Reset();
										}
									}

									// Clear the current level and reset the signal
		inline void					Clear				()
									{
										AtomicSet( &m_uCurrent, 0 );
										m_Event.Reset();
									}
									// Clear the current level, reset the signal, and set the expected level
		inline void					Reset				( uintcpu exp )
									{
										AtomicSet( &m_uTarget, exp );
										AtomicSet( &m_uCurrent, 0 );
										m_Event.Reset();
									}
									// Increase the expected level by 1
		inline void					IncreaseTarget		()
									{
										AtomicInc( &m_uTarget );
										if( m_uCurrent < m_uTarget ) {
											m_Event.Reset();
										}
									}
									// Wait indefinitely for the signal to be triggered
		inline void					Wait				()
									{
										m_Event.Wait();
									}
									// Wait for a certain amount of time for the signal to be triggered
		inline bool					TimedWait			( uintcpu milliseconds )
									{
										return m_Event.TimedWait( milliseconds );
									}
									// Check whether the signal is triggered and return immediately if it's not
		inline bool					TryWait				()
									{
										return m_Event.TryWait();
									}

	private:
		uintcpu						m_uCurrent, m_uTarget;
		CSignal						m_Event;
	};

}}

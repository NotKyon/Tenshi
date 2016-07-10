#pragma once

#include "HighPerformanceClock.hpp"

namespace Ax { namespace System {

	// Basic timing information
	class CTimer
	{
	public:
							// Constructor
		inline				CTimer						()
							: m_uBase( Microseconds() )
							, m_uFreq( 1 )
							{
							}
							// Destructor
		inline				~CTimer						()
							{
							}

							// Reset the timer to zero
		inline void			Reset						()
							{
								m_uBase = Microseconds();
							}
							// Reset the timer to the nearest frequency boundary
		inline void			CoalescedReset				()
							{
								const uint64 t = Microseconds();
								m_uBase = t - t%m_uFreq;
							}

							// Set the frequency in seconds
		inline void			SetFrequencySeconds			( double sec )
							{
								m_uFreq = uint64( sec*1000000.0 );
							}
							// Retrieve the frequency in seconds
		inline double		GetFrequencySeconds			() const
							{
								return double( m_uFreq )/1000000.0;
							}
							// Set the frequency in hertz
		inline void			SetFrequencyHertz			( double hz )
							{
								m_uFreq = uint64( 1000000.0/hz );
							}
							// Retrieve the frequency in hertz
		inline double		GetFrequencyHertz			() const
							{
								return 1000000.0/double( m_uFreq );
							}
							// Set the frequency in microseconds
		inline void			SetFrequency				( uint64 microseconds )
							{
								m_uFreq = microseconds > 0 ? microseconds : 1;
							}
							// Retrieve the frequency in microseconds
		inline uint64		GetFrequency				() const
							{
								return m_uFreq;
							}

							// Retrieve the total elapsed time in seconds since construction or the
							// last reset().
		inline double		GetElapsedSeconds			() const
							{
								return double( GetElapsed() )/1000000.0;
							}
							// Retrieve the total elapsed time in microseconds
		inline uint64		GetElapsed					() const
							{
								return Microseconds() - m_uBase;
							}
							// Retrieve the total number of triggers since the last reset
		inline uint64		GetCount					() const
							{
								return GetElapsed()/m_uFreq;
							}

							// Retrieve the current progress
		inline double		GetProgress					() const
							{
								return double( GetElapsed() )/double( m_uFreq );
							}

	protected:
		uint64 m_uBase;
		uint64 m_uFreq;
	};

}}

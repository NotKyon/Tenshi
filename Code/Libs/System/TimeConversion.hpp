#pragma once

#include "../Core/Types.hpp"
#include "../Platform/Platform.hpp"

#ifndef AX_TARGET_FPS
# define AX_TARGET_FPS 60
#endif

namespace Ax { namespace System {

	AX_CONSTEXPR_INLINE uint64 SecondsToMicroseconds( double seconds )
	{
		return uint64( seconds*1000000.0 );
	}
	AX_CONSTEXPR_INLINE uint32 SecondsToMilliseconds( double seconds )
	{
		return uint32( seconds*1000.0 );
	}

	AX_CONSTEXPR_INLINE double MicrosecondsToSeconds( uint64 microseconds )
	{
		return double( microseconds )/1000000.0;
	}
	AX_CONSTEXPR_INLINE double MillisecondsToSeconds( uint32 milliseconds )
	{
		return double( milliseconds )/1000.0;
	}

	AX_CONSTEXPR_INLINE uint64 MillisecondsToMicroseconds( uint32 milliseconds )
	{
		return uint64( milliseconds )*1000;
	}
	AX_CONSTEXPR_INLINE uint32 MicrosecondsToMilliseconds( uint64 microseconds )
	{
		return uint32( microseconds/1000 );
	}

	AX_CONSTEXPR_INLINE double SecondsPerFrame( double targetFPS = AX_TARGET_FPS )
	{
		return 1.0/targetFPS;
	}
	AX_CONSTEXPR_INLINE double SecondsToFrames( double seconds, double targetFPS = AX_TARGET_FPS )
	{
		return seconds/SecondsPerFrame( targetFPS );
	}
	AX_CONSTEXPR_INLINE double MillisecondsToFrames( uint32 milliseconds, double targetFPS = AX_TARGET_FPS )
	{
		return SecondsToFrames( MillisecondsToSeconds( milliseconds ), targetFPS );
	}
	AX_CONSTEXPR_INLINE double MicrosecondsToFrames( uint64 microseconds, double targetFPS = AX_TARGET_FPS )
	{
		return SecondsToFrames( MicrosecondsToSeconds( microseconds ), targetFPS );
	}

	AX_CONSTEXPR_INLINE uint64 MicrosecondsToNanoseconds( uint64 microseconds )
	{
		return microseconds*1000;
	}
	AX_CONSTEXPR_INLINE uint64 MillisecondsToNanoseconds( uint32 milliseconds )
	{
		return uint64( milliseconds )*1000000;
	}
	AX_CONSTEXPR_INLINE uint64 SecondsToNanoseconds( double seconds )
	{
		return uint64( seconds*1000000000.0 );
	}
	AX_CONSTEXPR_INLINE uint64 NanosecondsToMicroseconds( uint64 nanoseconds )
	{
		return nanoseconds/1000;
	}
	AX_CONSTEXPR_INLINE uint32 NanosecondsToMilliseconds( uint64 nanoseconds )
	{
		return uint32( nanoseconds/1000000 );
	}
	AX_CONSTEXPR_INLINE double NanosecondsToSeconds( uint64 nanoseconds )
	{
		return double( nanoseconds )/1000000000.0;
	}

	AX_CONSTEXPR_INLINE uint64 MicrosecondsToHundredNanoseconds( uint64 microseconds )
	{
		return microseconds*10;
	}
	AX_CONSTEXPR_INLINE uint64 HundredNanosecondsToMicroseconds( uint64 hundrednanoseconds )
	{
		return hundrednanoseconds/10;
	}

}}

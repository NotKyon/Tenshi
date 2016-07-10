#ifdef _WIN32
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
#endif
#include "HighPerformanceClock.hpp"
#include "../Core/Assert.hpp"

namespace
{

	using namespace Ax;
	
	inline uint64 ConvertResolution( uint64 t, uint64 f, uint64 r )
	{
		return t*r/f;
	}
#ifdef _WIN32
	inline uint64 QueryFrequency()
	{
		LARGE_INTEGER f;

		if( !QueryPerformanceFrequency( &f ) ) {
			f.QuadPart = 1;
		}

		return ( uint64 )f.QuadPart;
	}
#endif

}

double Ax::System::Seconds()
{
#ifdef _WIN32
	static const uint64 uBaseMicrosecs = Microseconds();
	const uint64 uCurrMicrosecs = Microseconds();

	return double( uCurrMicrosecs - uBaseMicrosecs )/1000000.0;
#else
# error TODO: Ax::System::Seconds() for this platform
#endif
}
Ax::uint64 Ax::System::Microseconds()
{
#ifdef _WIN32
	static const uint64 f = QueryFrequency();
	uint64 t;

	const BOOL r = QueryPerformanceCounter( ( LARGE_INTEGER * )&t );
	( void )r;

	AX_VERIFY( r );

	return ConvertResolution( t, f, 1000000 );
#else
# error TODO: Ax::System::Microseconds() for this platform
#endif
}
void Ax::System::Nanoseconds( int &seconds, int &nanoseconds )
{
#ifdef _WIN32
	static const uint64 f = QueryFrequency();
	uint64 t;

	QueryPerformanceCounter( ( LARGE_INTEGER * )&t );

	seconds = ( int )ConvertResolution( t, f, 1 );
	nanoseconds = ( int )ConvertResolution( t%f, f, 1000000000 );
#else
# error TODO: Ax::System::Nanoseconds() for this platform
#endif
}


Ax::uint32 Ax::System::Milliseconds_LowLatency()
{
#ifdef _WIN32
	static const Ax::uint32 base = GetTickCount();

	AX_STATIC_SUPPRESS( 28159 )
	return GetTickCount() - base;
#else
# error TODO: Ax::System::Milliseconds_LowLatency() for this platform
#endif
}

#ifdef _WIN32
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
# undef Yield
# include <Rpc.h> //UuidCreate() -- link to Rpcrt4.lib
# define USE_WINDOWS_UUID 1
#endif
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
# include <uuid/uuid.h> //uuid_generate_random() -- link to libuuid.a
# define USE_LINUX_UUID 1
#endif
#include <stdio.h>
#include "UUID.hpp"

Ax::String Ax::System::SUUID::ToString() const
{
	return
		String::Formatted
		(
			"{%.8X-%.4X-%.4X-%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X}",
			data1,
			data2,
			data3,
			data4[ 0 ],
			data4[ 1 ],
			data4[ 2 ],
			data4[ 3 ],
			data4[ 4 ],
			data4[ 5 ],
			data4[ 6 ],
			data4[ 7 ]
		);
}

void Ax::System::GenerateUuid( SUUID &uuid )
{
#if USE_WINDOWS_UUID
	( void )UuidCreate( ( ::UUID * )&uuid );
#elif USE_LINUX_UUID
	uuid_generate_random( ( uuid_t )&uuid ); // TODO: VERIFY!!!

	static bool bAlreadyPrinted = false;
	if( bAlreadyPrinted ) {
		return;
	}

	bAlreadyPrinted = true;

	fprintf( stderr, "%s(%i): WARNING: THIS UUID FUNCTION IS UNTESTED!\n", __FILE__, __LINE__ );
	fflush( stderr );
#else
	memset( &uuid, 0, sizeof( uuid ) );

	static bool bAlreadyPrinted = false;
	if( bAlreadyPrinted ) {
		return;
	}

	bAlreadyPrinted = true;

	fprintf( stderr, "%s(%i): ERROR: UUID UNAVAILABLE ON THIS PLATFORM!\n", __FILE__, __LINE__ );
	fflush( stderr );
#endif
}

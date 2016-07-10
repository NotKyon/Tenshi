#ifdef _WIN32
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif
#include <stdio.h>
#include "String.hpp"
#include "../Platform/BuildConf.hpp"
#include "Assert.hpp"

namespace Ax { namespace Detail {

enum class ErrorType
{
	Assert,
	Verify
};

static bool IsDevelopmentBuild()
{
#if AX_DEBUG_ENABLED
	return true;
#else
	//
	//	TODO: Check elsewhere
	//
	return false;
#endif
}

static void HandleError( const char *file, int line, const char *func, const char *expr, const char *msg, ErrorType err )
{
	const char *const title1 = err == ErrorType::Assert ? "assert error" : err == ErrorType::Verify ? "verify error" : "unknown error";
	const char *const title2 = err == ErrorType::Assert ? "Assert Error" : err == ErrorType::Verify ? "Verify Error" : "Unknown Error";
	const char *const title3 = err == ErrorType::Assert ? "Assert Error - Debug?" : err == ErrorType::Verify ? "Verify Error - Debug?" : "Unknown Error - Debug?";
	( void )title1;
	( void )title2;
	( void )title3;

#ifdef _WIN32
	char buf[ 4096 ];
	Format( buf, "%s(%i): %s: in %s: %s (%s)", file, line, title1, func, msg, expr );
	fprintf( stderr, "%s\n", buf );
	CopyTextToClipboard( buf );
#else
	fprintf( stderr, "%s(%i): %s: in %s: %s (%s)\n", file, line, title1, func, msg, expr );
#endif
	fflush( stderr );

#ifdef _WIN32
	Format( buf, "File: %s\nLine: %i\nFunction: %s\nExpression: %s\n\n%s", file, line, func, expr, msg );

	if( IsDebuggerPresent() || IsDevelopmentBuild() ) {
		AppendString( buf, NULL, "\n\nWould you like to debug?" );
		if( MessageBoxA( GetActiveWindow(), buf, title3, MB_ICONERROR | MB_YESNO ) == IDYES ) {
			DebugBreak();
		}
	} else {
# pragma warning( push )
# pragma warning( suppress: 6054 )
		( void )MessageBoxA( GetActiveWindow(), buf, title2, MB_ICONERROR | MB_OK );
# pragma warning( pop )
	}
#elif ( defined( __GNUC__ ) || defined( __clang__ ) )
	if( IsDevelopmentBuild() ) {
		__builtin_trap();
	}
#endif
}

}}

AX_NORETURN
void Ax::Detail::HandleAssert( const char *file, int line, const char *func, const char *expr, const char *msg )
{
	HandleError( file, line, func, expr, msg, ErrorType::Assert );
	exit( EXIT_FAILURE );
}
void Ax::Detail::HandleVerify( const char *file, int line, const char *func, const char *expr, const char *msg )
{
	HandleError( file, line, func, expr, msg, ErrorType::Verify );
}
AX_NORETURN
void Ax::Detail::HandleExpect( const char *file, int line, const char *func, const char *expr, const char *msg )
{
	HandleError( file, line, func, expr, msg, ErrorType::Verify );
	exit( EXIT_FAILURE );
}

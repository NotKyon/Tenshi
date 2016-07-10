#pragma once

#include "../Platform/Platform.hpp"

#ifndef AX_ASSERT_ENABLED
# if AX_DEBUG_ENABLED || AX_TEST_ENABLED
#  define AX_ASSERT_ENABLED 1
# else
#  define AX_ASSERT_ENABLED 0
# endif
#endif

namespace Ax { namespace Detail {

	AX_NORETURN void HandleAssert( const char *file, int line, const char *func, const char *expr, const char *msg );
	void HandleVerify( const char *file, int line, const char *func, const char *expr, const char *msg );
	AX_NORETURN void HandleExpect( const char *file, int line, const char *func, const char *expr, const char *msg );

} }

#define AX_ENABLED_ASSERT_MSG__( expr, msg )\
	( !( expr ) && ( Ax::Detail::HandleAssert( __FILE__, __LINE__, AX_PRETTY_FUNCTION, #expr, ( msg ) ), true ) )
#define AX_ENABLED_VERIFY_MSG__( expr, msg )\
	( !( expr ) ? ( Ax::Detail::HandleVerify( __FILE__, __LINE__, AX_PRETTY_FUNCTION, #expr, ( msg ) ), false ) : true )
#define AX_ENABLED_EXPECT_MSG__( expr, msg )\
	( !( expr ) && ( Ax::Detail::HandleExpect( __FILE__, __LINE__, AX_PRETTY_FUNCTION, #expr, ( msg ) ), false ) )

#if AX_STATIC_ANALYSIS
# define AX_DISABLED_ASSERT_MSG__( expr, msg )\
	AX_STATIC_ASSUME( expr )
# define AX_DISABLED_VERIFY_MSG__( expr, msg )\
	( expr )
# define AX_DISABLED_EXPECT_MSG( expr, msg )\
	AX_STATIC_ASSUME( expr )
#else
# define AX_DISABLED_ASSERT_MSG__( expr, msg )\
	( ( void )0 )
# define AX_DISABLED_VERIFY_MSG__( expr, msg )\
	( expr )
#endif

#if AX_ASSERT_ENABLED
# define AX_ASSERT_MSG( expr, msg ) AX_ENABLED_ASSERT_MSG__( expr, msg )
# define AX_VERIFY_MSG( expr, msg ) AX_ENABLED_VERIFY_MSG__( expr, msg )
# define AX_EXPECT_MSG( expr, msg ) AX_ENABLED_EXPECT_MSG__( expr, msg )
#else
# define AX_ASSERT_MSG( expr, msg ) AX_DISABLED_ASSERT_MSG__( expr, msg )
# define AX_VERIFY_MSG( expr, msg ) AX_DISABLED_VERIFY_MSG__( expr, msg )
# if defined( AX_DISABLED_EXPECT_MSG__ )
#  define AX_EXPECT_MSG( expr, msg ) AX_DISABLED_EXPECT_MSG__( expr, msg )
# else
#  define AX_EXPECT_MSG( expr, msg ) AX_ENABLED_EXPECT_MSG__( expr, msg )
# endif
#endif

#define AX_ASSERT( expr ) AX_ASSERT_MSG( expr, "Assertion failed" )
#define AX_VERIFY( expr ) AX_VERIFY_MSG( expr, "Verification failed" )
#define AX_EXPECT( expr ) AX_EXPECT_MSG( expr, "Expectation failed" )

#define AX_ASSERT_NOT_NULL( var ) AX_ASSERT_MSG( ( var ) != NULL, #var " is NULL" )
#define AX_VERIFY_NOT_NULL( var ) AX_VERIFY_MSG( ( var ) != NULL, #var " is NULL" )
#define AX_EXPECT_NOT_NULL( var ) AX_EXPECT_MSG( ( var ) != NULL, #var " is NULL" )

#define AX_ASSERT_IS_NULL( var ) AX_ASSERT_MSG( ( var ) == NULL, #var " is not NULL" )
#define AX_VERIFY_IS_NULL( var ) AX_VERIFY_MSG( ( var ) == NULL, #var " is not NULL" )
#define AX_EXPECT_IS_NULL( var ) AX_EXPECT_MSG( ( var ) == NULL, #var " is not NULL" )

#define AX_VERIFY_MEMORY( var ) AX_VERIFY_MSG( ( var ) != NULL, "Out of memory (non-fatal)" )
#define AX_EXPECT_MEMORY( var ) AX_EXPECT_MSG( ( var ) != NULL, "Out of memory (!!FATAL!!)" )

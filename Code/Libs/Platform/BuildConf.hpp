#pragma once

#define AX_BUILDTYPE_DEVELOPER		1
#define AX_BUILDTYPE_RETAIL			2

#ifndef AX_DEBUG_ENABLED
# if defined( DEBUG ) || defined( _DEBUG ) || defined( __DEBUG__ )
#  define AX_DEBUG_ENABLED 1
# elif defined( debug ) || defined( _debug ) || defined( __debug__ )
#  define AX_DEBUG_ENABLED 1
# elif defined( MK_DEBUG )
#  define AX_DEBUG_ENABLED 1
# elif defined( AX_BUILDTYPE ) && AX_BUILDTYPE == AX_BUILDTYPE_DEVELOPER
#  define AX_DEBUG_ENABLED 1
# else
#  define AX_DEBUG_ENABLED 0
# endif
#endif

#ifndef AX_TEST_ENABLED
# if defined( TEST ) || defined( _TEST ) || defined( __TEST__ )
#  define AX_TEST_ENABLED 1
# elif defined( test ) || defined( _test ) || defined( __test__ )
#  define AX_TEST_ENABLED 1
# elif defined( MK_TEST )
#  define AX_TEST_ENABLED 1
# else
#  define AX_TEST_ENABLED 0
# endif
#endif

#ifndef AX_BUILDTYPE
# if AX_DEBUG_ENABLED || AX_TEST_ENABLED
#  define AX_BUILDTYPE AX_BUILDTYPE_DEVELOPER
# elif defined( AX_DEVELOPER_BUILD ) && AX_DEVELOPER_BUILD == 1
#  define AX_BUILDTYPE AX_BUILDTYPE_DEVELOPER
# else
#  define AX_BUILDTYPE AX_BUILDTYPE_RETAIL
# endif
#endif

#if !defined( AX_DEVELOPER_BUILD ) && AX_BUILDTYPE == AX_BUILDTYPE_DEVELOPER
# define AX_DEVELOPER_BUILD 1
#endif
#if !defined( AX_RETAIL_BUILD ) && AX_BUILDTYPE == AX_BUILDTYPE_RETAIL
# define AX_RETAIL_BUILD 1
#endif

#if defined( AX_DEVELOPER_BUILD ) && defined( AX_RETAIL_BUILD )
# error Platform: AX_DEVELOPER_BUILD and AX_RETAIL_BUILD are both defined but are mutually exclusive
#endif

#ifndef AX_INTRINSICS_ENABLED
# define AX_INTRINSICS_ENABLED 1
#endif

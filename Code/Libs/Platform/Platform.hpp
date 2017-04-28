#pragma once

#include "CxxSupport.hpp"
#include "MiscTricks.hpp"
#include "BuildConf.hpp"
#include "StaticAnalysis.hpp"

// compilers
#define AX_COMPILER_MSC				0x01
#define AX_COMPILER_GCC				0x02
#define AX_COMPILER_CLANG			0x04
#define AX_COMPILER_INTEL			0x08

// architectures
#define AX_ARCH_INTEL				0x01
#define AX_ARCH_POWER				0x02
#define AX_ARCH_MIPS				0x04
#define AX_ARCH_ARM					0x08

#define AX_ARCH_32					0x40
#define AX_ARCH_64					0x80

// intrinsics
#define AX_INTRIN_SSE				0x01
#define AX_INTRIN_NEON				0x02

// platform
#define AX_PLATFORM_WINDOWS			0x01
#define AX_PLATFORM_LINUX			0x02
#define AX_PLATFORM_MAC				0x04

#define AX_PLATFORM_DESKTOP			0x100
#define AX_PLATFORM_MOBILE			0x200
#define AX_PLATFORM_STORE			0x400

#if AX_STORE_ENABLED
# define AX_PLATFORM_STORE_FLAG__	AX_PLATFORM_STORE
#else
# define AX_PLATFORM_STORE_FLAG__	0
#endif

/// @def AX_COMPILER
/// Stores a bit-field of the detected compilers.
///
/// @def AX_COMPILER_MSC
/// Represents the Microsoft Visual C/C++ compiler.
/// @def AX_COMPILER_GCC
/// Represents the GNU C/C++ compiler.
/// @def AX_COMPILER_CLANG
/// Represents the LLVM Clang compiler.
/// @def AX_COMPILER_INTEL
/// Represents the Intel C/C++ compiler.
#ifndef AX_COMPILER
# if defined( _MSC_VER )
#  if defined( __INTEL_COMPILER )
#   define AX_COMPILER				( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  else
#   define AX_COMPILER				( AX_COMPILER_MSC )
#  endif
# elif defined( __GNUC__ )
#  if defined( __INTEL_COMPILER )
#   define AX_COMPILER				( AX_COMPILER_GCC | AX_COMPILER_INTEL )
#  elif defined( __clang__ )
#   define AX_COMPILER				( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  else
#   define AX_COMPILER				( AX_COMPILER_GCC )
#  endif
# elif defined( __INTEL_COMPILER )
#  define AX_COMPILER				( AX_COMPILER_INTEL )
# elif defined( __clang__ )
#  define AX_COMPILER				( AX_COMPILER_CLANG )
# else
#  error Unknown compiler
# endif
#endif

/// @def AX_ARCH
/// Stores a bit-field of the detected architectures. ( There will be only one
/// main architecture, but there will also be a 32-bit or 64-bit flag. )
///
/// @def AX_ARCH_INTEL
/// Represents the ubiquitous Intel/AMD x86/x64 processors.
/// @def AX_ARCH_POWER
/// Represents the IBM POWER brand of processors.
/// @def AX_ARCH_MIPS
/// Represents the MIPS line of processors.
/// @def AX_ARCH_ARM
/// Represents the ARM line of processors.
///
/// @def AX_ARCH_32
/// 32-bit code is active.
/// @def AX_ARCH_64
/// 64-bit code is active.
#ifndef AX_ARCH
# if defined( __amd64__ ) || defined( __x86_64__ ) || defined( _M_AMD64 )
#  define AX_ARCH					( AX_ARCH_INTEL | AX_ARCH_64 )
# elif defined( __x86__ ) || defined( _M_IX86 )
#  define AX_ARCH					( AX_ARCH_INTEL | AX_ARCH_32 )
# else
#  error Unknown architecture
# endif
#endif

/// @def AX_INTRIN
/// Represents the set of intrinsics activated. ( This will be 0 if
/// AX_INTRINSICS_ENABLED is set to 0. )
///
/// @def AX_INTRIN_SSE
/// SSE intrinsics.
/// @def AX_INTRIN_NEON
/// NEON intrinsics.
#ifndef AX_INTRIN
# if AX_INTRINSICS_ENABLED
#  if AX_ARCH & AX_ARCH_INTEL
#   define AX_INTRIN				( AX_INTRIN_SSE )
#  elif AX_ARCH & AX_ARCH_ARM
#   define AX_INTRIN				( AX_INTRIN_NEON )
#  else
#   define AX_INTRIN				0
#  endif
# else
#  define AX_INTRIN					0
# endif
#endif

/// @def AX_PLATFORM
/// A bit-field of flags representing the target platform and its
/// characteristics.
///
/// @def AX_PLATFORM_WINDOWS
/// Microsoft Windows operating systems.
/// @def AX_PLATFORM_LINUX
/// GNU/Linux operating systems.
/// @def AX_PLATFORM_MAC
/// Apple Mac OS X operating systems.
///
/// @def AX_PLATFORM_DESKTOP
/// Targeting the computer desktops.
/// @def AX_PLATFORM_MOBILE
/// Targeting mobile devices ( phones, tablets ).
/// @def AX_PLATFORM_STORE
/// Targeting an app store of some kind.
#ifndef AX_PLATFORM
# if defined( _WIN32 )
#  define AX_PLATFORM				( AX_PLATFORM_WINDOWS | AX_PLATFORM_DESKTOP | AX_PLATFORM_STORE_FLAG__ )
# elif defined( __linux__ )
#  if defined( ANDROID )
#   define AX_PLATFORM				( AX_PLATFORM_LINUX | AX_PLATFORM_MOBILE | AX_PLATFORM_STORE )
#  else
#   define AX_PLATFORM				( AX_PLATFORM_LINUX | AX_PLATFORM_DESKTOP | AX_PLATFORM_STORE_FLAG__ )
#  endif
# elif defined( __APPLE__ )
#  if defined( IPHONE_SIMULATOR ) || defined( IPHONE ) || defined( IOS )
#   define AX_PLATFORM				( AX_PLATFORM_MAC | AX_PLATFORM_MOBILE | AX_PLATFORM_STORE )
#  else
#   define AX_PLATFORM				( AX_PLATFORM_MAC | AX_PLATFORM_DESKTOP | AX_PLATFORM_STORE_FLAG__ )
#  endif
# else
#  error Unknown platform
# endif
#endif

/// @def AX_OS
/// Pass an OS name into this macro to retrieve a 0 or 1 value indicating
/// whether that is the current target OS.
#ifndef AX_OS
  // Windows (Desktop)
# if ( AX_PLATFORM & AX_PLATFORM_WINDOWS ) && !( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_Windows__			1
# else
#  define AX_OS_Windows__			0
# endif
  // Windows (Metro/Store/RT)
# if ( AX_PLATFORM & AX_PLATFORM_WINDOWS ) && ( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_WinRT__				1
# else
#  define AX_OS_WinRT__				0
# endif
  // Linux
# if ( AX_PLATFORM & AX_PLATFORM_LINUX ) && !( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_Linux__				1
# else
#  define AX_OS_Linux__				0
# endif
  // Android
# if ( AX_PLATFORM & AX_PLATFORM_LINUX ) && ( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_Android__			1
# else
#  define AX_OS_Android__			0
# endif
  // Mac OS X
# if ( AX_PLATFORM & AX_PLATFORM_MAC ) && !( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_OSX__				1
# else
#  define AX_OS_OSX__				0
# endif
  // iOS
# if ( AX_PLATFORM & AX_PLATFORM_MAC ) && ( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_OS_iOS__				1
# else
#  define AX_OS_iOS__				0
# endif

# define AX_OS(TargetOS__)			AX_OS_##TargetOS__##__
#endif

/// @def AX_FUNCTION
/// Current function identifier.
#ifndef AX_FUNCTION
# if AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_FUNCTION				__func__
# elif AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_FUNCTION				__FUNCTION__
# else
#  define AX_FUNCTION				( ( const char * )0 )
# endif
#endif

/// @def AX_PRETTY_FUNCTION
/// Current function identifier (with extra information)
#ifndef AX_PRETTY_FUNCTION
# if AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_PRETTY_FUNCTION		__FUNCTION__
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_PRETTY_FUNCTION		__PRETTY_FUNCTION__
# elif __STDC_VERSION__ >= 19901
#  define AX_PRETTY_FUNCTION		__func__
# else
#  define AX_PRETTY_FUNCTION		( ( const char * )0 )
# endif
#endif

/// @def AX_PURE
/// Pure-functional attribute.
///
/// Functions marked as pure-functional do not depend on varying state except
/// for their input parameters, and do not alter state. i.e., their output is
/// solely dependent on their input, and nothing else.
#ifndef AX_PURE
# if AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_PURE					__attribute__( ( const ) )
# else
#  define AX_PURE
# endif
#endif

/// @def AX_WEAK
/// Weak-linking attribute.
///
/// Symbols marked as weak-linked can appear multiple times across several
/// object files. Only one symbol will be selected. This helps reduce memory and
/// storage consumption.
///
/// @def AX_HAS_WEAK
/// Set to 1 if AX_WEAK is available, or 0 if not.
#ifndef AX_WEAK
# if AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_WEAK					__declspec( selectany )
#  define AX_HAS_WEAK				1
/*
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_WEAK					__attribute__( ( weak ) )
#  define AX_HAS_WEAK				1
*/
# else
#  define AX_WEAK
#  define AX_HAS_WEAK				0
# endif
#endif

/// @def AX_GLOBAL
/// Preferred qualifiers and attributes for weak globals
#ifndef AX_GLOBAL
# if AX_HAS_WEAK
#  define AX_GLOBAL					AX_WEAK static extern
# else
#  define AX_GLOBAL					static
# endif
#endif

/// @def AX_GLOBAL_CONST
/// Preferred qualifiers and attributes for global constants.
#ifndef AX_GLOBAL_CONST
# if AX_HAS_WEAK
#  define AX_GLOBAL_CONST AX_WEAK	extern const
# else
#  define AX_GLOBAL_CONST			static const
# endif
#endif

/// @def AX_NORETURN
/// Function attribute stating that it will not return to the calling code.
#ifndef AX_NORETURN
# if AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_NORETURN				__declspec( noreturn )
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_NORETURN				__attribute__( ( noreturn ) )
# else
#  define AX_NORETURN
# endif
#endif

/// @def AX_CACHE_SIZE
/// Default cache-size.
#ifndef AX_CACHE_SIZE
# if ( AX_ARCH & ( AX_ARCH_INTEL | AX_ARCH_64 ) ) == ( AX_ARCH_INTEL | AX_ARCH_64 )
#  define AX_CACHE_SIZE				128
# else
#  define AX_CACHE_SIZE				64
# endif
#endif
#define AX_POINTERS_PER_CACHELINE	( AX_CACHE_SIZE/sizeof( void * ) )

/// @def AX_ALIGN
/// Compiler attribute for aligning.
///
/// @def AX_HAS_ALIGN
/// Set to 1 if the compiler provides a method for aligning symbols, or 0 if
/// not.
///
/// @def AX_CACHEALIGN
/// Enables alignment to the default cache-size
#ifndef AX_ALIGN
# if AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_ALIGN( x )				__declspec( align( x ) )
#  define AX_HAS_ALIGN				1
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_ALIGN( x )				__attribute__( ( align( x ) ) )
#  define AX_HAS_ALIGN				1
# else
#  define AX_ALIGN( x )
#  define AX_HAS_ALIGN				0
# endif
# define AX_CACHEALIGN				AX_ALIGN( AX_CACHE_SIZE )
#endif

/// @def AX_HAS_VECTORCALL
/// Set to 1 if the vector-call calling convention is supported by the compiler,
/// or 0 if not.
#ifndef AX_HAS_VECTORCALL
# if ( ( AX_COMPILER & AX_COMPILER_MSC ) && ( defined( _M_IX86 ) ||\
defined( _M_X64 ) ) && ( !defined( _M_IX86_FP ) || _M_IX86_FP > 1 ) && \
( ( _MSC_FULL_VER >= 170065501 && _MSC_VER < 1800 ) || \
_MSC_FULL_VER >= 180020418 ) ) && defined( _WIN32 )
#  define AX_HAS_VECTORCALL			1
# else
#  define AX_HAS_VECTORCALL			0
# endif
#endif

/// @def AX_VECTORCALL
/// Preferred calling convention for functions taking SIMD vector parameters.
#ifndef AX_VECTORCALL
# if AX_HAS_VECTORCALL
#  define AX_VECTORCALL				__vectorcall
# elif defined(_WIN32)
#  define AX_VECTORCALL				__fastcall
# else
#  define AX_VECTORCALL             /* none */
# endif
#endif
/// @def AX_VCALL
/// Same as AX_VECTORCALL, but shorter for convenience.
#ifndef AX_VCALL
# define AX_VCALL					AX_VECTORCALL
#endif

/// @def AX_CONCAT( x,y )
/// Concatenates two symbol names ( \a x and \a y ) into one.
///
/// @def AX_ANONVAR( x )
/// Creates an anonymous variable with base name \a x.
#define AX_CONCAT_IMPL( x, y )		x##y
#define AX_CONCAT( x, y )			AX_CONCAT_IMPL( x, y )

#ifdef __COUNTER__
# define AX_ANONVAR( x )			AX_CONCAT( x, __COUNTER__ )
#else
# define AX_ANONVAR( x )			AX_CONCAT( x, __LINE__ )
#endif

/// @def AX_IF_LIKELY
/// An if-statement marked as being likely to succeed.
///
/// @warning Use this sparingly and only when you are certain that a particular
///          if-statement will succeed often. Failure to do so can have adverse
///          effects on performance.
///
/// @def AX_IF_UNLIKELY
/// An if-statement marked as being unlikely to succeed.
///
/// @warning Use this sparingly and only when you are certain that a particular
///          if-statement will not succeed often. Failure to do so can have
///          adverse effects on performance.
#if AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
	# define AX_IF_LIKELY( expr )	if( __builtin_expect( !!( expr ), 1 ) )
# define AX_IF_UNLIKELY( expr )		if( __builtin_expect( !!( expr ), 0 ) )
#else
# define AX_IF_LIKELY( expr )		if( expr )
# define AX_IF_UNLIKELY( expr )		if( expr )
#endif

#ifndef AX_PRINTF_PARM
# if AX_MSC_2010
#  include <CodeAnalysis/sourceannotations.h>
#  define AX_PRINTF_PARM			[SA_FormatString(Style="printf")]
# else
#  define AX_PRINTF_PARM
# endif
#endif

#ifndef AX_PRINTF_ATTR
# if AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_PRINTF_ATTR( x, y )	__attribute__( ( format( printf, x, y ) ) )
# else
#  define AX_PRINTF_ATTR( x, y )
# endif
#endif

#ifndef AX_FORCEINLINE
# if AX_COMPILER & ( AX_COMPILER_MSC | AX_COMPILER_INTEL )
#  define AX_FORCEINLINE			__forceinline
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_FORCEINLINE			inline __attribute__((always_inline))
# else
#  define AX_FORCEINLINE			inline
# endif
#endif

#ifndef AX_THREADLOCAL
# ifdef _WIN32
#  define AX_THREADLOCAL			__declspec( thread )
#  define AX_HAS_THREADLOCAL		1
# elif AX_COMPILER & ( AX_COMPILER_GCC | AX_COMPILER_CLANG )
#  define AX_THREADLOCAL			__attribute__((thread_local))
#  define AX_HAS_THREADLOCAL		1
# else
#  define AX_THREADLOCAL
#  define AX_HAS_THREADLOCAL		0
# endif
#endif

#ifndef AX_OVERRIDE
# if AX_CXX_N3272
#  define AX_OVERRIDE				override
# else
#  define AX_OVERRIDE
# endif
#endif
#ifndef AX_FINAL
# if AX_CXX_N3272
#  define AX_FINAL					final
# else
#  define AX_FINAL
# endif
#endif

/*
===============================================================================

	C++11 SUPPORT

===============================================================================
*/

//
//	alignas [N2341]
//
#ifndef AX_CXX_ALIGNAS_ENABLED
# define AX_CXX_ALIGNAS_ENABLED AX_CXX_N2341
#endif

//
//	alignof [N2341]
//
#ifndef AX_CXX_ALIGNOF_ENABLED
# define AX_CXX_ALIGNOF_ENABLED AX_CXX_N2341
#endif

//
//	C++11 Atomics [N2427]
//
#ifndef AX_CXX_ATOMICS_ENABLED
# define AX_CXX_ATOMICS_ENABLED AX_CXX_N2427
#endif

//
//	auto [N1984,N2546]
//
#ifndef AX_CXX_AUTO_ENABLED
# define AX_CXX_AUTO_ENABLED AX_CXX_N1984
#endif

//
//	C99 preprocessor [N1653]
//
#ifndef AX_C99_PREPROCESSOR_ENABLED
# define AX_C99_PREPROCESSOR_ENABLED AX_CXX_N1653
#endif

//
//	constexpr [N2235]
//
#ifndef AX_CXX_CONSTEXPR_ENABLED
# define AX_CXX_CONSTEXPR_ENABLED AX_CXX_N2235
#endif

//
//	decltype [N2343,N3276]
//
#ifndef AX_CXX_DECLTYPE_ENABLED
# define AX_CXX_DECLTYPE_ENABLED AX_CXX_N2343
#endif

//
//	Defaulted and Deleted Functions [N2346]
//
#ifndef AX_CXX_DEFAULT_DELETE_FUNCTIONS_ENABLED
# define AX_CXX_DEFAULT_DELETE_FUNCTIONS_ENABLED AX_CXX_N2346
#endif

//
//	Delegating Constructors [N1986]
//
#ifndef AX_CXX_DELEGATING_CTORS_ENABLED
# define AX_CXX_DELEGATING_CTORS_ENABLED AX_CXX_N1986
#endif

//
//	Explicit Conversion Operators [N2437]
//
#ifndef AX_CXX_EXPLICIT_CONVERSION_OPERATORS_ENABLED
# define AX_CXX_EXPLICIT_CONVERSION_OPERATORS_ENABLED AX_CXX_N2437
#endif

//
//	Extended 'friend' Declarations [N1791]
//
#ifndef AX_CXX_EXTENDED_FRIEND_DECLARATIONS_ENABLED
# define AX_CXX_EXTENDED_FRIEND_DECLARATIONS_ENABLED AX_CXX_N1791
#endif

//
//	extern template [N1987]
//
#ifndef AX_CXX_EXTERN_TEMPLATE_ENABLED
# define AX_CXX_EXTERN_TEMPLATE_ENABLED AX_CXX_N1987
#endif

//
//	Forward Declarations For Enums [N2764]
//
#ifndef AX_CXX_FORWARD_ENUMS_ENABLED
# define AX_CXX_FORWARD_ENUMS_ENABLED AX_CXX_N2764
#endif

//
//	Inheriting Constructors [N2540]
//
#ifndef AX_CXX_INHERITING_CONSTRUCTORS_ENABLED
# define AX_CXX_INHERITING_CONSTRUCTORS_ENABLED AX_CXX_N2540
#endif

//
//	Initializer Lists [N2672]
//
#ifndef AX_CXX_INIT_LISTS_ENABLED
# define AX_CXX_INIT_LISTS_ENABLED AX_CXX_N2672
#endif

//
//	Lambda Expressions and Closures [N2550,N2658,N2927]
//
#ifndef AX_CXX_LAMBDA_ENABLED
# define AX_CXX_LAMBDA_ENABLED AX_CXX_N2550
#endif

//
//	Local/Anonymous Types as Template Arguments [N2657]
//
#ifndef AX_CXX_LOCAL_TYPE_TEMPLATE_ARGS_ENABLED
# define AX_CXX_LOCAL_TYPE_TEMPLATE_ARGS_ENABLED AX_CXX_N2657
#endif

//
//	Long Long [N1811]
//
#ifndef AX_CXX_LONG_LONG_ENABLED
# define AX_CXX_LONG_LONG_ENABLED AX_CXX_N1811
#endif

//
//	New Function Declaration Syntax For Deduced Return Types [N2541]
//
#ifndef AX_CXX_AUTO_FUNCTIONS_ENABLED
# define AX_CXX_AUTO_FUNCTIONS_ENABLED AX_CXX_N2541
#endif

//
//	nullptr [N2431]
//
#ifndef AX_CXX_NULLPTR_ENABLED
# define AX_CXX_NULLPTR_ENABLED AX_CXX_N2431
#endif

//
//	R-Value References / std::move [N2118,N2844,N2844+,N3053]
//
#ifndef AX_CXX_RVALUE_REFS_ENABLED
# define AX_CXX_RVALUE_REFS_ENABLED AX_CXX_N2118
#endif

//
//	static_assert [N1720]
//
#ifndef AX_CXX_STATIC_ASSERT_ENABLED
# define AX_CXX_STATIC_ASSERT_ENABLED AX_CXX_N1720
#endif

//
//	Strongly-typed Enums [N2347]
//
#ifndef AX_CXX_STRONG_ENUMS_ENABLED
# define AX_CXX_STRONG_ENUMS_ENABLED AX_CXX_N2347
#endif

//
//	Template Aliases [N2258]
//
#ifndef AX_CXX_TEMPLATE_ALIASES_ENABLED
# define AX_CXX_TEMPLATE_ALIASES_ENABLED ( AX_CXX_N2258 )
#endif

//
//	Thread Local Storage [N2659]
//
#ifndef AX_CXX_TLS_ENABLED
# define AX_CXX_TLS_ENABLED AX_CXX_N2659
#endif

//
//	Built-in Type Traits [N1836]
//
#ifndef AX_CXX_TYPE_TRAITS_ENABLED
# define AX_CXX_TYPE_TRAITS_ENABLED AX_CXX_N1836
#endif

//
//	Variadic Templates [N2242,N2555]
//
#ifndef AX_CXX_VARIADIC_TEMPLATES_ENABLED
//# define AX_CXX_VARIADIC_TEMPLATES_ENABLED AX_CXX_N2242
# define AX_CXX_VARIADIC_TEMPLATES_ENABLED 0
#endif

//
//	Range-based For-loops [N2930]
//
#ifndef AX_CXX_RANGE_FOR_ENABLED
# define AX_CXX_RANGE_FOR_ENABLED AX_CXX_N2930
#endif

//
//	override and final [N2928,N3206,N3272]
//
#ifndef AX_CXX_OVERRIDE_FINAL_ENABLED
# define AX_CXX_OVERRIDE_FINAL_ENABLED AX_CXX_N2928
#endif

//
//	Attributes [N2761]
//
#ifndef AX_CXX_ATTRIBUTES_ENABLED
# define AX_CXX_ATTRIBUTES_ENABLED AX_CXX_N2761
#endif

//
//	Ref-Qualifiers [N2439]
//
#ifndef AX_CXX_REF_QUALIFIERS_ENABLED
# define AX_CXX_REF_QUALIFIERS_ENABLED AX_CXX_N2439
#endif

//
//	Non-static Data Member Initializers [N2756]
//
#ifndef AX_CXX_NONSTATIC_INIT_ENABLED
# define AX_CXX_NONSTATIC_INIT_ENABLED AX_CXX_N2756
#endif

/*
===============================================================================

	C++11 HELPERS

===============================================================================
*/

#ifndef AX_DELETE_FUNC
# if AX_CXX_N2346
#  define AX_DELETE_FUNC			= delete
# else
#  define AX_DELETE_FUNC
# endif
#endif

#ifndef AX_DELETE_COPYCTOR
# if AX_CXX_N2346
#  define AX_DELETE_COPYCTOR(N_)	N_( const N_ & ) = delete
# else
#  define AX_DELETE_COPYCTOR(N_)
# endif
#endif
#ifndef AX_DELETE_ASSIGNOP
# if AX_CXX_N2346
#  define AX_DELETE_ASSIGNOP(N_)	N_ &operator=( const N_ & ) = delete
# else
#  define AX_DELETE_ASSIGNOP(N_)
# endif
#endif
#ifndef AX_DELETE_COPYFUNCS
# define AX_DELETE_COPYFUNCS(T_)\
									AX_DELETE_COPYCTOR(T_);\
									AX_DELETE_ASSIGNOP(T_)
#endif

#ifndef AX_NOTHROW
# define AX_NOTHROW					throw()
#endif

#ifndef AX_CONSTEXPR
# if AX_CXX_CONSTEXPR_ENABLED
#  define AX_CONSTEXPR				constexpr
# else
#  define AX_CONSTEXPR
# endif
#endif

#ifndef AX_CONSTEXPR_INLINE
# if AX_CXX_CONSTEXPR_ENABLED
#  define AX_CONSTEXPR_INLINE		AX_CONSTEXPR
# else
#  define AX_CONSTEXPR_INLINE		inline
# endif
#endif

#ifndef AX_STATIC_CONST
# if AX_CXX_CONSTEXPR_ENABLED
#  define AX_STATIC_CONST			constexpr
# else
#  define AX_STATIC_CONST			static const
# endif
#endif

#ifndef AX_HAS_STATIC_ASSERT
# ifdef __cplusplus
#  if defined( _MSC_VER ) && _MSC_VER >= 1600
#   define AX_HAS_STATIC_ASSERT		1
#  elif defined( __GNUC__ ) && ( __GNUC__*100 + __GNUC_MINOR__*10 ) >= 410
#   define AX_HAS_STATIC_ASSERT		1
#  elif defined( __clang__ )
#   define AX_HAS_STATIC_ASSERT		1
#  else
#   define AX_HAS_STATIC_ASSERT		0
#  endif
# else
#  define AX_HAS_STATIC_ASSERT		0
# endif
#endif

#ifndef AX_STATIC_ASSERT_ENABLED
# if AX_HAS_STATIC_ASSERT
#  define AX_STATIC_ASSERT_ENABLED	1
# else
#  define AX_STATIC_ASSERT_ENABLED	0
# endif
#endif

#if AX_STATIC_ASSERT_ENABLED
# if AX_HAS_STATIC_ASSERT
#  define ax_static_assert( expr, msg ) static_assert( expr, msg )
# else
#  define ax_static_assert( expr, msg )\
	enum { AX_CONCAT( AX_StaticAssert_, __LINE__ ) = 1/( +!!( expr ) ) }
# endif
#else
# define ax_static_assert( expr, msg )
#endif

#ifndef AX_DLLIMPORT
# ifdef _WIN32
#  define AX_DLLIMPORT				__declspec(dllimport)
# else
#  define AX_DLLIMPORT
# endif
#endif

#ifndef AX_DLLEXPORT
# ifdef _WIN32
#  define AX_DLLEXPORT				__declspec(dllexport)
# else
#  define AX_DLLEXPORT
# endif
#endif

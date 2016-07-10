#pragma once

#if defined( __clang__ )
# define AX_CLANG_VER ( __clang_major__*100 + __clang_minor__*10 + __clang_patchlevel__ )
#else
# define AX_CLANG_VER 0
#endif

#if defined( __GNUC__ )
# define AX_GCC_VER ( __GNUC__*100 + __GNUC_MINOR__*10 + __GNUC_PATCHLEVEL__ )
#else
# define AX_GCC_VER 0
#endif

#if defined( __INTEL_COMPILER )
# define AX_ICC_VER ( __INTEL_COMPILER/10 )
#else
# define AX_ICC_VER 0
#endif

#if defined( _MSC_VER )
# define AX_MSC_VER ( _MSC_VER/10 )
#else
# define AX_MSC_VER 0
#endif

#define AX_CLANG_2_9 ( AX_CLANG_VER >= 290 )
#define AX_CLANG_3_0 ( AX_CLANG_VER >= 300 )
#define AX_CLANG_3_1 ( AX_CLANG_VER >= 310 )
#define AX_CLANG_3_2 ( AX_CLANG_VER >= 320 )
#define AX_CLANG_3_3 ( AX_CLANG_VER >= 330 )
#define AX_CLANG_3_4 ( AX_CLANG_VER >= 340 )

#define AX_GCC_4_0 ( AX_GCC_VER >= 400 )
#define AX_GCC_4_1 ( AX_GCC_VER >= 410 )
#define AX_GCC_4_2 ( AX_GCC_VER >= 420 )
#define AX_GCC_4_3 ( AX_GCC_VER >= 430 )
#define AX_GCC_4_4 ( AX_GCC_VER >= 440 )
#define AX_GCC_4_5 ( AX_GCC_VER >= 450 )
#define AX_GCC_4_6 ( AX_GCC_VER >= 460 )
#define AX_GCC_4_7 ( AX_GCC_VER >= 470 )
#define AX_GCC_4_8 ( AX_GCC_VER >= 480 )
#define AX_GCC_4_8_1 ( AX_GCC_VER >= 481 )

#define AX_ICC_11_0 ( AX_ICC_VER >= 110 )
#define AX_ICC_11_1 ( AX_ICC_VER >= 111 )
#define AX_ICC_12_0 ( AX_ICC_VER >= 120 )
#define AX_ICC_12_1 ( AX_ICC_VER >= 121 )
#define AX_ICC_13_0 ( AX_ICC_VER >= 130 )
#define AX_ICC_14_0 ( AX_ICC_VER >= 140 )

#define AX_MSC_2010 ( AX_MSC_VER >= 160 )
#define AX_MSC_2012 ( AX_MSC_VER >= 170 )
#define AX_MSC_2013 ( AX_MSC_VER >= 180 )
#define AX_MSC_2013_CTP ( AX_MSC_VER >= 181 ) // TODO: How to calculate?

/*
===============================================================================

	C++11 SUPPORT

===============================================================================
*/

//
//	GENERAL
//

// Initialization of class objects by r-values
#define AX_CXX_N1610 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 || AX_MSC_2010 )
// static_assert
#define AX_CXX_N1720 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 || AX_MSC_2010 )
// Multi-declarator auto
#define AX_CXX_N1737 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_0 )
// Right angle brackets
#define AX_CXX_N1757 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 || AX_MSC_2010 )
// Extended friend declarations
#define AX_CXX_N1791 ( AX_CLANG_2_9 || AX_GCC_4_7 || AX_ICC_12_0 || AX_MSC_2010 )
// Built-in type traits
#define AX_CXX_N1836 ( AX_CLANG_3_0 || AX_GCC_4_3 || AX_ICC_11_0 || AX_MSC_2010 )
// auto: Deducing the type of variable from its initializer expression
#define AX_CXX_N1984 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_0 || AX_MSC_2010 )
// Delegating constructors
#define AX_CXX_N1986 ( AX_CLANG_3_0 || AX_GCC_4_7 || AX_ICC_14_0 || AX_MSC_2010 )
// extern templates
#define AX_CXX_N1987 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 || AX_MSC_2010 )
// R-value references
#define AX_CXX_N2118 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_12_0 || AX_MSC_2010 )
// Universal character names in literals
#define AX_CXX_N2170 ( AX_CLANG_3_1 || AX_GCC_4_5 || AX_ICC_12_1 )
// Generalized constant expressions
#define AX_CXX_N2235 ( AX_CLANG_3_1 || AX_GCC_4_6 || AX_ICC_14_0 )
// Variadic templates v0.9
#define AX_CXX_N2242 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_14_0 || AX_MSC_2013 )
// char16_t/char32_t types
#define AX_CXX_N2249 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_14_0 )
// Extended sizeof
#define AX_CXX_N2253 ( AX_CLANG_3_1 || AX_GCC_4_4 )
// Alias templates
#define AX_CXX_N2258 ( AX_CLANG_3_0 || AX_GCC_4_7 || AX_ICC_12_1 || AX_MSC_2013 )
// Alignment support in language
#define AX_CXX_N2341 ( AX_CLANG_3_3 || AX_GCC_4_8 )
// Standard-layout and trivial types
#define AX_CXX_N2342 ( AX_CLANG_3_0 || AX_GCC_4_5 || AX_MSC_2012 )
// decltype v1.0
#define AX_CXX_N2343 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_12_0 || AX_MSC_2010 )
// Defaulted and deleted functions
#define AX_CXX_N2346 ( AX_CLANG_3_0 || AX_GCC_4_4 || AX_ICC_12_0 || AX_MSC_2013 )
// Explicit enum bases and scoped enums
#define AX_CXX_N2347_PARTIAL ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_13_0 || AX_MSC_2012 )
// Strongly typed enums
#define AX_CXX_N2347 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_14_0 || AX_MSC_2012 )
// nullptr
#define AX_CXX_N2431 ( AX_CLANG_3_0 || AX_GCC_4_6 || AX_ICC_12_1 || AX_MSC_2010 )
// Explicit conversion operators
#define AX_CXX_N2437 ( AX_CLANG_3_0 || AX_GCC_4_5 || AX_ICC_13_0 || AX_MSC_2013 )
// R-value references for *this
#define AX_CXX_N2439 ( AX_CLANG_2_9 || AX_GCC_4_8_1 || AX_ICC_14_0 )
// Raw and unicode string literals
#define AX_CXX_N2442 ( AX_CLANG_3_0 || AX_GCC_4_5 || AX_ICC_14_0 || AX_MSC_2013 )
// Inline namespaces
#define AX_CXX_N2535 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_14_0 )
// Inheriting constructors
#define AX_CXX_N2540 ( AX_CLANG_3_3 || AX_GCC_4_8 )
// auto: Trailing return types; late-specified return types
#define AX_CXX_N2541 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_1 || AX_MSC_2010 )
// Unrestricted unions
#define AX_CXX_N2544 ( AX_CLANG_3_1 || AX_GCC_4_6 )
// auto v1.0: Removal of auto as storage-class specifier
#define AX_CXX_N2546 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_0 || AX_MSC_2010 )
// Lambdas v0.9: Lambda expressions and closures
#define AX_CXX_N2550 ( AX_CLANG_3_1 || AX_GCC_4_5 || AX_ICC_12_0 || AX_MSC_2010 )
// Variadic templates v1.0
#define AX_CXX_N2555 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_1 || AX_MSC_2013 )
// New-style expression SFINAE
#define AX_CXX_N2634 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_1 )
// Local and unnamed types as template arguments
#define AX_CXX_N2657 ( AX_CLANG_2_9 || AX_GCC_4_5 || AX_ICC_12_0 || AX_MSC_2010 )
// Lambdas v1.0: Constness of lambda functions
#define AX_CXX_N2658 ( AX_CLANG_3_1 || AX_GCC_4_5 || AX_ICC_12_0 || AX_MSC_2010 )
// Minimal support for garbage collection
#define AX_CXX_N2670 ( AX_MSC_2010 )
// General initializer lists
#define AX_CXX_N2672 ( AX_CLANG_3_1 || AX_GCC_4_4 || AX_ICC_14_0 || AX_MSC_2013 )
// Non-static data member initializers
#define AX_CXX_N2756 ( AX_CLANG_3_0 || AX_GCC_4_7 || AX_ICC_14_0 )
// Standard attributes
#define AX_CXX_N2761 ( AX_CLANG_3_3 || AX_GCC_4_8 || AX_ICC_12_1 )
// Forward declared enums
#define AX_CXX_N2764 ( AX_CLANG_3_1 || AX_GCC_4_6 || AX_ICC_14_0 || AX_MSC_2010 )
// User-defined literals
#define AX_CXX_N2765 ( AX_CLANG_3_1 || AX_GCC_4_7 )
// R-value references v2
#define AX_CXX_N2844 ( AX_CLANG_3_0 || AX_GCC_4_6 || AX_ICC_12_0 || AX_MSC_2010 )
// Lambdas v1.1: New wording for C++0x/C++11 lambdas
#define AX_CXX_N2927 ( AX_CLANG_3_1 || AX_GCC_4_5 || AX_ICC_12_0 || AX_MSC_2012 )
// Explicit virtual overrides
#define AX_CXX_N2928 ( AX_ICC_14_0 )
// Range-based for loops
#define AX_CXX_N2930 ( AX_CLANG_3_0 || AX_GCC_4_6 || AX_ICC_13_0 || AX_MSC_2012 )
// Additional type traits
#define AX_CXX_N2947 ( AX_ICC_13_0 )
// noexcept; allowing move constructors to throw
#define AX_CXX_N3050 ( AX_CLANG_3_0 || AX_GCC_4_6 || AX_ICC_14_0 )
// Conversions of lambdas to function pointers
#define AX_CXX_N3052 ( AX_ICC_13_0 )
// R-values: Defining move special member functions
#define AX_CXX_N3053 ( AX_CLANG_3_0 || AX_GCC_4_6 || AX_ICC_14_0 )
// Explicit virtual overrides: Override control (eliminating attributes)
#define AX_CXX_N3206 ( AX_CLANG_3_0 || AX_GCC_4_7 || AX_MSC_2012 )
// override and final
#define AX_CXX_N3272 ( AX_CLANG_3_0 || AX_GCC_4_7 || AX_ICC_14_0 || AX_MSC_2012 )
// decltype v1.1: Decltype and call expressions
#define AX_CXX_N3276 ( AX_CLANG_3_1 || AX_GCC_4_8_1 || AX_ICC_12_0 || AX_MSC_2012 )
// Allow typename outside of templates
#define AX_CXX_CI382 ( AX_ICC_12_0 )
// Default template arguments for function templates
#define AX_CXX_DR226 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_12_1 || AX_MSC_2013 )

//
//	CONCURRENCY
//

// exception_ptr: Propagating exceptions
#define AX_CXX_N2179 ( AX_CLANG_2_9 || AX_GCC_4_4 || AX_ICC_12_0 || AX_MSC_2010 )
// Reworded sequence points
#define AX_CXX_N2239 ( AX_CLANG_3_3 || AX_GCC_VER )
// quick_exit: Abandoning a process and at_quick_exit
#define AX_CXX_N2440 ( AX_GCC_4_8 )
// Atomic types and operations
#define AX_CXX_N2427 ( AX_CLANG_3_1 || AX_GCC_4_4 || AX_ICC_13_0 || AX_MSC_2012 )
// Memory model
#define AX_CXX_N2429 ( AX_CLANG_3_2 || AX_GCC_4_8 )
// Atomics in signal handlers
#define AX_CXX_N2547 ( AX_CLANG_3_1 || AX_GCC_VER )
// Thread-local storage
#define AX_CXX_N2659 ( AX_CLANG_3_3 || AX_GCC_4_8 )
// Magic statics: Dynamic initialization and destruction with concurrency
#define AX_CXX_N2660 ( AX_CLANG_2_9 || AX_GCC_4_3 )
// Data-dependency ordering
#define AX_CXX_N2664 ( AX_CLANG_3_2 || AX_GCC_4_4 || AX_MSC_2012 )
// Strong compare and exchange
#define AX_CXX_N2748 ( AX_CLANG_3_1 || AX_GCC_4_5 || AX_MSC_2012 )
// Bidirectional fences
#define AX_CXX_N2752 ( AX_CLANG_3_1 || AX_GCC_4_8 || AX_MSC_2012 )
// Data-dependency ordering: function annotation
#define AX_CXX_N2782 0

//
//	C99 SUPPORT
//

// C99 preprocessor
#define AX_CXX_N1653 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 )
// long long
#define AX_CXX_N1811 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 || AX_MSC_2010 )
// Extended integer types
#define AX_CXX_N1988 ( AX_GCC_VER )
// __func__
#define AX_CXX_N2340 ( AX_CLANG_2_9 || AX_GCC_4_3 || AX_ICC_11_1 )



/*
===============================================================================

	C++14 SUPPORT

===============================================================================
*/

//
//	CORE
//

// Tweak to certain C++ contextual conversions
#define AX_CXX_N3323 ( AX_CLANG_3_4 )
// Binary literals
#define AX_CXX_N3472 ( AX_CLANG_VER || AX_ICC_11_1 )
// decltype( auto ); return type deduction for normal functions
#define AX_CXX_N3638 ( AX_CLANG_3_4 || AX_MSC_2013_CTP )
// Initializd lambda captures
#define AX_CXX_N3648 ( AX_CLANG_3_4 )
// Generic lambdas
#define AX_CXX_N3649 ( AX_CLANG_3_4 )
// Variable templates
#define AX_CXX_N3651 ( AX_CLANG_3_4 )
// Relaxed requirements on constexpr functions
#define AX_CXX_N3652 ( AX_CLANG_3_4 )
// Member initializers and aggregates
#define AX_CXX_N3653 ( AX_CLANG_3_3 )
// Clarifying memory allocation
#define AX_CXX_N3664 ( AX_CLANG_3_4 )
// [[deprecated]] attribute
#define AX_CXX_N3760 ( AX_CLANG_3_4 )
// Single quotation mark as digit separator
#define AX_CXX_N3781 ( AX_CLANG_3_4 )
// Sized deallocation
#define AX_CXX_N3778 ( AX_CLANG_3_4 )

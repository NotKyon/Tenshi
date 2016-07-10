#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../Platform/Platform.hpp"

#ifndef AX_HAS_STDINT_H
# if !defined( _MSC_VER ) || _MSC_VER >= 1600
#  define AX_HAS_STDINT_H 1
# else
#  define AX_HAS_STDINT_H 0
# endif
#endif

#if AX_HAS_STDINT_H
# include <stdint.h>
typedef int8_t						ax_s8_t;
typedef int16_t						ax_s16_t;
typedef int32_t						ax_s32_t;
typedef int64_t						ax_s64_t;
typedef uint8_t						ax_u8_t;
typedef uint16_t					ax_u16_t;
typedef uint32_t					ax_u32_t;
typedef uint64_t					ax_u64_t;
#elif defined( _MSC_VER )
typedef signed __int8				ax_s8_t;
typedef signed __int16				ax_s16_t;
typedef signed __int32				ax_s32_t;
typedef signed __int64				ax_s64_t;
typedef unsigned __int8				ax_u8_t;
typedef unsigned __int16			ax_u16_t;
typedef unsigned __int32			ax_u32_t;
typedef unsigned __int64			ax_u64_t;
#else
typedef signed char					ax_s8_t;
typedef signed short				ax_s16_t;
typedef signed int					ax_s32_t;
typedef signed long long int		ax_s64_t;
typedef unsigned char				ax_u8_t;
typedef unsigned short				ax_u16_t;
typedef unsigned int				ax_u32_t;
typedef unsigned long long int		ax_u64_t;
#endif

#ifdef __cplusplus
typedef bool ax_bool_t;
#elif !defined( _MSC_VER ) || _MSC_VER >= 1600
# include <stdbool.h>
typedef _Bool						ax_bool_t;
#else
# ifndef __bool_true_false_are_defined
#  define __bool_true_false_are_defined 1
typedef enum { false = 0, true = 1 } _Bool;
# endif
typedef _Bool						ax_bool_t;
#endif

typedef size_t						ax_uptr_t;
typedef ptrdiff_t					ax_sptr_t;
typedef ax_u32_t					ax_uint_t;
typedef ax_s32_t					ax_sint_t;

ax_static_assert( sizeof( ax_u8_t ) == 1, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_u16_t ) == 2, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_u32_t ) == 4, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_u64_t ) == 8, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_s8_t ) == 1, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_s16_t ) == 2, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_s32_t ) == 4, "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_s64_t ) == 8, "Ax: Size mismatch" );

ax_static_assert( sizeof( ax_sptr_t ) == sizeof( void * ), "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_uptr_t ) == sizeof( void * ), "Ax: Size mismatch" );

ax_static_assert( sizeof( ax_uint_t ) >= sizeof( ax_u32_t ), "Ax: Size mismatch" );
ax_static_assert( sizeof( ax_sint_t ) >= sizeof( ax_s32_t ), "Ax: Size mismatch" );

#ifdef __cplusplus
namespace Ax
{

	typedef ax_u8_t					uint8;
	typedef ax_u16_t				uint16;
	typedef ax_u32_t				uint32;
	typedef ax_u64_t				uint64;

	typedef ax_s8_t					int8;
	typedef ax_s16_t				int16;
	typedef ax_s32_t				int32;
	typedef ax_s64_t				int64;

	typedef ax_uptr_t				uintptr;
	typedef ax_sptr_t				intptr;
	typedef ax_uint_t				uintcpu;
	typedef ax_sint_t				intcpu;

# if AX_CXX_N2431
	typedef decltype(nullptr)		nullptr_t;
# else
	typedef void *					nullptr_t;
# endif

	template< typename tType >
	void Swap( tType &a, tType &b )
	{
		const tType c = a;
		a = b;
		b = c;
	}

	enum class ECase
	{
		Sensitive,
		Insensitive
	};

}
#endif

#pragma once

#include "MemoryOrder.hpp"
#include "../../Core/Types.hpp"

#if ~AX_ARCH & AX_ARCH_64
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif

#define AX_ATOMIC_EXCHANGE_FULL32( Dst, Src )\
	( uint32( _InterlockedExchange( ( volatile long * )( Dst ), ( long )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULL32( Dst, Src, Cmp )\
	( uint32( _InterlockedCompareExchange( ( volatile long * )( Dst ), ( long )( Src ), ( long )( Cmp ) ) ) )

#define AX_ATOMIC_FETCH_ADD_FULL32( Dst, Src )\
	( uint32( _InterlockedExchangeAdd( ( volatile long * )( Dst ), ( long )( Src ) ) ) )
#define AX_ATOMIC_FETCH_SUB_FULL32( Dst, Src )\
	( uint32( _InterlockedExchangeAdd( ( volatile long * )( Dst ), -( long )( Src ) ) ) )
#define AX_ATOMIC_FETCH_AND_FULL32( Dst, Src )\
	( uint32( _InterlockedAnd( ( volatile long * )( Dst ), ( long )( Src ) ) ) )
#define AX_ATOMIC_FETCH_OR_FULL32( Dst, Src )\
	( uint32( _InterlockedOr( ( volatile long * )( Dst ), ( long )( Src ) ) ) )
#define AX_ATOMIC_FETCH_XOR_FULL32( Dst, Src )\
	( uint32( _InterlockedXor( ( volatile long * )( Dst ), ( long )( Src ) ) ) )


#if AX_ARCH & AX_ARCH_64
# define AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )\
	( uint64( _InterlockedExchange64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )\
	( uint64( _InterlockedCompareExchange64( ( volatile long long * )( Dst ), ( long long )( Src ), ( long long )( Cmp ) ) ) )

# define AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )\
	( uint64( _InterlockedExchangeAdd64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )\
	( uint64( _InterlockedExchangeAdd64( ( volatile long long * )( Dst ), -( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )\
	( uint64( _InterlockedAnd64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )\
	( uint64( _InterlockedOr64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )\
	( uint64( _InterlockedXor64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
#else
# define AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )\
	( uint64( InterlockedExchange64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )\
	( uint64( InterlockedCompareExchange64( ( volatile long long * )( Dst ), ( long long )( Src ), ( long long )( Cmp ) ) ) )

# define AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )\
	( uint64( InterlockedExchangeAdd64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )\
	( uint64( InterlockedExchangeAdd64( ( volatile long long * )( Dst ), -( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )\
	( uint64( InterlockedAnd64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )\
	( uint64( InterlockedOr64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
# define AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )\
	( uint64( InterlockedXor64( ( volatile long long * )( Dst ), ( long long )( Src ) ) ) )
#endif


#if AX_ARCH & AX_ARCH_64
# define AX_ATOMIC_EXCHANGE_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_EXCHANGE_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_COMPARE_EXCHANGE_FULLUPTR( Dst, Src, Cmp )\
	( uintptr( AX_ATOMIC_COMPARE_EXCHANGE_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ), ( uintptr )( Cmp ) ) ) )

# define AX_ATOMIC_FETCH_ADD_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_ADD_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_SUB_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_SUB_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_AND_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_AND_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_OR_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_OR_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_XOR_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_XOR_FULL64( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#else
# define AX_ATOMIC_EXCHANGE_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_EXCHANGE_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_COMPARE_EXCHANGE_FULLUPTR( Dst, Src, Cmp )\
	( uintptr( AX_ATOMIC_COMPARE_EXCHANGE_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ), ( uintptr )( Cmp ) ) ) )

# define AX_ATOMIC_FETCH_ADD_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_ADD_FULL632( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_SUB_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_SUB_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_AND_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_AND_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_OR_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_OR_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
# define AX_ATOMIC_FETCH_XOR_FULLUPTR( Dst, Src )\
	( uintptr( AX_ATOMIC_FETCH_XOR_FULL32( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#endif


#define AX_ATOMIC_EXCHANGE_FULLPTR( Dst, Src )\
	( ( void * )( _InterlockedExchangePointer( ( void *volatile * )( Dst ), ( void * )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( Dst, Src, Cmp )\
	( ( void * )( _InterlockedCompareExchangePointer( ( void *volatile * )( Dst ), ( void * )( Src ), ( void * )( Cmp ) ) ) )

#pragma once

#include "MemoryOrder.hpp"
#include "../../Core/Types.hpp"

#define AX_ATOMIC_EXCHANGE_FULL32( Dst, Src )\
	( uint32( __sync_lock_test_and_set( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULL32( Dst, Src, Cmp )\
	( uint32( __sync_val_compare_and_swap( ( volatile uint32 * )( Dst ), ( uint32 )( Src ), ( uint32 )( Cmp ) ) ) )

#define AX_ATOMIC_FETCH_ADD_FULL32( Dst, Src )\
	( uint32( __sync_fetch_and_add( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_SUB_FULL32( Dst, Src )\
	( uint32( __sync_fetch_and_sub( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_AND_FULL32( Dst, Src )\
	( uint32( __sync_fetch_and_and( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_OR_FULL32( Dst, Src )\
	( uint32( __sync_fetch_and_or( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_XOR_FULL32( Dst, Src )\
	( uint32( __sync_fetch_and_xor( ( volatile uint32 * )( Dst ), ( uint32 )( Src ) ) ) )


#define AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )\
	( uint64( __sync_lock_test_and_set( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )\
	( uint64( __sync_val_compare_and_swap( ( volatile uint64 * )( Dst ), ( uint64 )( Src ), ( uint64 )( Cmp ) ) ) )

#define AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )\
	( uint64( __sync_fetch_and_add( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )\
	( uint64( __sync_fetch_and_sub( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )\
	( uint64( __sync_fetch_and_and( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )\
	( uint64( __sync_fetch_and_or( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )
#define AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )\
	( uint64( __sync_fetch_and_xor( ( volatile uint64 * )( Dst ), ( uint64 )( Src ) ) ) )


#define AX_ATOMIC_EXCHANGE_FULLUPTR( Dst, Src )\
	( uintptr( __sync_lock_test_and_set( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULLUPTR( Dst, Src, Cmp )\
	( uintptr( __sync_val_compare_and_swap( ( volatile uintptr * )( Dst ), ( uintptr )( Src ), ( uint64 )( Cmp ) ) ) )

#define AX_ATOMIC_FETCH_ADD_FULLUPTR( Dst, Src )\
	( uintptr( __sync_fetch_and_add( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#define AX_ATOMIC_FETCH_SUB_FULLUPTR( Dst, Src )\
	( uintptr( __sync_fetch_and_sub( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#define AX_ATOMIC_FETCH_AND_FULLUPTR( Dst, Src )\
	( uintptr( __sync_fetch_and_and( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#define AX_ATOMIC_FETCH_OR_FULLUPTR( Dst, Src )\
	( uintptr( __sync_fetch_and_or( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )
#define AX_ATOMIC_FETCH_XOR_FULLUPTR( Dst, Src )\
	( uintptr( __sync_fetch_and_xor( ( volatile uintptr * )( Dst ), ( uintptr )( Src ) ) ) )


#define AX_ATOMIC_EXCHANGE_FULLPTR( Dst, Src )\
	( ( void * )( __sync_lock_test_and_set( ( void *volatile * )( Dst ), ( void * )( Src ) ) ) )
#define AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( Dst, Src, Cmp )\
	( ( void * )( __sync_val_compare_and_swap( ( void *volatile * )( Dst ), ( void * )( Src ), ( void * )( Cmp ) ) ) )

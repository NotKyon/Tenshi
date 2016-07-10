#pragma once

#include "Detail/MemoryOrder.hpp"
#if defined( _MSC_VER ) || defined( __INTEL_COMPILER )
# include "Detail/Atomics_MSVC.hpp"
#elif defined( __GNUC__ ) || defined( __clang__ )
# include "Detail/Atomics_GCC.hpp"
#else
# error Atomics: Unsupported compiler
#endif
#include "../Core/Types.hpp"

/*
===============================================================================

	ATOMIC MACROS - ACQUIRE

===============================================================================
*/
#ifndef AX_ATOMIC_EXCHANGE_ACQ32
# define AX_ATOMIC_EXCHANGE_ACQ32( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL32( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_ACQ32( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL32( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_ACQ32
# define AX_ATOMIC_FETCH_ADD_ACQ32( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_ACQ32( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_AND_ACQ32( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_OR_ACQ32( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_ACQ32( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL32( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_ACQ64
# define AX_ATOMIC_EXCHANGE_ACQ64( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_ACQ64( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_ACQ64
# define AX_ATOMIC_FETCH_ADD_ACQ64( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_ACQ64( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_AND_ACQ64( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_OR_ACQ64( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_ACQ64( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_ACQPTR
# define AX_ATOMIC_EXCHANGE_ACQPTR( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULLPTR( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_ACQPTR( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( Dst, Src, Cmp )
#endif


/*
===============================================================================

	ATOMIC MACROS - RELEASE

===============================================================================
*/
#ifndef AX_ATOMIC_EXCHANGE_REL32
# define AX_ATOMIC_EXCHANGE_REL32( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL32( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_REL32( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL32( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_REL32
# define AX_ATOMIC_FETCH_ADD_REL32( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_REL32( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_AND_REL32( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_OR_REL32( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_REL32( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL32( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_REL64
# define AX_ATOMIC_EXCHANGE_REL64( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_REL64( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_REL64
# define AX_ATOMIC_FETCH_ADD_REL64( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_REL64( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_AND_REL64( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_OR_REL64( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_REL64( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_RELPTR
# define AX_ATOMIC_EXCHANGE_RELPTR( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULLPTR( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_RELPTR( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( Dst, Src, Cmp )
#endif


/*
===============================================================================

	ATOMIC MACROS - RELAXED (NON-SEQUENTIAL)

===============================================================================
*/
#ifndef AX_ATOMIC_EXCHANGE_NSQ32
# define AX_ATOMIC_EXCHANGE_NSQ32( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL32( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_NSQ32( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL32( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_NSQ32
# define AX_ATOMIC_FETCH_ADD_NSQ32( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_NSQ32( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_AND_NSQ32( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_OR_NSQ32( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL32( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_NSQ32( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL32( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_NSQ64
# define AX_ATOMIC_EXCHANGE_NSQ64( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULL64( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_NSQ64( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULL64( Dst, Src, Cmp )
#endif
#ifndef AX_ATOMIC_FETCH_ADD_NSQ64
# define AX_ATOMIC_FETCH_ADD_NSQ64( Dst, Src )\
	AX_ATOMIC_FETCH_ADD_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_SUB_NSQ64( Dst, Src )\
	AX_ATOMIC_FETCH_SUB_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_AND_NSQ64( Dst, Src )\
	AX_ATOMIC_FETCH_AND_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_OR_NSQ64( Dst, Src )\
	AX_ATOMIC_FETCH_OR_FULL64( Dst, Src )
# define AX_ATOMIC_FETCH_XOR_NSQ64( Dst, Src )\
	AX_ATOMIC_FETCH_XOR_FULL64( Dst, Src )
#endif

#ifndef AX_ATOMIC_EXCHANGE_NSQPTR
# define AX_ATOMIC_EXCHANGE_NSQPTR( Dst, Src )\
	AX_ATOMIC_EXCHANGE_FULLPTR( Dst, Src )
# define AX_ATOMIC_COMPARE_EXCHANGE_NSQPTR( Dst, Src, Cmp )\
	AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( Dst, Src, Cmp )
#endif


namespace Ax { namespace Async {

	/*
	===========================================================================

		ATOMICS - FULL BARRIER - 32-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int32 AtomicSet( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_EXCHANGE_FULL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSetEq( volatile int32 *dst, int32 src, int32 cmp, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_FULL32( dst, src, cmp );
	}
	AX_FORCEINLINE int32 AtomicInc( volatile int32 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicDec( volatile int32 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicAdd( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSub( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicAnd( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_AND_FULL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicOr( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_OR_FULL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicXor( volatile int32 *dst, int32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_XOR_FULL32( dst, src );
	}


	AX_FORCEINLINE uint32 AtomicSet( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_EXCHANGE_FULL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSetEq( volatile uint32 *dst, uint32 src, uint32 cmp, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_FULL32( dst, src, cmp );
	}
	AX_FORCEINLINE uint32 AtomicInc( volatile uint32 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicDec( volatile uint32 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicAdd( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSub( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicAnd( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_AND_FULL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicOr( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_OR_FULL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicXor( volatile uint32 *dst, uint32 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_XOR_FULL32( dst, src );
	}


	/*
	===========================================================================

		ATOMICS - FULL BARRIER - 64-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int64 AtomicSet( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_EXCHANGE_FULL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSetEq( volatile int64 *dst, int64 src, int64 cmp, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_FULL64( dst, src, cmp );
	}
	AX_FORCEINLINE int64 AtomicInc( volatile int64 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicDec( volatile int64 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicAdd( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSub( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicAnd( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_AND_FULL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicOr( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_OR_FULL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicXor( volatile int64 *dst, int64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_XOR_FULL64( dst, src );
	}


	AX_FORCEINLINE uint64 AtomicSet( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_EXCHANGE_FULL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSetEq( volatile uint64 *dst, uint64 src, uint64 cmp, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_FULL64( dst, src, cmp );
	}
	AX_FORCEINLINE uint64 AtomicInc( volatile uint64 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicDec( volatile uint64 *dst, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicAdd( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_ADD_FULL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSub( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_SUB_FULL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicAnd( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_AND_FULL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicOr( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_OR_FULL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicXor( volatile uint64 *dst, uint64 src, Mem::Full = Mem::Full() )
	{
		return AX_ATOMIC_FETCH_XOR_FULL64( dst, src );
	}

	/*
	===========================================================================

		ATOMICS - FULL BARRIER - POINTERS

	===========================================================================
	*/
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtr( tDst *volatile *dst, tSrc *src, Mem::Full = Mem::Full() )
	{
		return ( tDst * )AX_ATOMIC_EXCHANGE_FULLPTR( dst, src );
	}
	template< typename tPtr >
	AX_FORCEINLINE tPtr *AtomicSetPtr( tPtr *volatile *dst, decltype( nullptr ), Mem::Full = Mem::Full() )
	{
		return ( tPtr * )AX_ATOMIC_EXCHANGE_FULLPTR( dst, NULL );
	}

	template< typename tDst, typename tSrc, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, tCmp *cmp, Mem::Full = Mem::Full() )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( dst, src, cmp );
	}
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, decltype( nullptr ), Mem::Full = Mem::Full() )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( dst, src, NULL );
	}
	template< typename tDst, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, decltype( nullptr ), tCmp *cmp, Mem::Full = Mem::Full() )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_FULLPTR( dst, NULL, cmp );
	}
	

	/*
	===========================================================================

		ATOMICS - ACQUIRE BARRIER - 32-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int32 AtomicSet( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_EXCHANGE_ACQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSetEq( volatile int32 *dst, int32 src, int32 cmp, Mem::Acquire )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_ACQ32( dst, src, cmp );
	}
	AX_FORCEINLINE int32 AtomicInc( volatile int32 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicDec( volatile int32 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicAdd( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSub( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicAnd( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_AND_ACQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicOr( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_OR_ACQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicXor( volatile int32 *dst, int32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_XOR_ACQ32( dst, src );
	}


	AX_FORCEINLINE uint32 AtomicSet( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_EXCHANGE_ACQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSetEq( volatile uint32 *dst, uint32 src, uint32 cmp, Mem::Acquire )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_ACQ32( dst, src, cmp );
	}
	AX_FORCEINLINE uint32 AtomicInc( volatile uint32 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicDec( volatile uint32 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicAdd( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSub( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicAnd( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_AND_ACQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicOr( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_OR_ACQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicXor( volatile uint32 *dst, uint32 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_XOR_ACQ32( dst, src );
	}


	/*
	===========================================================================

		ATOMICS - ACQUIRE BARRIER - 64-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int64 AtomicSet( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_EXCHANGE_ACQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSetEq( volatile int64 *dst, int64 src, int64 cmp, Mem::Acquire )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_ACQ64( dst, src, cmp );
	}
	AX_FORCEINLINE int64 AtomicInc( volatile int64 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicDec( volatile int64 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicAdd( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSub( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicAnd( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_AND_ACQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicOr( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_OR_ACQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicXor( volatile int64 *dst, int64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_XOR_ACQ64( dst, src );
	}


	AX_FORCEINLINE uint64 AtomicSet( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_EXCHANGE_ACQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSetEq( volatile uint64 *dst, uint64 src, uint64 cmp, Mem::Acquire )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_ACQ64( dst, src, cmp );
	}
	AX_FORCEINLINE uint64 AtomicInc( volatile uint64 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicDec( volatile uint64 *dst, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicAdd( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_ADD_ACQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSub( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_SUB_ACQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicAnd( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_AND_ACQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicOr( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_OR_ACQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicXor( volatile uint64 *dst, uint64 src, Mem::Acquire )
	{
		return AX_ATOMIC_FETCH_XOR_ACQ64( dst, src );
	}

	/*
	===========================================================================

		ATOMICS - ACQUIRE BARRIER - POINTERS

	===========================================================================
	*/
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtr( tDst *volatile *dst, tSrc *src, Mem::Acquire )
	{
		return ( tDst * )AX_ATOMIC_EXCHANGE_ACQPTR( dst, src );
	}
	template< typename tPtr >
	AX_FORCEINLINE tPtr *AtomicSetPtr( tPtr *volatile *dst, decltype( nullptr ), Mem::Acquire )
	{
		return ( tPtr * )AX_ATOMIC_EXCHANGE_ACQPTR( dst, NULL );
	}

	template< typename tDst, typename tSrc, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, tCmp *cmp, Mem::Acquire )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_ACQPTR( dst, src, cmp );
	}
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, decltype( nullptr ), Mem::Acquire )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_ACQPTR( dst, src, NULL );
	}
	template< typename tDst, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, decltype( nullptr ), tCmp *cmp, Mem::Acquire )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_ACQPTR( dst, NULL, cmp );
	}
	

	/*
	===========================================================================

		ATOMICS - RELEASE BARRIER - 32-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int32 AtomicSet( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_EXCHANGE_REL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSetEq( volatile int32 *dst, int32 src, int32 cmp, Mem::Release )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_REL32( dst, src, cmp );
	}
	AX_FORCEINLINE int32 AtomicInc( volatile int32 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicDec( volatile int32 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicAdd( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSub( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicAnd( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_AND_REL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicOr( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_OR_REL32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicXor( volatile int32 *dst, int32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_XOR_REL32( dst, src );
	}


	AX_FORCEINLINE uint32 AtomicSet( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_EXCHANGE_REL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSetEq( volatile uint32 *dst, uint32 src, uint32 cmp, Mem::Release )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_REL32( dst, src, cmp );
	}
	AX_FORCEINLINE uint32 AtomicInc( volatile uint32 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicDec( volatile uint32 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicAdd( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSub( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicAnd( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_AND_REL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicOr( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_OR_REL32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicXor( volatile uint32 *dst, uint32 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_XOR_REL32( dst, src );
	}


	/*
	===========================================================================

		ATOMICS - RELEASE BARRIER - 64-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int64 AtomicSet( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_EXCHANGE_REL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSetEq( volatile int64 *dst, int64 src, int64 cmp, Mem::Release )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_REL64( dst, src, cmp );
	}
	AX_FORCEINLINE int64 AtomicInc( volatile int64 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicDec( volatile int64 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicAdd( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSub( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicAnd( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_AND_REL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicOr( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_OR_REL64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicXor( volatile int64 *dst, int64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_XOR_REL64( dst, src );
	}


	AX_FORCEINLINE uint64 AtomicSet( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_EXCHANGE_REL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSetEq( volatile uint64 *dst, uint64 src, uint64 cmp, Mem::Release )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_REL64( dst, src, cmp );
	}
	AX_FORCEINLINE uint64 AtomicInc( volatile uint64 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicDec( volatile uint64 *dst, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicAdd( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_ADD_REL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSub( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_SUB_REL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicAnd( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_AND_REL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicOr( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_OR_REL64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicXor( volatile uint64 *dst, uint64 src, Mem::Release )
	{
		return AX_ATOMIC_FETCH_XOR_REL64( dst, src );
	}

	/*
	===========================================================================

		ATOMICS - RELEASE BARRIER - POINTERS

	===========================================================================
	*/
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtr( tDst *volatile *dst, tSrc *src, Mem::Release )
	{
		return ( tDst * )AX_ATOMIC_EXCHANGE_RELPTR( dst, src );
	}
	template< typename tPtr >
	AX_FORCEINLINE tPtr *AtomicSetPtr( tPtr *volatile *dst, decltype( nullptr ), Mem::Release )
	{
		return ( tPtr * )AX_ATOMIC_EXCHANGE_RELPTR( dst, NULL );
	}

	template< typename tDst, typename tSrc, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, tCmp *cmp, Mem::Release )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_RELPTR( dst, src, cmp );
	}
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, decltype( nullptr ), Mem::Release )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_RELPTR( dst, src, NULL );
	}
	template< typename tDst, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, decltype( nullptr ), tCmp *cmp, Mem::Release )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_RELPTR( dst, NULL, cmp );
	}
	

	/*
	===========================================================================

		ATOMICS - RELAXED BARRIER - 32-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int32 AtomicSet( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_EXCHANGE_NSQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSetEq( volatile int32 *dst, int32 src, int32 cmp, Mem::Relaxed )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_NSQ32( dst, src, cmp );
	}
	AX_FORCEINLINE int32 AtomicInc( volatile int32 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicDec( volatile int32 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ32( dst, 1 );
	}
	AX_FORCEINLINE int32 AtomicAdd( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicSub( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicAnd( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_AND_NSQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicOr( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_OR_NSQ32( dst, src );
	}
	AX_FORCEINLINE int32 AtomicXor( volatile int32 *dst, int32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_XOR_NSQ32( dst, src );
	}


	AX_FORCEINLINE uint32 AtomicSet( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_EXCHANGE_NSQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSetEq( volatile uint32 *dst, uint32 src, uint32 cmp, Mem::Relaxed )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_NSQ32( dst, src, cmp );
	}
	AX_FORCEINLINE uint32 AtomicInc( volatile uint32 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicDec( volatile uint32 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ32( dst, 1 );
	}
	AX_FORCEINLINE uint32 AtomicAdd( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicSub( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicAnd( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_AND_NSQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicOr( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_OR_NSQ32( dst, src );
	}
	AX_FORCEINLINE uint32 AtomicXor( volatile uint32 *dst, uint32 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_XOR_NSQ32( dst, src );
	}


	/*
	===========================================================================

		ATOMICS - RELAXED BARRIER - 64-BIT

	===========================================================================
	*/
	AX_FORCEINLINE int64 AtomicSet( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_EXCHANGE_NSQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSetEq( volatile int64 *dst, int64 src, int64 cmp, Mem::Relaxed )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_NSQ64( dst, src, cmp );
	}
	AX_FORCEINLINE int64 AtomicInc( volatile int64 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicDec( volatile int64 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ64( dst, 1 );
	}
	AX_FORCEINLINE int64 AtomicAdd( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicSub( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicAnd( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_AND_NSQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicOr( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_OR_NSQ64( dst, src );
	}
	AX_FORCEINLINE int64 AtomicXor( volatile int64 *dst, int64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_XOR_NSQ64( dst, src );
	}


	AX_FORCEINLINE uint64 AtomicSet( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_EXCHANGE_NSQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSetEq( volatile uint64 *dst, uint64 src, uint64 cmp, Mem::Relaxed )
	{
		return AX_ATOMIC_COMPARE_EXCHANGE_NSQ64( dst, src, cmp );
	}
	AX_FORCEINLINE uint64 AtomicInc( volatile uint64 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicDec( volatile uint64 *dst, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ64( dst, 1 );
	}
	AX_FORCEINLINE uint64 AtomicAdd( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_ADD_NSQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicSub( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_SUB_NSQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicAnd( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_AND_NSQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicOr( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_OR_NSQ64( dst, src );
	}
	AX_FORCEINLINE uint64 AtomicXor( volatile uint64 *dst, uint64 src, Mem::Relaxed )
	{
		return AX_ATOMIC_FETCH_XOR_NSQ64( dst, src );
	}

	/*
	===========================================================================

		ATOMICS - RELAXED BARRIER - POINTERS

	===========================================================================
	*/
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtr( tDst *volatile *dst, tSrc *src, Mem::Relaxed )
	{
		return ( tDst * )AX_ATOMIC_EXCHANGE_NSQPTR( dst, src );
	}
	template< typename tPtr >
	AX_FORCEINLINE tPtr *AtomicSetPtr( tPtr *volatile *dst, decltype( nullptr ), Mem::Relaxed )
	{
		return ( tPtr * )AX_ATOMIC_EXCHANGE_NSQPTR( dst, NULL );
	}

	template< typename tDst, typename tSrc, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, tCmp *cmp, Mem::Relaxed )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_NSQPTR( dst, src, cmp );
	}
	template< typename tDst, typename tSrc >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, tSrc *src, decltype( nullptr ), Mem::Relaxed )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_NSQPTR( dst, src, NULL );
	}
	template< typename tDst, typename tCmp >
	AX_FORCEINLINE tDst *AtomicSetPtrEq( tDst *volatile *dst, decltype( nullptr ), tCmp *cmp, Mem::Relaxed )
	{
		return ( tDst * )AX_ATOMIC_COMPARE_EXCHANGE_NSQPTR( dst, NULL, cmp );
	}


	/*
	===========================================================================

		ATOMICS - WRAPPER CLASS

	===========================================================================
	*/
	typedef uintcpu atomic_t;

	struct SAtomicInteger
	{
	public:
									// Constructor
		AX_FORCEINLINE				SAtomicInteger		( atomic_t value = 0 )
									: mValue( value )
									{
									}
									// Destructor
		AX_FORCEINLINE				~SAtomicInteger		()
									{
									}

									// Increment the current value
		AX_FORCEINLINE atomic_t		Increment			()
									{
										return AtomicInc( &mValue, Mem::Acquire() );
									}
									// Decrement the current value
		AX_FORCEINLINE atomic_t		Decrement			()
									{
										return AtomicDec( &mValue, Mem::Release() );
									}

									// Set the current value
		AX_FORCEINLINE atomic_t		Set					( uintcpu x )
									{
										return AtomicSet( &mValue, x );
									}
									// Retrieve the current value
		AX_FORCEINLINE atomic_t		Get					() const
									{
										return mValue;
									}

									// Prefix increment
		AX_FORCEINLINE
		SAtomicInteger &			operator++			()
									{
										AtomicInc( &mValue, Mem::Acquire() );
										return *this;
									}
									// Postfix increment
		AX_FORCEINLINE atomic_t		operator++			( int )
									{
										return AtomicInc( &mValue, Mem::Acquire() );
									}

									// Prefix decrement
		AX_FORCEINLINE
		SAtomicInteger &			operator--			()
									{
										AtomicDec( &mValue, Mem::Release() );
										return *this;
									}
									// Postfix decrement
		AX_FORCEINLINE atomic_t		operator--			( int )
									{
										return AtomicDec( &mValue, Mem::Release() );
									}

	private:
		volatile atomic_t			mValue;
	};

}}

#pragma once

#include "../Platform/Platform.hpp"
#include "../Core/TypeTraits.hpp"

namespace Ax { namespace Math {

	/// Unsigned bit-shift right
	template< typename tInt >
	inline tInt BitShiftRightU( tInt x, tInt y )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		typedef typename TMakeUnsigned< tInt >::type tUnsigned;

		return
			static_cast< tInt >(
				static_cast< tUnsigned >( x ) >> y
			);
	}
	/// Signed bit-shift right
	template< typename tInt >
	inline tInt BitShiftRightS( tInt x, tInt y )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		typedef typename TMakeSigned< tInt >::type tSigned;

		return
			static_cast< tInt >(
				static_cast< tSigned >( x ) >> y
			);
	}

	/// Put a zero bit between each of the lower bits of the given value
	template< typename tInt >
	inline tInt BitExpand( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );

		tInt r = 0;

		for( uintcpu i = 0; i < ( sizeof( x )*8 )/2; ++i )
		{
			r |= ( x & ( 1 << i ) ) << i;
		}

		return r;
	}
	/// Take each other bit of a value and merge into one value
	template< typename tInt >
	inline tInt BitMerge( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );

		tInt r = 0;

		for( uintcpu i = 0; i < ( sizeof( x )*8 )/2; ++i )
		{
			r |= ( x & ( 1 << ( i*2 ) ) ) >> i;
		}

		return r;
	}

	/// Specialization of BitExpand() for uint32
	inline uint32 BitExpand( uint32 x )
	{
		// http://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
		x &= 0xFFFF;

		x = ( x ^ ( x << 8 ) ) & 0x00FF00FF;
		x = ( x ^ ( x << 4 ) ) & 0x0F0F0F0F;
		x = ( x ^ ( x << 2 ) ) & 0x33333333;
		x = ( x ^ ( x << 1 ) ) & 0x55555555;

		return x;
	}
	/// Specialization of BitMerge() for uint32
	inline uint32 BitMerge( uint32 x )
	{
		// http://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
		x &= 0x55555555;

		x = ( x ^ ( x >> 1 ) ) & 0x33333333;
		x = ( x ^ ( x >> 2 ) ) & 0x0F0F0F0F;
		x = ( x ^ ( x >> 4 ) ) & 0x00FF00FF;
		x = ( x ^ ( x >> 8 ) ) & 0x0000FFFF;

		return x;
	}

	/// Turn off the right-most set bit (e.g., 01011000 -> 01010000)
	template< typename tInt >
	inline tInt BitRemoveLowestSet( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return x & ( x - 1 );
	}
	/// Determine whether a number is a power of two
	template< typename tInt >
	inline bool BitIsPowerOfTwo( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return BitRemoveLowestSet( x ) == 0;
	}
	/// Isolate the right-most set bit (e.g., 01011000 -> 00001000)
	template< typename tInt >
	inline tInt BitIsolateLowestSet( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return x & ( -x );
	}
	/// Isolate the right-most clear bit (e.g., 10100111 -> 00001000)
	template< typename tInt >
	inline tInt BitIsolateLowestClear( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return -x & ( x + 1 );
	}
	/// Create a mask of the trailing clear bits (e.g., 01011000 -> 00000111)
	template< typename tInt >
	inline tInt BitIdentifyLowestClears( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return -x & ( x - 1 );
	}
	/// Create a mask that identifies the least significant set bit and the
	/// trailing clear bits (e.g., 01011000 -> 00001111)
	template< typename tInt >
	inline tInt BitIdentifyLowestSetAndClears( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return x ^ ( x - 1 );
	}
	/// Propagate the lowest set bit to the lower clear bits
	template< typename tInt >
	inline tInt BitPropagateLowestSet( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return x | ( x - 1 );
	}
	/// Find the absolute value of an integer
	template< typename tInt >
	inline tInt BitAbs( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		const tInt y = BitShiftRightS( x, ( tInt )( sizeof( x )*8 - 1 ) );
		return ( x ^ y ) - y;
	}
	/// Find the sign of an integer
	template< typename tInt >
	inline tInt BitSign( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		static const tInt s = sizeof( tInt )*8 - 1;
		return BitShiftRightS( x, s ) | BitShiftRightU( -x, s );
	}
	/// Transfer the sign of src into dst
	template< typename tInt >
	inline tInt BitCopySign( tInt dst, tInt src )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		const tInt t = BitShiftRightS( src, ( tInt )( sizeof( tInt )*8 - 1 ) );
		return ( BitAbs( dst ) + t ) ^ t;
	}
	/// Rotate a field of bits left
	template< typename tInt >
	inline tInt BitRotateLeft( tInt x, tInt y )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return ( x << y ) | BitShiftRightU( x, sizeof( x )*8 - y );
	}
	/// Rotate a field of bits right
	template< typename tInt >
	inline tInt BitRotateRight( tInt x, tInt y )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );
		return BitShiftRightU( x, y ) | ( x << ( sizeof( x )*8 - y ) );
	}
	/// Count the number of set bits
	template< typename tInt >
	inline tInt BitCount( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );

		tInt r = x;

		for( int i = 0; i < sizeof( x )*8; ++i )
		{
			r -= x >> ( 1 << i );
		}

		return r;
	}
	/// Specialization of BitCount() for 32-bit integers
	inline uint32 BitCount( uint32 x )
	{
		x = x - ( ( x >> 1 ) & 0x55555555 );
		x = ( x & 0x33333333 ) + ( ( x >> 2 ) & 0x33333333 );
		x = ( x + ( x >> 4 ) ) & 0x0F0F0F0F;
		x = x + ( x >> 8 );
		x = x + ( x >> 16 );
		return x & 0x0000003F;
	}
	/// Compute the parity of an integer (true for odd, false for even)
	template< typename tInt >
	inline tInt BitParity( tInt x )
	{
		static_assert( TIsInt< tInt >::value, "Integer type required" );

		x = x ^ ( x >> 1 );
		x = x ^ ( x >> 2 );
		x = x ^ ( x >> 4 );

		if( sizeof( x ) > 1 )
		{
			x = x ^ ( x >> 8 );
		
			if( sizeof( x ) > 2 )
			{
				x = x ^ ( x >> 16 );

				if( sizeof( x ) > 4 )
				{
					x = x ^ ( x >> 32 );

					int i = 8;
					while( sizeof( x ) > i )
					{
						x = x ^ ( x >> ( i*8 ) );
						i = i*2;
					}
				}
			}
		}

		return x;
	}

	/// Treat the bits of a signed-integer as a float encoding.
	inline float IntBitsToFloat( int32 x )
	{
		union
		{
			float f;
			int32 i;
		} v;

		v.i = x;
		return v.f;
	}
	inline double IntBitsToFloat( int64 x )
	{
		union
		{
			double f;
			int64 i;
		} v;

		v.i = x;
		return v.f;
	}

	/// Treat the bits of an unsigned-integer as a float encoding.
	inline float UintBitsToFloat( uint32 x )
	{
		union
		{
			float f;
			uint32 i;
		} v;
		
		v.i = x;
		return v.f;
	}
	inline double UintBitsToFloat( uint64 x )
	{
		union
		{
			double f;
			uint64 i;
		} v;
		
		v.i = x;
		return v.f;
	}
	
	/// Retrieve the encoding of a float's bits as a signed-integer.
	inline int32 FloatToIntBits( float x )
	{
		union
		{
			float f;
			int32 i;
		} v;
		
		v.f = x;
		return v.i;
	}
	inline int64 FloatToIntBits( double x )
	{
		union
		{
			double f;
			int64 i;
		} v;
		
		v.f = x;
		return v.i;
	}
	
	/// Retrieve the encoding of a float's bits as an unsigned-integer.
	inline uint32 FloatToUintBits( float x )
	{
		union
		{
			float f;
			uint32 i;
		} v;
		
		v.f = x;
		return v.i;
	}
	inline uint64 FloatToUintBits( double x )
	{
		union
		{
			double f;
			uint64 i;
		} v;
		
		v.f = x;
		return v.i;
	}

	/// Check whether a floating-point value is a NAN.
	inline bool IsNAN( float x )
	{
		const uint32 xi = FloatToUintBits( x );
		return ( xi & 0x7F800000 ) == 0x7F800000 && ( xi & 0x7FFFFF ) != 0;
	}
	/// Check whether a floating-point value is infinity.
	inline bool IsInf( float x )
	{
		return ( FloatToUintBits( x ) & 0x7FFFFFFF ) == 0x7F800000;
	}

}}

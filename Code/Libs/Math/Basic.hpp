#pragma once

#include "Const.hpp"
#include "Bits.hpp"

#include "Intrinsics.hpp"

/// @def AX_LB(x)
/// Retrieve the index of the last bit of an integer, \a x.
#define AX_LB( x ) ( sizeof( x )*8 - 1 )

namespace Ax { namespace Math {

	/// Convert degrees to radians.
	inline float Degrees( float x )
	{
		return x/180.0f*AX_PI;
	}
	/// Convert radians to degrees.
	inline float Radians( float x )
	{
		return x/AX_PI*180.0f;
	}

	/// Get the sign of a number.
	///
	/// @return -1, 0, or 1.
	inline float Sign( float x )
	{
		return ( float )( 1|( ( int32 )x >> AX_LB( x ) ) );
	}
	/// Get the sign of an integer number.
	///
	/// @return -1, 0, or 1.
	inline int32 Sign( int32 x )
	{
		return BitSign( x );
	}

	/// Get 1 if the value is > 0 or 0 if <= 0
	///
	/// @return 1 or 0
	inline float Heaviside( float x )
	{
#if 0
		// TODO: Test this code path
		return UintBitsToFloat( ~FloatToUintBits( x ) >> 31 );
#else
		return x > 0.0f ? 1.0f : 0.0f;
#endif
	}

	/// Get the absolute value of a number.
	inline float Abs( float x )
	{
		return UintBitsToFloat( FloatToUintBits( x ) & ( uint32 )~AX_LB( x ) );
	}
	/// Get the absolute value of an integer number.
	inline int32 Abs( int32 x )
	{
		return BitAbs( x );
	}

	/// Copy the sign of 'b' into 'a'.
	inline float CopySign( float a, float b )
	{
		return Sign( b )*Abs( a );
	}
	/// Copy the sign of 'b' into 'a'
	inline int32 CopySign( int32 a, int32 b )
	{
		return BitCopySign( a, b );
	}

	/// Find the minimum of two integer values.
	inline int32 Min( int32 a, int32 b )
	{
		return a < b ? a : b;
	}
	/// Find the maximum of two integer values.
	inline int32 Max( int32 a, int32 b )
	{
		return a > b ? a : b;
	}
	/// Find the minimum of two values.
	inline float Min( float a, float b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		float r;
		_mm_store_ss( &r, _mm_min_ss( _mm_set_ss( a ), _mm_set_ss( b ) ) );
		return r;
#else
		return a < b ? a : b;
#endif
	}
	/// Find the maximum of two values.
	inline float Max( float a, float b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		float r;
		_mm_store_ss( &r, _mm_max_ss( _mm_set_ss( a ), _mm_set_ss( b ) ) );
		return r;
#else
		return a > b ? a : b;
#endif
	}
	/// Clamp a value to a range.
	inline float Clamp( float x, float l, float h )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		float r = 0.0f;
		return _mm_store_ss( &r, _mm_min_ss( _mm_max_ss( _mm_set_ss( x ), _mm_set_ss( l ) ), _mm_set_ss( h ) ) ), r;
#else
		float r = x;

		r = ( r + h - Abs( r - h ) )*0.5f;
		r = ( r + l - Abs( r - l ) )*0.5f;

		return r;
#endif
	}
	/// Clamp a value to the range of 0 to 1.
	inline float Saturate( float x )
	{
		return Clamp( x, 0.0f, 1.0f );
	}

	/// Linearly interpolate between two values.
	template< typename T, typename U >
	inline T Lerp( const T &x, const T &y, const U &t )
	{
		return x + ( y - x )*t;
	}
	/// Cubically interpolate between four values.
	template< typename T, typename U >
	inline T Cerp( const T &x, const T &y, const T &z, const T &w, const U &t )
	{
		const T a = ( w - z ) - ( x - y );
		const T b = ( x - y ) - a;
		const T c = z - x;
		return t*( t*( t*a + b ) + c ) + y;
	}

	/// Round a value down
	inline float Floor( float f )
	{
		if( ( FloatToUintBits( f ) & 0x801FFFFF ) > 0x80000000 ) {
			return ( float )( ( int )f - 1 );
		}

		return ( float )( int )f;
	}

	/// Wrap an angle between 0 and 360.
	inline float Wrap360( float angle )
	{
		// This simultaneously checks whether angle is above 360 or below 0
		// 0x43B40000 is the integer bits representation 360.0f
		if( FloatToUintBits( angle ) >= 0x43B40000 ) {
			angle -= Floor( angle/360.0f )*360.0f;
		}
		
		return angle;
	}
	/// Wrap an angle between -180 and 180.
	inline float Wrap180( float angle )
	{
		angle = Wrap360( angle );
		return angle > 180.0f ? angle - 360.0f : angle;
	}

	/// Calculate the delta between two angles.
	///
	/// @return Angle between -180 and 180.
	inline float AngleDelta( float a, float b )
	{
		return Wrap180( a - b );
	}

	/// Get the fractional part of a float.
	inline float Frac( float x )
	{
		return x - Floor( x );
	}

	/// Approximate a square root.
	inline float FastSqrt( float x )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_store_ss( &x, _mm_sqrt_ss( _mm_load_ss( &x ) ) ), x;
#else
		int32 i = FloatToIntBits( x );

		i  -= 0x3F800000;
		i >>= 1;
		i  += 0x3F800000;

		return IntBitsToFloat( i );
#endif
	}
	/// Approximate the reciprocal of a square root.
	inline float FastInvSqrt( float x )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_store_ss( &x, _mm_rsqrt_ss( _mm_load_ss( &x ) ) ), x;
#else
		const float f = IntBitsToFloat( 0x5F3759DF - ( FloatToIntBits( x ) >> 1 ) );
		return f*( 1.5f - 0.5f*x*f*f );
#endif
	}

}}

#pragma once

#include "../Platform/Platform.hpp"
#include "../Core/TypeTraits.hpp"

#include "Intrinsics.hpp"

namespace Ax { namespace Math {

	/// @typedef float v128[ 4 ]
	/// @brief 128-bit SIMD vector intrinsic type.
	///
	/// @typedef v128 p_v128
	/// @brief 128-bit SIMD vector type for parameter passing.
	
#if AX_INTRIN & AX_INTRIN_SSE
	typedef __m128					v128;
	
	typedef v128					p_v128;
#elif AX_INTRIN == 0
	struct v128
	{
		union
		{
			float					f[ 4 ];
			int32					i[ 4 ];
			uint32					u[ 4 ];
		};
	};

	typedef const v128 &			p_v128;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif

	/*
	===========================================================================
	
		BASIC VECTOR PROCESSING FUNCTIONS
	
	===========================================================================
	*/

	/// Get a vector filled with zeros.
	inline v128 AX_VCALL VecZero()
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_setzero_ps();
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = 0.0f;
		r.f[ 1 ] = 0.0f;
		r.f[ 2 ] = 0.0f;
		r.f[ 3 ] = 0.0f;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Calculate the negation of a vector.
	inline v128 AX_VCALL VecNegate( p_v128 a )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_sub_ps( _mm_setzero_ps(), a );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = -a.f[ 0 ];
		r.f[ 1 ] = -a.f[ 1 ];
		r.f[ 2 ] = -a.f[ 2 ];
		r.f[ 3 ] = -a.f[ 3 ];

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}

	/// Set a vector to a collection of floating-point values.
	inline v128 AX_VCALL VecSet( float x, float y, float z, float w )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_set_ps( w, z, y, x );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = x;
		r.f[ 1 ] = y;
		r.f[ 2 ] = z;
		r.f[ 3 ] = w;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Set a vector to a collection of integer values.
	inline v128 AX_VCALL VecSetInt( uint32 x, uint32 y, uint32 z, uint32 w )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_castsi128_ps( _mm_set_epi32( w, z, y, x ) );
#elif AX_INTRIN == 0
		v128 r;

		r.u[ 0 ] = x;
		r.u[ 1 ] = y;
		r.u[ 2 ] = z;
		r.u[ 3 ] = w;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Replicate a single floating-point value to all components of a vector.
	inline v128 AX_VCALL VecSet1( float x )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_set_ps1( x );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = x;
		r.f[ 1 ] = x;
		r.f[ 2 ] = x;
		r.f[ 3 ] = x;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Replicate a single integer value to all components of a vector.
	inline v128 AX_VCALL VecSetInt1( uint32 x )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_castsi128_ps( _mm_set1_epi32( x ) );
#elif AX_INTRIN == 0
		v128 r;

		r.u[ 0 ] = x;
		r.u[ 1 ] = x;
		r.u[ 2 ] = x;
		r.u[ 3 ] = x;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}

	/// Retrieve a vector where each bit of each component is set to 1.
	inline v128 AX_VCALL VecTrue()
	{
		return VecSetInt1( 0xFFFFFFFF );
	}
	/// Retrieve a vector where each bit of each component is set to 0.
	inline v128 AX_VCALL VecFalse()
	{
		return VecZero();
	}

	/// Add each component of vector \a a with each component from vector \a b.
	inline v128 AX_VCALL VecAdd( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_add_ps( a, b );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = a.f[ 0 ] + b.f[ 0 ];
		r.f[ 1 ] = a.f[ 1 ] + b.f[ 1 ];
		r.f[ 2 ] = a.f[ 2 ] + b.f[ 2 ];
		r.f[ 3 ] = a.f[ 3 ] + b.f[ 3 ];

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Subtract each component of vector \a a with each component from vector
	/// \a b.
	inline v128 AX_VCALL VecSub( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_sub_ps( a, b );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = a.f[ 0 ] - b.f[ 0 ];
		r.f[ 1 ] = a.f[ 1 ] - b.f[ 1 ];
		r.f[ 2 ] = a.f[ 2 ] - b.f[ 2 ];
		r.f[ 3 ] = a.f[ 3 ] - b.f[ 3 ];

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Multiply each component of vector \a a with each component from vector
	/// \a b.
	inline v128 AX_VCALL VecMul( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_mul_ps( a, b );
#elif AX_INTRIN == 0
		v128 r;
		
		r.f[ 0 ] = a.f[ 0 ]*b.f[ 0 ];
		r.f[ 1 ] = a.f[ 1 ]*b.f[ 1 ];
		r.f[ 2 ] = a.f[ 2 ]*b.f[ 2 ];
		r.f[ 3 ] = a.f[ 3 ]*b.f[ 3 ];

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Divide each component of vector \a a with each component from vector
	/// \a b.
	inline v128 AX_VCALL VecDiv( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_div_ps( a, b );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = a.f[ 0 ]/b.f[ 0 ];
		r.f[ 1 ] = a.f[ 1 ]/b.f[ 1 ];
		r.f[ 2 ] = a.f[ 2 ]/b.f[ 2 ];
		r.f[ 3 ] = a.f[ 3 ]/b.f[ 3 ];
		
		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Multiply each component of vector \a a with scalar \a b.
	inline v128 AX_VCALL VecScale( p_v128 a, float b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_mul_ps( a, _mm_set_ps1( b ) );
#elif AX_INTRIN == 0
		v128 r;
		
		r.f[ 0 ] = a.f[ 0 ]*b;
		r.f[ 1 ] = a.f[ 1 ]*b;
		r.f[ 2 ] = a.f[ 2 ]*b;
		r.f[ 3 ] = a.f[ 3 ]*b;
		
		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}


	/*
	===========================================================================
	
		VECTOR SWIZZLE
	
	===========================================================================
	*/

	enum
	{
		X = 0,
		Y,
		Z,
		W,

		R = 0,
		G,
		B,
		A,
		
		S = 0,
		T,
		P,
		Q
	};

	/// @brief Swizzle the components of a vector.
	///
	/// Creates a new vector whose components are taken from a set of components
	/// in the source vector.
	///
	/// Each template parameter represents a component of the source vector.
	/// Index 0 represents the X component, 1 the Y component, 2 is Z, and 3 is
	/// W.
	///
	/// @arg @c tX picks which component of the source vector will be
	///             replicated in the destination vector's X component.
	/// @arg @c tY picks which component of the source vector will be
	///             replicated in the destination vector's Y component.
	/// @arg @c tZ picks which component of the source vector will be
	///             replicated in the destination vector's Z component.
	/// @arg @c tW picks which component of the source vector will be
	///             replicated in the destination vector's W component.
	///
	/// @param a The source vector.
	/// @return The destination vector.
	template< uint32 tX, uint32 tY, uint32 tZ, uint32 tW >
	inline v128 AX_VCALL VecSwizzle( p_v128 a )
	{
		static_assert( tX < 4, "Invalid X component" );
		static_assert( tY < 4, "Invalid Y component" );
		static_assert( tZ < 4, "Invalid Z component" );
		static_assert( tW < 4, "Invalid W component" );

#if AX_INTRIN & AX_INTRIN_SSE
		return _mm_shuffle_ps( a, a, _MM_SHUFFLE( tW, tZ, tY, tX ) );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = a.f[ tX ];
		r.f[ 1 ] = a.f[ tY ];
		r.f[ 2 ] = a.f[ tZ ];
		r.f[ 3 ] = a.f[ tW ];

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	template<>
	inline v128 AX_VCALL VecSwizzle< X, Y, Z, W >( p_v128 a )
	{
		// special case optimization for returning the exact same value
		return a;
	}
	
	/*
	===========================================================================
	
		GEOMETRY FUNCTIONS
	
	===========================================================================
	*/

	/// Performs a dot product.
	inline float AX_VCALL VecDot( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		__m128 r, t;

		// t = xx, yy, zz, ww
		t = _mm_mul_ps( a, b );

		// r = yy, xx, ww, zz
		r = _mm_shuffle_ps( t, t, _MM_SHUFFLE( 1, 0, 3, 2 ) );

		// r = xx + zz, yy + ww, zz + xx, ww + yy (last two are redundant)
		r = _mm_add_ps( r, t );

		// t = yy + ww, xx + zz, xx + zz, xx + zz
		t = _mm_shuffle_ps( r, r, _MM_SHUFFLE( 0, 0, 0, 1 ) );

		// r = ( xx + zz ) + ( yy + ww )
		r = _mm_add_ss( r, t );

		// done
		float f;
		return _mm_store_ss( &f, r ), f;
#elif AX_INTRIN == 0
		return a.f[ 0 ]*b.f[ 0 ] + a.f[ 1 ]*b.f[ 1 ] + a.f[ 2 ]*b.f[ 2 ] + a.f[ 3 ]*b.f[ 3 ];
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	/// Performs a cross product.
	inline v128 AX_VCALL VecCross( p_v128 a, p_v128 b )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		__m128 r, s, t;

		// get the first columns (A=y,z,x; B=z,x,y)
		s = _mm_shuffle_ps( a, a, _MM_SHUFFLE( 0, 0, 2, 1 ) );
		t = _mm_shuffle_ps( b, b, _MM_SHUFFLE( 0, 1, 0, 2 ) );

		// multiply (Ay*Bz, Az*Bx, Ax*By)
		r = _mm_mul_ps( s, t );

		// get the second columns (A=z,x,y; B=y,z,x)
		s = _mm_shuffle_ps( a, a, _MM_SHUFFLE( 0, 1, 0, 2 ) );
		t = _mm_shuffle_ps( b, b, _MM_SHUFFLE( 0, 0, 2, 1 ) );

		// multiply (Az*By, Ax*Bz, Ay*Bx)
		s = _mm_mul_ps( s, t );

		// find the cross product (NOTE: 'w' will have meaningless data)
		return _mm_sub_ps( r, s );
#elif AX_INTRIN == 0
		v128 r;

		r.f[ 0 ] = a.f[ 1 ]*b.f[ 2 ] - a.f[ 2 ]*b.f[ 1 ];
		r.f[ 1 ] = a.f[ 2 ]*b.f[ 0 ] - a.f[ 0 ]*b.f[ 2 ];
		r.f[ 2 ] = a.f[ 0 ]*b.f[ 1 ] - a.f[ 1 ]*b.f[ 0 ];
		r.f[ 3 ] = 0.0f;

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	
	/// @brief Performs a 4D cross product.
	///
	/// This is useful for matrix inverses.
	inline v128 VecCross4( p_v128 a, p_v128 b, p_v128 c )
	{
#if AX_INTRIN & AX_INTRIN_SSE
		__m128 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;

		/*
			 (Ay*(Bz*Cw - Cz*Bw) - Az*(By*Cw - Cy*Bw) + Aw*(By*Cz - Cy*Bz))
			-(Ax*(Bz*Cw - Cz*Bw) - Az*(Bx*Cw - Cx*Bw) + Aw*(Bx*Cz - Cx*Bz))
			 (Ax*(By*Cw - Cy*Bw) - Ay*(Bx*Cw - Cx*Bw) + Aw*(Bx*Cy - Cx*By))
			-(Ax*(By*Cz - Cy*Bz) - Ay*(Bx*Cz - Cx*Bz) + Aw*(Bx*Cy - Cy*By))
		*/

		// B.zzyy*C.wwwz
		t0 = _mm_mul_ps( _mm_shuffle_ps( b, b, _MM_SHUFFLE( 1, 1, 2, 2 ) ),
						 _mm_shuffle_ps( c, c, _MM_SHUFFLE( 2, 3, 3, 3 ) ) );

		// C.zzyy*B.wwwz
		t1 = _mm_mul_ps( _mm_shuffle_ps( c, c, _MM_SHUFFLE( 1, 1, 2, 2 ) ),
						 _mm_shuffle_ps( b, b, _MM_SHUFFLE( 2, 3, 3, 3 ) ) );

		// B.yxxx*C.wwwz
		t2 = _mm_mul_ps( _mm_shuffle_ps( b, b, _MM_SHUFFLE( 0, 0, 0, 1 ) ),
						 _mm_shuffle_ps( c, c, _MM_SHUFFLE( 2, 3, 3, 3 ) ) );

		// C.yxxx*B.wwwz
		t3 = _mm_mul_ps( _mm_shuffle_ps( c, c, _MM_SHUFFLE( 0, 0, 0, 1 ) ),
						 _mm_shuffle_ps( b, b, _MM_SHUFFLE( 2, 3, 3, 3 ) ) );

		// B.yxxx*C.zzyy
		t4 = _mm_mul_ps( _mm_shuffle_ps( b, b, _MM_SHUFFLE( 0, 0, 0, 1 ) ),
						 _mm_shuffle_ps( c, c, _MM_SHUFFLE( 1, 1, 2, 2 ) ) );

		// C.yxxy*B.zzyy
		t5 = _mm_mul_ps( _mm_shuffle_ps( c, c, _MM_SHUFFLE( 1, 0, 0, 1 ) ),
						 _mm_shuffle_ps( b, b, _MM_SHUFFLE( 1, 1, 2, 2 ) ) );

		// ( B.zzyy*C.wwwz - C.zzyy*B.wwwz )*A.yxxx
		t6 = _mm_mul_ps( _mm_shuffle_ps( a, a, _MM_SHUFFLE( 0, 0, 0, 1 ) ),
						 _mm_sub_ps( t0, t1 ) );

		// ( B.yxxx*C.wwwz - C.yxxx*B.wwwz )*A.zzyy
		t7 = _mm_mul_ps( _mm_shuffle_ps( a, a, _MM_SHUFFLE( 1, 1, 2, 2 ) ),
						 _mm_sub_ps( t2, t3 ) );

		// ( B.yxxx*C.zzyy - C.yxxy*B.zzyy )*A.wwww
		t8 = _mm_mul_ps( _mm_shuffle_ps( a, a, _MM_SHUFFLE( 3, 3, 3, 3 ) ),
						 _mm_sub_ps( t4, t5 ) );

		// <first> - <second> + <third>
		t9 = _mm_add_ps( _mm_sub_ps( t6, t7 ), t8 );

		// finalize the result (prepare signs: +,-,+,-)
		return _mm_mul_ps( _mm_set_ps( -1, 1, -1, 1 ), t9 );
#elif AX_INTRIN == 0
		const float Ax = a.f[ 0 ], Ay = a.f[ 1 ], Az = a.f[ 2 ], Aw = a.f[ 3 ];
		const float Bx = b.f[ 0 ], By = b.f[ 1 ], Bz = b.f[ 2 ], Bw = b.f[ 3 ];
		const float Cx = c.f[ 0 ], Cy = c.f[ 1 ], Cz = c.f[ 2 ], Cw = c.f[ 3 ];

		v128 r;

		r.f[ 0 ] =  ( Ay*( Bz*Cw - Cz*Bw ) - Az*( By*Cw - Cy*Bw ) + Aw*( By*Cz - Cy*Bz ) );
		r.f[ 1 ] = -( Ax*( Bz*Cw - Cz*Bw ) - Az*( Bx*Cw - Cx*Bw ) + Aw*( Bx*Cz - Cx*Bz ) );
		r.f[ 2 ] =  ( Ax*( By*Cw - Cy*Bw ) - Ay*( Bx*Cw - Cx*Bw ) + Aw*( Bx*Cy - Cx*By ) );
		r.f[ 3 ] = -( Ax*( By*Cz - Cy*Bz ) - Ay*( Bx*Cz - Cx*Bz ) + Aw*( Bx*Cy - Cy*By ) );

		return r;
#else
# error AX_INTRIN: Unhandled intrinsic
#endif
	}
	
	/*
	===========================================================================
	
		VECTOR OPERATORS
	
	===========================================================================
	*/

#if !defined(__GNUC__) || defined(__clang__)

	inline v128 AX_VECTORCALL operator+( p_v128 a )
	{
		return a;
	}
	inline v128 AX_VECTORCALL operator-( p_v128 a )
	{
		return VecNegate( a );
	}

	inline v128 AX_VCALL operator+( p_v128 a, p_v128 b )
	{
		return VecAdd( a, b );
	}
	inline v128 AX_VCALL operator-( p_v128 a, p_v128 b )
	{
		return VecSub( a, b );
	}
	inline v128 AX_VCALL operator*( p_v128 a, p_v128 b )
	{
		return VecMul( a, b );
	}
	inline v128 AX_VCALL operator/( p_v128 a, p_v128 b )
	{
		return VecDiv( a, b );
	}

	inline v128 AX_VCALL operator*( p_v128 a, float b )
	{
		return VecScale( a, b );
	}
	inline v128 AX_VCALL operator*( float a, p_v128 b )
	{
		return VecScale( b, a );
	}

	inline v128 AX_VCALL operator/( p_v128 a, float b )
	{
		return a*( 1.0f/b );
	}
	
	inline v128 &AX_VCALL operator+=( v128 &a, p_v128 b )
	{
		a = a + b;
		return a;
	}
	inline v128 &AX_VCALL operator-=( v128 &a, p_v128 b )
	{
		a = a - b;
		return a;
	}
	inline v128 &AX_VCALL operator*=( v128 &a, p_v128 b )
	{
		a = a*b;
		return a;
	}
	inline v128 &AX_VCALL operator/=( v128 &a, p_v128 b )
	{
		a = a/b;
		return a;
	}

	inline v128 &AX_VCALL operator*=( v128 &a, float b )
	{
		a = a*b;
		return a;
	}
	inline v128 &AX_VCALL operator/=( v128 &a, float b )
	{
		a = a*( 1.0f/b );
		return a;
	}

#endif

}}

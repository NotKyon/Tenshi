#pragma once

#include "Types.hpp"
#include "../Platform/Platform.hpp"

namespace Ax
{

	/// Stores an integer for each usable byte-size.
	template< unsigned int tSize >
	struct TSizedInt
	{
#if AX_CXX_N1720
		static_assert( ( tSize & ( tSize - 1 ) ) == 0, "Need power of two" );
		static_assert( tSize >= 1, "Invalid size" );
#endif

		/// Signed-integer version.
		typedef void				sint;
		/// Unsigned-integer version.
		typedef void				uint;
	};
		
	/// Specialization of TSizedInt for 8-bit integers.
	template<>
	struct TSizedInt< 1 >
	{
		typedef int8				sint;
		typedef uint8				uint;
	};
	/// Specialization of TSizedInt for 16-bit integers.
	template<>
	struct TSizedInt< 2 >
	{
		typedef int16				sint;
		typedef uint16				uint;
	};
	/// Specialization of TSizedInt for 32-bit integers.
	template<>
	struct TSizedInt< 4 >
	{
		typedef int32				sint;
		typedef uint32				uint;
	};
	/// Specialization of TSizedInt for 64-bit integers.
	template<>
	struct TSizedInt< 8 >
	{
		typedef int64				sint;
		typedef uint64				uint;
	};

	/// Declare an integral constant
	template< typename tElement, tElement tValue >
	struct TIntConst
	{
#if AX_CXX_CONSTEXPR_ENABLED
		static constexpr tElement	value = tValue;
#else
		static const tElement		value = tValue;
#endif

		typedef tElement			value_type;
		typedef TIntConst< tElement, tValue > type;
#if AX_CXX_CONSTEXPR_ENABLED
		constexpr operator value_type() { return value; }
#endif
	};

	/// Classification of TIntConst for TTrue
	typedef TIntConst< bool, true > TTrue;
	/// Classification of TIntConst for TFalse
	typedef TIntConst< bool, false > TFalse;

	/// Determine whether a given type is an integer
	template< typename tElement > struct TIsInt: public TFalse {};
	/// Specialization of TIsInt for uint8
	template<> struct TIsInt< uint8 >: public TTrue {};
	/// Specialization of TIsInt for uint16
	template<> struct TIsInt< uint16 >: public TTrue {};
	/// Specialization of TIsInt for uint32
	template<> struct TIsInt< uint32 >: public TTrue {};
	/// Specialization of TIsInt for uint64
	template<> struct TIsInt< uint64 >: public TTrue {};
	/// Specialization of TIsInt for int8
	template<> struct TIsInt< int8 >: public TTrue {};
	/// Specialization of TIsInt for int16
	template<> struct TIsInt< int16 >: public TTrue {};
	/// Specialization of TIsInt for int32
	template<> struct TIsInt< int32 >: public TTrue {};
	/// Specialization of TIsInt for int64
	template<> struct TIsInt< int64 >: public TTrue {};
	/// Specialization of TIsInt for signed long
	template<> struct TIsInt< signed long >: public TTrue {};
	/// Specialization of TIsInt for unsigned long
	template<> struct TIsInt< unsigned long >: public TTrue {};

	/// Determine whether a type is signed
	template< typename tInt >
	struct TIsSigned: public TIntConst< bool, tInt( -1 ) < tInt( 0 ) > {};
	/// Determine whether a type is unsigned
	template< typename tInt >
	struct TIsUnsigned: public TIntConst< bool, tInt( 0 ) < tInt( -1 ) > {};

	/// Retrieve the limits of a given integer type
	template< typename tInt >
	struct TIntLimits
	{
#if AX_CXX_N1720
		static_assert( TIsInt< tInt >::value == true, "Non-integer type passed to TIntLimits<>" );
#endif

		static const uint32			kNumBits			= sizeof( tInt )*8;
		static const uint32			kLastBit			= sizeof( tInt )*8 - 1;
		static const tInt			kMax				= ( TIsUnsigned< tInt >::value ) ? ~tInt( 0 ) : tInt( ( 1ULL<<( sizeof( tInt )*8 - 1 ) ) - 1 );
		static const tInt			kMin				= ( TIsUnsigned< tInt >::value ) ?        0   : tInt(   1ULL<<( sizeof( tInt )*8 - 1 ) );
	};

	static const int8				kInt8Min			= TIntLimits< int8 >::kMin;
	static const int8				kInt8Max			= TIntLimits< int8 >::kMax;
	static const int16				kInt16Min			= TIntLimits< int16 >::kMin;
	static const int16				kInt16Max			= TIntLimits< int16 >::kMax;
	static const int32				kInt32Min			= TIntLimits< int32 >::kMin;
	static const int32				kInt32Max			= TIntLimits< int32 >::kMax;
	static const int64				kInt64Min			= TIntLimits< int64 >::kMin;
	static const int64				kInt64Max			= TIntLimits< int64 >::kMax;
	static const intptr				kIntPtrMin			= TIntLimits< intptr >::kMin;
	static const intptr				kIntPtrMax			= TIntLimits< intptr >::kMax;
	static const intcpu				kIntCpuMin			= TIntLimits< intcpu >::kMin;
	static const intcpu				kIntCpuMax			= TIntLimits< intcpu >::kMax;

	static const uint8				kUint8Min			= TIntLimits< uint8 >::kMin;
	static const uint8				kUint8Max			= TIntLimits< uint8 >::kMax;
	static const uint16				kUint16Min			= TIntLimits< uint16 >::kMin;
	static const uint16				kUint16Max			= TIntLimits< uint16 >::kMax;
	static const uint32				kUint32Min			= TIntLimits< uint32 >::kMin;
	static const uint32				kUint32Max			= TIntLimits< uint32 >::kMax;
	static const uint64				kUint64Min			= TIntLimits< uint64 >::kMin;
	static const uint64				kUint64Max			= TIntLimits< uint64 >::kMax;
	static const uintptr			kUintPtrMin			= TIntLimits< uintptr >::kMin;
	static const uintptr			kUintPtrMax			= TIntLimits< uintptr >::kMax;
	static const uintcpu			kUintCpuMin			= TIntLimits< uintcpu >::kMin;
	static const uintcpu			kUintCpuMax			= TIntLimits< uintcpu >::kMax;

#if AX_CXX_N2118
	/// Determine whether a type is an l-value reference
	template< typename tElement > struct TIsLValueRef: public TFalse {};
	/// Specialization of is_lvalue_reference for l-value references
	template< typename tElement > struct TIsLValueRef< tElement & >: public TTrue {};

	/// Determine whether a type is an r-value reference
	template< typename tElement > struct TIsRValueRef: public TFalse {};
	/// Specialization of is_rvalue_reference for r-value references
	template< typename tElement > struct TIsRValueRef< tElement && >: public TTrue {};
#endif

#ifndef AX_HAS_TRIVIAL_DESTRUCTOR
# if AX_CXX_TYPE_TRAITS_ENABLED
#  define AX_HAS_TRIVIAL_DESTRUCTOR( tElement ) __has_trivial_destructor( tElement )
# else
#  define AX_HAS_TRIVIAL_DESTRUCTOR( tElement ) false
# endif
#endif

	/// Determine whether a type has a trivial-destructor
	template< typename tObject >
	struct THasTrivialDestructor: public TIntConst< bool, AX_HAS_TRIVIAL_DESTRUCTOR( tObject ) > {};

#define AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( tElement )\
	template<> struct Ax::THasTrivialDestructor< tElement >: public TTrue {}

	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( int8 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( int16 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( int32 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( int64 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( uint8 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( uint16 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( uint32 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( uint64 );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( float );
	AX_DECLARE_HAS_TRIVIAL_DESTRUCTOR( double );

	/// \brief Removes the const-qualifier from a type if it has one.
	///
	/// TRemoveConst< const int >::type would be 'int' without the 'const'. This
	/// is useful for template programming.
	template< typename tElement >
	struct TRemoveConst
	{
		typedef tElement type;
	};
	/// Specialization of TRemoveConst for <tt>const tElement</tt>.
	template< typename tElement >
	struct TRemoveConst< const tElement >
	{
		typedef tElement type;
	};

	/// \brief Removes the volatile-qualifier from a type if it has one.
	///
	/// TRemoveVolatile< volatile int >::type would be 'int' without the
	/// 'volatile'. This is useful for template programming.
	template< typename tElement >
	struct TRemoveVolatile { typedef tElement type; };
	/// Specialization of TRemoveVolatile for <tt>volatile tElement</tt>.
	template< typename tElement >
	struct TRemoveVolatile< volatile tElement > { typedef tElement type; };
	/// Remove the const and volatile qualifiers from a type if it has any.
	template< typename tElement > struct TRemoveCV { typedef tElement type; };
	/// Specialization of TRemoveCV for const types
	template< typename tElement > struct TRemoveCV< const tElement > { typedef tElement type; };
	/// Specialization of TRemoveCV for volatile types
	template< typename tElement > struct TRemoveCV< volatile tElement > { typedef tElement type; };
	/// Specialization of TRemoveCV for const and volatile types
	template< typename tElement >
	struct TRemoveCV< const volatile tElement > { typedef tElement type; };
	/// Remove a reference from a type
	template< typename tElement > struct TRemoveRef { typedef tElement type; };
	/// Specialization of TRemoveRef for l-value references
	template< typename tElement > struct TRemoveRef< tElement & > { typedef tElement type; };
	/// Specialization of TRemoveRef for r-value references
	template< typename tElement > struct TRemoveRef< tElement && > { typedef tElement type; };
	/// Remove a pointer from a type
	template< typename tElement > struct TRemovePtr { typedef tElement type; };
	/// Specialization of TRemovePtr for pointers
	template< typename tElement > struct TRemovePtr< tElement * > { typedef tElement type; };

	/// Adds a const-qualifier to a type if it doesn't already have one.
	template< typename tElement >
	struct TAddConst
	{
		typedef const typename TRemoveConst< tElement >::type type;
	};
	/// Adds a volatile-qualifier to a type if it doesn't already have one.
	template< typename tElement >
	struct TAddVolatile
	{
		typedef volatile typename TRemoveVolatile< tElement >::type type;
	};
	/// Adds a const and volatile qualifier to a type if it doesn't have one
	template< typename tElement >
	struct TAddCV
	{
		typedef const volatile typename TRemoveCV< tElement >::type type;
	};
	/// Add a reference to a type if it doesn't already have one
	template< typename tElement >
	struct TAddRef
	{
		typedef typename TRemoveRef< tElement >::type &type;
	};
	/// Specialization of TAddRef for void
	template<> struct TAddRef< void > { typedef void type; };
	/// Specialization of TAddRef for const-qualified void
	template<> struct TAddRef< const void > { typedef const void type; };
	/// Specialization of TAddRef for volatile-qualified void
	template<>
	struct TAddRef< volatile void >
	{
		typedef volatile void type;
	};
	/// Specialization of TAddRef for const-volatile-qualified void
	template<>
	struct TAddRef< const volatile void >
	{
		typedef const volatile void type;
	};
	/// Add a l-value reference to a type if it doesn't already have one
	template< typename tElement >
	struct TAddLValueRef: public TAddRef< tElement > {};

#if AX_CXX_N2118
	/// Add a r-value reference to a type if it doesn't already have one
	template< typename tElement >
	struct TAddRValueRef
	{
		typedef typename TRemoveRef< tElement >::type &&type;
	};
	/// Specialization of TAddRValueRef for void
	template<>
	struct TAddRValueRef< void > { typedef void type; };
	/// Specialization of TAddRValueRef for const-void
	template<>
	struct TAddRValueRef< const void > { typedef const void type; };
	/// Specialization of TAddRValueRef for volatile-void
	template<>
	struct TAddRValueRef< volatile void >
	{
		typedef volatile void type;
	};
	/// Specialization of TAddRValueRef for const-volatile-void
	template<>
	struct TAddRValueRef< const volatile void >
	{
		typedef const volatile void type;
	};
#endif

	/// Add a pointer to a type if it doesn't already have one
	template< typename tElement >
	struct TAddPtr
	{
		typedef typename TRemovePtr< tElement >::type *type;
	};

	/// Copy the const and volatile qualifiers of a source type to a destination
	/// type
	template< typename tDst, typename tSrc >
	struct TCopyCV
	{
		typedef tDst type;
	};
	/// Specialization of TCopyCV for const source
	template< typename tDst, typename tSrc >
	struct TCopyCV< tDst, const tSrc >
	{
		typedef typename TAddConst< tDst >::type type;
	};
	/// Specialization of TCopyCV for volatile source
	template< typename tDst, typename tSrc >
	struct TCopyCV< tDst, volatile tSrc >
	{
		typedef typename TAddVolatile< tDst >::type type;
	};
	/// Specialization of TCopyCV for const and volatile source
	template< typename tDst, typename tSrc >
	struct TCopyCV< tDst, const volatile tSrc >
	{
		typedef typename TAddCV< tDst >::type type;
	};

#if AX_CXX_N2118
	/// Get an r-value reference
	template< class tElement >
	inline tElement &&Forward( typename TRemoveRef< tElement >::type &x ) AX_NOTHROW
	{
		return static_cast< tElement && >( x );
	}
	/// Get an r-value reference (from another r-value reference)
	template< class tElement >
	inline tElement &&Forward( typename TRemoveRef< tElement >::type &&x ) AX_NOTHROW
	{
		static_assert( !TIsLValueRef< tElement >::value, "Unable to forward" );
		return static_cast< tElement && >( x );
	}

	/// Helper function to bind to move functions rather than copy functions
	template< class tElement >
	inline typename TRemoveRef< tElement >::type &&Move( tElement &&x ) AX_NOTHROW
	{
		return static_cast< typename TRemoveRef< tElement >::type && >( x );
	}
#endif

	/// Mark a type as signed
	template< typename tInt >
	struct TMakeSigned
	{
		static_assert( TIsInt< tInt >::value, "Integral type required" );

		typedef
			typename TCopyCV<
				typename TSizedInt< sizeof( tInt ) >::sint,
				tInt
			>::type type;
	};
	/// Mark a type as unsigned
	template< typename tInt >
	struct TMakeUnsigned
	{
		static_assert( TIsInt< tInt >::value, "Integral type required" );

		typedef
			typename TCopyCV<
				typename TSizedInt< sizeof( tInt ) >::uint,
				tInt
			>::type type;
	};

	/// Determine whether an integer can be down-casted without a loss in
	/// precision.
	template< typename tSrcInt, typename tDstInt >
	inline bool CanDowncast( const tSrcInt &src ) AX_NOTHROW
	{
		typedef typename TSizedInt< sizeof( tSrcInt ) >::uint src_uint;
		typedef typename TSizedInt< sizeof( tDstInt ) >::uint dst_uint;

		if( sizeof( tDstInt ) >= sizeof( tSrcInt ) ) {
			return true;
		}

		if( src_uint( src ) <= src_uint( dst_uint( -1 ) ) ) {
			return true;
		}

		return false;
	}
	/// Perform a Downcast from one type to another, warning on precision loss
	template< typename tDstInt, typename tSrcInt >
	inline tDstInt Downcast( const tSrcInt &src )
	{
		AX_ASSERT_MSG( ( CanDowncast< tSrcInt, tDstInt >( src ) ), "Cannot Downcast without a loss in precision" );
		return static_cast< tDstInt >( src );
	}
	/// Cast from a virtual base class to a derived class
	template< typename tDst, typename tSrc >
	inline tDst *VirtualCast( tSrc *x )
	{
#if AX_CXX_N1836 && AX_CXX_N1720
		// We are making an important assumption here...
		static_assert( __is_base_of( tSrc, tDst ), "tSrc must be the base type of tDst" );
#endif

		// Need to know how big tDst is on its own (if it didn't inherit from tSrc)
		static const size_t diffsize = sizeof( tDst ) - sizeof( tSrc );

		// The dst address is the src address minus the dst's relative size
		const size_t srcaddr = size_t( x );
		const size_t dstaddr = srcaddr - diffsize;

		// We now have the casted address
		return reinterpret_cast< tDst * >( dstaddr );
	}

	/// Retrieve the size of an array
	template< typename tElement, size_t tSize >
	inline size_t ArraySize( const tElement( & )[ tSize ] )
	{
		return tSize;
	}
	/// Retrieve the total size of a two-dimensional array
	template< typename tElement, size_t tSize1, size_t tSize2 >
	inline size_t ArraySize( const tElement( & )[ tSize1 ][ tSize2 ] )
	{
		return tSize1*tSize2;
	}
	/// Retrieve the total size of a three-dimensional array
	template< typename tElement, size_t tSize1, size_t tSize2, size_t tSize3 >
	inline size_t ArraySize( const tElement( & )[ tSize1 ][ tSize2 ][ tSize3 ] )
	{
		return tSize1*tSize2*tSize3;
	}
	/// Retrieve the total size of a four-dimensional array
	template< typename tElement, size_t tSize1, size_t tSize2, size_t tSize3, size_t tSize4 >
	inline size_t ArraySize( const tElement( & )[ tSize1 ][ tSize2 ][ tSize3 ][ tSize4 ] )
	{
		return tSize1*tSize2*tSize3*tSize4;
	}

	/// Evaluate an expression and select a type depending on the result
	template< bool tExpression, typename tTrue, typename tFalse >
	struct TConditional
	{
		typedef tFalse type;
	};
	/// Specialization of TConditional<> for a true expression
	template< typename tTrue, typename tFalse >
	struct TConditional< true, tTrue, tFalse >
	{
		typedef tTrue type;
	};

	/// \brief Select an index type based on a buffer size
	template< uint64 tBufferSize >
	struct TSelectIndex
	{
		typedef
			typename TConditional
			<
				( tBufferSize > 1ULL<<32 ),
				uint64,
				typename TConditional
				<
					( tBufferSize > 1ULL<<16 ),
					uint32,
					typename TConditional
					<
						( tBufferSize > 1ULL<<8 ),
						uint16,
						uint8
					>::type
				>::type
			>::type
			type;
	};

#define AX_TEST_IDX__( Size, Type )\
	( sizeof( TSelectIndex< Size >::type ) == sizeof( Type ) )

#if AX_CXX_N1720
	static_assert( AX_TEST_IDX__( 0x100UL, uint8 ), "Bad selection (uint8)" );
	static_assert( AX_TEST_IDX__( 0x101UL, uint16 ), "Bad selection (uint16,1)" );
	static_assert( AX_TEST_IDX__( 0x10000UL, uint16 ), "Bad selection (uint16)" );
	static_assert( AX_TEST_IDX__( 0x10001UL, uint32 ), "Bad selection (uint32)" );
#endif

}

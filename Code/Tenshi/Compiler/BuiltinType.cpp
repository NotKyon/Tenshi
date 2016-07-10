#include "_PCH.hpp"
#include "BuiltinType.hpp"
#include "Environment.hpp"

namespace Tenshi { namespace Compiler {

	static unsigned AsInt( EBuiltinType T ) { return ( unsigned )T; }

	// Retrieve the built-in type information for a given type
	const SBuiltinTypeInfo &GetBuiltinTypeInfo( EBuiltinType T )
	{
		static SBuiltinTypeInfo		Info				[ 64 ];
		static bool					bDidInit			= false;

		if( !bDidInit ) {
			memset( &Info[0], 0, sizeof( Info ) );
			bDidInit = true;

			SBuiltinTypeInfo *p;

			// Invalid type
			p = &Info[ AsInt( EBuiltinType::Invalid ) ];
			p->bNoPromote = true;

			// UDT
			p = &Info[ AsInt( EBuiltinType::UserDefined ) ];
			p->bNoPromote = true;

			// Nothing
			p = &Info[ AsInt( EBuiltinType::Void ) ];
			p->bNoPromote = true;

			// Anything
			p = &Info[ AsInt( EBuiltinType::Any ) ];

			// Boolean value
			p = &Info[ AsInt( EBuiltinType::Boolean ) ];
			p->bIsUnsigned = true;
			p->cBits = 1;
			
			// 8-bit signed integer
			p = &Info[ AsInt( EBuiltinType::Int8 ) ];
			p->bIsSigned = true;
			p->cBits = 8;

			// 16-bit signed integer
			p = &Info[ AsInt( EBuiltinType::Int16 ) ];
			p->bIsSigned = true;
			p->cBits = 16;

			// 32-bit signed integer
			p = &Info[ AsInt( EBuiltinType::Int32 ) ];
			p->bIsSigned = true;
			p->cBits = 32;

			// 64-bit signed integer
			p = &Info[ AsInt( EBuiltinType::Int64 ) ];
			p->bIsSigned = true;
			p->cBits = 64;

			// 128-bit signed integer
			p = &Info[ AsInt( EBuiltinType::Int128 ) ];
			p->bIsSigned = true;
			p->cBits = 128;

			// 8-bit unsigned integer
			p = &Info[ AsInt( EBuiltinType::UInt8 ) ];
			p->bIsUnsigned = true;
			p->cBits = 8;

			// 16-bit unsigned integer
			p = &Info[ AsInt( EBuiltinType::UInt16 ) ];
			p->bIsUnsigned = true;
			p->cBits = 16;

			// 32-bit unsigned integer
			p = &Info[ AsInt( EBuiltinType::UInt32 ) ];
			p->bIsUnsigned = true;
			p->cBits = 32;

			// 64-bit unsigned integer
			p = &Info[ AsInt( EBuiltinType::UInt64 ) ];
			p->bIsUnsigned = true;
			p->cBits = 64;

			// 128-bit unsigned integer
			p = &Info[ AsInt( EBuiltinType::UInt128 ) ];
			p->bIsUnsigned = true;
			p->cBits = 128;

			// 16-bit float
			p = &Info[ AsInt( EBuiltinType::Float16 ) ];
			p->bIsFloat = true;
			p->cBits = 16;

			// 32-bit float
			p = &Info[ AsInt( EBuiltinType::Float32 ) ];
			p->bIsFloat = true;
			p->cBits = 32;

			// 64-bit float
			p = &Info[ AsInt( EBuiltinType::Float64 ) ];
			p->bIsFloat = true;
			p->cBits = 64;

			// Unsigned-normalized 8-bit float
			p = &Info[ AsInt( EBuiltinType::UNorm8 ) ];
			p->bIsUnsigned = true;
			p->bIsNormalized = true;
			p->cBits = 8;

			// Unsigned-normalized 16-bit float
			p = &Info[ AsInt( EBuiltinType::UNorm16 ) ];
			p->bIsUnsigned = true;
			p->bIsNormalized = true;
			p->cBits = 16;

			// Unsigned-normalized 32-bit float
			p = &Info[ AsInt( EBuiltinType::UNorm32 ) ];
			p->bIsUnsigned = true;
			p->bIsNormalized = true;
			p->cBits = 32;

			// Unsigned-normalized 64-bit float
			p = &Info[ AsInt( EBuiltinType::UNorm64 ) ];
			p->bIsUnsigned = true;
			p->bIsNormalized = true;
			p->cBits = 64;

			// Signed-normalized 8-bit float
			p = &Info[ AsInt( EBuiltinType::SNorm8 ) ];
			p->bIsSigned = true;
			p->bIsNormalized = true;
			p->cBits = 8;

			// Signed-normalized 16-bit float
			p = &Info[ AsInt( EBuiltinType::SNorm16 ) ];
			p->bIsSigned = true;
			p->bIsNormalized = true;
			p->cBits = 16;

			// Signed-normalized 32-bit float
			p = &Info[ AsInt( EBuiltinType::SNorm32 ) ];
			p->bIsSigned = true;
			p->bIsNormalized = true;
			p->cBits = 32;

			// Signed-normalized 64-bit float
			p = &Info[ AsInt( EBuiltinType::SNorm64 ) ];
			p->bIsSigned = true;
			p->bIsNormalized = true;
			p->cBits = 64;

			// 128-bit SIMD vector (no specific format)
			p = &Info[ AsInt( EBuiltinType::SIMDVector128 ) ];
			p->bIsSIMD = true;
			p->cBits = 128;

			// 256-bit SIMD vector
			p = &Info[ AsInt( EBuiltinType::SIMDVector256 ) ];
			p->bIsSIMD = true;
			p->cBits = 256;

			// 128-bit SIMD vector of 4 Float32 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector128_4Float32 ) ];
			p->bIsFloat = true;
			p->bIsSIMD = true;
			p->cBits = 128;
			p->cColumns = 1;
			p->cRows = 4;

			// 128-bit SIMD vector of 2 Float64 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector128_2Float64 ) ];
			p->bIsFloat = true;
			p->bIsSIMD = true;
			p->cBits = 128;
			p->cColumns = 1;
			p->cRows = 2;

			// 128-bit SIMD vector of 4 Int32 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector128_4Int32 ) ];
			p->bIsSigned = true;
			p->bIsSIMD = true;
			p->cBits = 128;
			p->cColumns = 1;
			p->cRows = 4;

			// 128-bit SIMD vector of 2 Int64 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector128_2Int64 ) ];
			p->bIsSigned = true;
			p->bIsSIMD = true;
			p->cBits = 128;
			p->cColumns = 1;
			p->cRows = 2;

			// 256-bit SIMD vector of 8 Float32 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector256_8Float32 ) ];
			p->bIsFloat = true;
			p->bIsSIMD = true;
			p->cBits = 256;
			p->cColumns = 1;
			p->cRows = 8;

			// 256-bit SIMD vector of 4 Float64 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector256_4Float64 ) ];
			p->bIsFloat = true;
			p->bIsSIMD = true;
			p->cBits = 256;
			p->cColumns = 1;
			p->cRows = 4;

			// 256-bit SIMD vector of 8 Int32 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector256_8Int32 ) ];
			p->bIsSigned = true;
			p->bIsSIMD = true;
			p->cBits = 256;
			p->cColumns = 1;
			p->cRows = 8;

			// 256-bit SIMD vector of 4 Int64 items
			p = &Info[ AsInt( EBuiltinType::SIMDVector256_4Int64 ) ];
			p->bIsSigned = true;
			p->bIsSIMD = true;
			p->cBits = 256;
			p->cColumns = 1;
			p->cRows = 4;

			// C string (const char *)
			p = &Info[ AsInt( EBuiltinType::ConstUTF8Pointer ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;

			// C string (const wchar_t *)
			p = &Info[ AsInt( EBuiltinType::ConstUTF16Pointer ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;

			// String object
			p = &Info[ AsInt( EBuiltinType::StringObject ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;

			// Array object
			p = &Info[ AsInt( EBuiltinType::ArrayObject ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;
			p->bNoPromote = true;

			// Linked list object
			p = &Info[ AsInt( EBuiltinType::LinkedListObject ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;
			p->bNoPromote = true;

			// Binary tree object
			p = &Info[ AsInt( EBuiltinType::BinaryTreeObject ) ];
			p->bIsUnsigned = true;
			p->bIsPointer = true;
			p->bNoPromote = true;
		
			// 2 x Float32 as a vector
			p = &Info[ AsInt( EBuiltinType::Vector2f ) ];
			p->bIsFloat = true;
			p->cBits = 2*32;
			p->cColumns = 2;
			p->cRows = 1;

			// 3 x Float32 as a vector
			p = &Info[ AsInt( EBuiltinType::Vector3f ) ];
			p->bIsFloat = true;
			p->cBits = 3*32;
			p->cColumns = 3;
			p->cRows = 1;

			// 4 x Float32 as a vector
			p = &Info[ AsInt( EBuiltinType::Vector4f ) ];
			p->bIsFloat = true;
			p->cBits = 4*32;
			p->cColumns = 4;
			p->cRows = 1;

			// 2x2 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix2f ) ];
			p->bIsFloat = true;
			p->cBits = 2*2*32;
			p->cColumns = 2;
			p->cRows = 2;

			// 3x3 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix3f ) ];
			p->bIsFloat = true;
			p->cBits = 3*3*32;
			p->cColumns = 3;
			p->cRows = 3;

			// 4x4 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix4f ) ];
			p->bIsFloat = true;
			p->cBits = 4*4*32;
			p->cColumns = 4;
			p->cRows = 4;

			// 2x3 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix23f ) ];
			p->bIsFloat = true;
			p->cBits = 2*3*32;
			p->cColumns = 2;
			p->cRows = 3;

			// 2x4 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix24f ) ];
			p->bIsFloat = true;
			p->cBits = 2*4*32;
			p->cColumns = 2;
			p->cRows = 4;

			// 3x2 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix32f ) ];
			p->bIsFloat = true;
			p->cBits = 3*2*32;
			p->cColumns = 3;
			p->cRows = 2;

			// 3x4 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix34f ) ];
			p->bIsFloat = true;
			p->cBits = 3*4*32;
			p->cColumns = 3;
			p->cRows = 4;

			// 4x2 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix42f ) ];
			p->bIsFloat = true;
			p->cBits = 4*2*32;
			p->cColumns = 4;
			p->cRows = 2;

			// 4x3 x Float32 as a matrix
			p = &Info[ AsInt( EBuiltinType::Matrix43f ) ];
			p->bIsFloat = true;
			p->cBits = 4*3*32;
			p->cColumns = 4;
			p->cRows = 3;
		
			// 4 x Float32 as a quaternion
			p = &Info[ AsInt( EBuiltinType::Quaternionf ) ];
			p->bIsFloat = true;
			p->cBits = 4*32;
			p->cColumns = 4;
			p->cRows = 1;
		}

		AX_ASSERT( AsInt( T ) < (unsigned)(sizeof(Info)/sizeof(Info[0])) );
		return Info[ AsInt( T ) ];
	}

	const char *BuiltinTypeToString( EBuiltinType Type )
	{
		switch( Type )
		{
		case EBuiltinType::Invalid:						return "Invalid";

		case EBuiltinType::UserDefined:					return "UserDefined";
		case EBuiltinType::Void:						return "Void";
		case EBuiltinType::Any:							return "Any";
		case EBuiltinType::Boolean:						return "Boolean";
		case EBuiltinType::Int8:						return "Int8";
		case EBuiltinType::Int16:						return "Int16";
		case EBuiltinType::Int32:						return "Int32";
		case EBuiltinType::Int64:						return "Int64";
		case EBuiltinType::Int128:						return "Int128";
		case EBuiltinType::UInt8:						return "UInt8";
		case EBuiltinType::UInt16:						return "UInt16";
		case EBuiltinType::UInt32:						return "UInt32";
		case EBuiltinType::UInt64:						return "UInt64";
		case EBuiltinType::UInt128:						return "UInt128";
		case EBuiltinType::Float16:						return "Float16";
		case EBuiltinType::Float32:						return "Float32";
		case EBuiltinType::Float64:						return "Float64";
		case EBuiltinType::UNorm8:						return "UNorm8";
		case EBuiltinType::UNorm16:						return "UNorm16";
		case EBuiltinType::UNorm32:						return "UNorm32";
		case EBuiltinType::UNorm64:						return "UNorm64";
		case EBuiltinType::SNorm8:						return "SNorm8";
		case EBuiltinType::SNorm16:						return "SNorm16";
		case EBuiltinType::SNorm32:						return "SNorm32";
		case EBuiltinType::SNorm64:						return "SNorm64";
		case EBuiltinType::SIMDVector128:				return "SIMDVector128";
		case EBuiltinType::SIMDVector256:				return "SIMDVector256";
		case EBuiltinType::SIMDVector128_4Float32:		return "SIMDVector128_4Float32";
		case EBuiltinType::SIMDVector128_2Float64:		return "SIMDVector128_2Float64";
		case EBuiltinType::SIMDVector128_4Int32:		return "SIMDVector128_4Int32";
		case EBuiltinType::SIMDVector128_2Int64:		return "SIMDVector128_2Int64";
		case EBuiltinType::SIMDVector256_8Float32:		return "SIMDVector256_8Float32";
		case EBuiltinType::SIMDVector256_4Float64:		return "SIMDVector256_4Float64";
		case EBuiltinType::SIMDVector256_8Int32:		return "SIMDVector256_8Int32";
		case EBuiltinType::SIMDVector256_4Int64:		return "SIMDVector256_4Int64";
		case EBuiltinType::ConstUTF8Pointer:			return "ConstUTF8Pointer";
		case EBuiltinType::ConstUTF16Pointer:			return "ConstUTF16Pointer";
		case EBuiltinType::StringObject:				return "StringObject";
		case EBuiltinType::ArrayObject:					return "ArrayObject";
		case EBuiltinType::LinkedListObject:			return "LinkedListObject";
		case EBuiltinType::BinaryTreeObject:			return "BinaryTreeObject";
		
		case EBuiltinType::Vector2f:					return "Vector2f";
		case EBuiltinType::Vector3f:					return "Vector3f";
		case EBuiltinType::Vector4f:					return "Vector4f";

		case EBuiltinType::Matrix2f:					return "Matrix2f";
		case EBuiltinType::Matrix3f:					return "Matrix3f";
		case EBuiltinType::Matrix4f:					return "Matrix4f";
		case EBuiltinType::Matrix23f:					return "Matrix23f";
		case EBuiltinType::Matrix24f:					return "Matrix24f";
		case EBuiltinType::Matrix32f:					return "Matrix32f";
		case EBuiltinType::Matrix34f:					return "Matrix34f";
		case EBuiltinType::Matrix42f:					return "Matrix42f";
		case EBuiltinType::Matrix43f:					return "Matrix43f";
		
		case EBuiltinType::Quaternionf:					return "Quaternionf";
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return "(unknown)";
	}

	bool IsTrivial( EBuiltinType Type )
	{
		switch( Type )
		{
		case EBuiltinType::Any:
		case EBuiltinType::UserDefined:
		case EBuiltinType::Void:
			AX_ASSERT_MSG( false, "Invalid built-in type" );
			return true;

		case EBuiltinType::ArrayObject:
		case EBuiltinType::BinaryTreeObject:
		case EBuiltinType::LinkedListObject:
		case EBuiltinType::StringObject:
			return false;

		case EBuiltinType::Boolean:
		case EBuiltinType::ConstUTF8Pointer:
		case EBuiltinType::ConstUTF16Pointer:
		case EBuiltinType::Float16:
		case EBuiltinType::Float32:
		case EBuiltinType::Float64:
		case EBuiltinType::Int8:
		case EBuiltinType::Int16:
		case EBuiltinType::Int32:
		case EBuiltinType::Int64:
		case EBuiltinType::Int128:
		case EBuiltinType::SIMDVector128:
		case EBuiltinType::SIMDVector128_4Float32:
		case EBuiltinType::SIMDVector128_4Int32:
		case EBuiltinType::SIMDVector128_2Float64:
		case EBuiltinType::SIMDVector128_2Int64:
		case EBuiltinType::SIMDVector256:
		case EBuiltinType::SIMDVector256_8Float32:
		case EBuiltinType::SIMDVector256_8Int32:
		case EBuiltinType::SIMDVector256_4Float64:
		case EBuiltinType::SIMDVector256_4Int64:
		case EBuiltinType::SNorm8:
		case EBuiltinType::SNorm16:
		case EBuiltinType::SNorm32:
		case EBuiltinType::SNorm64:
		case EBuiltinType::UInt8:
		case EBuiltinType::UInt16:
		case EBuiltinType::UInt32:
		case EBuiltinType::UInt64:
		case EBuiltinType::UInt128:
		case EBuiltinType::UNorm8:
		case EBuiltinType::UNorm16:
		case EBuiltinType::UNorm32:
		case EBuiltinType::UNorm64:
			return true;

		case EBuiltinType::Vector2f:
		case EBuiltinType::Vector3f:
		case EBuiltinType::Vector4f:
		case EBuiltinType::Matrix2f:
		case EBuiltinType::Matrix3f:
		case EBuiltinType::Matrix4f:
		case EBuiltinType::Matrix23f:
		case EBuiltinType::Matrix24f:
		case EBuiltinType::Matrix32f:
		case EBuiltinType::Matrix34f:
		case EBuiltinType::Matrix42f:
		case EBuiltinType::Matrix43f:
		case EBuiltinType::Quaternionf:
			return true;
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return 0;
	}
	// Check whether a type is a plain-old-data type
	bool IsPOD( EBuiltinType T )
	{
		const SBuiltinTypeInfo &Info = GetBuiltinTypeInfo( T );
		return
			Info.bIsFloat || Info.bIsNormalized || Info.bIsSigned ||
			Info.bIsUnsigned || Info.bIsPointer || Info.bIsSIMD;
	}
	// Check whether a type is a string (of some form)
	bool IsString( EBuiltinType T )
	{
		switch( T )
		{
		case EBuiltinType::StringObject:
		case EBuiltinType::ConstUTF8Pointer:
		case EBuiltinType::ConstUTF16Pointer:
			return true;

		default:
			break;
		}

		return false;
	}
	// Check whether a type is a number
	bool IsNumber( EBuiltinType T )
	{
		return IsIntNumber( T ) || IsRealNumber( T );
	}
	// Check whether a type is an integer number
	bool IsIntNumber( EBuiltinType T )
	{
		const SBuiltinTypeInfo &Info = GetBuiltinTypeInfo( T );
		return
			Info.bIsSigned || Info.bIsUnsigned;
	}
	// Check whether a type is a real number
	bool IsRealNumber( EBuiltinType T )
	{
		const SBuiltinTypeInfo &Info = GetBuiltinTypeInfo( T );
		return
			Info.bIsFloat || Info.bIsNormalized;
	}
	// Check whether a type is an SIMD type
	bool IsSIMD( EBuiltinType T )
	{
		const SBuiltinTypeInfo &Info = GetBuiltinTypeInfo( T );
		return
			Info.bIsSIMD;
	}
	// Check whether a type is signed
	bool IsSigned( EBuiltinType T )
	{
		return GetBuiltinTypeInfo( T ).bIsSigned;
	}
	// Check whether a type is unsigned
	bool IsUnsigned( EBuiltinType T )
	{
		return GetBuiltinTypeInfo( T ).bIsUnsigned;
	}
	// Retrieve the number of rows in a type
	unsigned GetTypeRows( EBuiltinType T )
	{
		return GetBuiltinTypeInfo( T ).cRows;
	}
	// Retrieve the number of columns in a type
	unsigned GetTypeColumns( EBuiltinType T )
	{
		return GetBuiltinTypeInfo( T ).cColumns;
	}
	// Retrieve the size of a built-in type, in bytes
	unsigned GetTypeSize( EBuiltinType T )
	{
		const SBuiltinTypeInfo &Info = GetBuiltinTypeInfo( T );

		if( Info.bIsPointer ) {
			return ( unsigned )g_Env->GetPointerSizeInBytes();
		}

		const unsigned cBits = Info.cBits;
		return cBits/8 + ( +( cBits%8 != 0 ) );
	}
	// Determine the type promotion between two types (Invalid if one can't be converted to the other)
	EBuiltinType FindTypePromotion( EBuiltinType T1, EBuiltinType T2 )
	{
		if( T1 == T2 ) {
			return T1;
		}

		const SBuiltinTypeInfo &Info1 = GetBuiltinTypeInfo( T1 );
		const SBuiltinTypeInfo &Info2 = GetBuiltinTypeInfo( T2 );

		// No promotion allowed and neither type matches
		if( Info1.bNoPromote || Info2.bNoPromote ) {
			return EBuiltinType::Invalid;
		}

		// If either type is a string
		if( IsString( T1 ) || IsString( T2 ) ) {
			return EBuiltinType::StringObject;
		}

		// If either type is a vector or matrix then the other shall will be promoted
		if( Info1.cRows!=0 || Info2.cRows!=0 ) {
			const unsigned cDims1 = Info1.cRows*Info1.cColumns;
			const unsigned cDims2 = Info2.cRows*Info2.cColumns;

			// Select the larger of the two
			if( cDims1 >= cDims2 ) {
				return T1;
			}

			return T2;
		}

		// If either type is a float
		if( Info1.bIsFloat || Info2.bIsFloat ) {
			const unsigned cFloatBits1 = Info1.bIsFloat ? Info1.cBits : 0;
			const unsigned cFloatBits2 = Info2.bIsFloat ? Info2.cBits : 0;

			// Select the largest of the floats
			return cFloatBits1 >= cFloatBits2 ? T1 : T2;
		}

		// If either type is normalized
		if( Info1.bIsNormalized || Info2.bIsNormalized ) {
			const unsigned cNormBits1 = Info1.bIsNormalized ? Info1.cBits : 0;
			const unsigned cNormBits2 = Info2.bIsNormalized ? Info2.cBits : 0;

			const unsigned cMaxBits = cNormBits1 > cNormBits2 ? cNormBits1 : cNormBits2;
			const bool bIsSigned = Info1.bIsSigned || Info2.bIsSigned;

			switch( cMaxBits )
			{
			case 8:
				return bIsSigned ? EBuiltinType::SNorm8 : EBuiltinType::UNorm8;

			case 16:
				return bIsSigned ? EBuiltinType::SNorm16 : EBuiltinType::UNorm16;

			case 32:
				return bIsSigned ? EBuiltinType::SNorm32 : EBuiltinType::UNorm32;

			case 64:
				return bIsSigned ? EBuiltinType::SNorm64 : EBuiltinType::UNorm64;

			default:
				AX_ASSERT_MSG( false, "Unreachable (Unhandled or invalid max bits; normalized)" );
			}

			return EBuiltinType::Invalid;
		}

		// If either type is unsigned
		if( Info1.bIsUnsigned || Info2.bIsUnsigned ) {
			const unsigned cIntBits1 = ( Info1.bIsUnsigned || Info1.bIsSigned ) ? Info1.cBits : 0;
			const unsigned cIntBits2 = ( Info2.bIsUnsigned || Info2.bIsSigned ) ? Info2.cBits : 0;
			const unsigned cMaxBits = cIntBits1 > cIntBits2 ? cIntBits1 : cIntBits2;

			switch( cMaxBits )
			{
			case 8:					return EBuiltinType::UInt8;
			case 16:				return EBuiltinType::UInt16;
			case 32:				return EBuiltinType::UInt32;
			case 64:				return EBuiltinType::UInt64;
			case 128:				return EBuiltinType::UInt128;

			default:
				AX_ASSERT_MSG( false, "Unreachable (Unhandled or invalid max bits; unsigned)" );
			}

			return EBuiltinType::Invalid;
		}
		// If either type is signed
		if( Info1.bIsSigned || Info2.bIsSigned ) {
			const unsigned cIntBits1 = ( Info1.bIsUnsigned || Info1.bIsSigned ) ? Info1.cBits : 0;
			const unsigned cIntBits2 = ( Info2.bIsUnsigned || Info2.bIsSigned ) ? Info2.cBits : 0;
			const unsigned cMaxBits = cIntBits1 > cIntBits2 ? cIntBits1 : cIntBits2;

			switch( cMaxBits )
			{
			case 8:					return EBuiltinType::Int8;
			case 16:				return EBuiltinType::Int16;
			case 32:				return EBuiltinType::Int32;
			case 64:				return EBuiltinType::Int64;
			case 128:				return EBuiltinType::Int128;

			default:
				AX_ASSERT_MSG( false, "Unreachable (Unhandled or invalid max bits; unsigned)" );
			}

			return EBuiltinType::Invalid;
		}

		// This type is not understood
		AX_ASSERT_MSG( false, "Unhandled type promotion" );
		return EBuiltinType::Invalid;
	}

	// Retrieve the component type of a vector/SIMD type
	EBuiltinType GetVectorTypeComponentType( EBuiltinType T )
	{
		switch( T )
		{
		case EBuiltinType::SIMDVector128_4Int32:
		case EBuiltinType::SIMDVector256_8Int32:
			return EBuiltinType::Int32;

		case EBuiltinType::SIMDVector128_2Int64:
		case EBuiltinType::SIMDVector256_4Int64:
			return EBuiltinType::Int64;

		case EBuiltinType::Vector2f:
		case EBuiltinType::Vector3f:
		case EBuiltinType::Vector4f:
		case EBuiltinType::Matrix2f:
		case EBuiltinType::Matrix3f:
		case EBuiltinType::Matrix4f:
		case EBuiltinType::Matrix23f:
		case EBuiltinType::Matrix24f:
		case EBuiltinType::Matrix32f:
		case EBuiltinType::Matrix34f:
		case EBuiltinType::Matrix42f:
		case EBuiltinType::Matrix43f:
		case EBuiltinType::Quaternionf:
		case EBuiltinType::SIMDVector128_4Float32:
		case EBuiltinType::SIMDVector256_8Float32:
			return EBuiltinType::Float32;

		case EBuiltinType::SIMDVector256_4Float64:
		case EBuiltinType::SIMDVector128_2Float64:
			return EBuiltinType::Float64;
		
		default:
			break;
		}

		return EBuiltinType::Invalid;
	}
	// Determine the swizzle type from the number of accesses and the core type
	EBuiltinType GetVectorSwizzleType( EBuiltinType ComponentType, unsigned cAxes )
	{
		if( cAxes <= 1 ) {
			return ComponentType;
		}

		if( ComponentType == EBuiltinType::Float32 ) {
			if( cAxes == 2 ) {
				return EBuiltinType::Vector2f;
			} else if( cAxes == 3 ) {
				return EBuiltinType::Vector3f;
			} else if( cAxes == 4 ) {
				return EBuiltinType::Vector4f;
			}
		}

		return EBuiltinType::Invalid;
	}

	// Convert a type to its integer equivalent (for casting)
	EBuiltinType GetCastSafeType( EBuiltinType T )
	{
		switch( T )
		{
		case EBuiltinType::Boolean:
			return EBuiltinType::Int8;
		}

		return T;
	}
	// Convert a given built-in type to its signed equivalent
	EBuiltinType MakeTypeSigned( EBuiltinType T )
	{
		switch( T )
		{
		case EBuiltinType::UInt8:			return EBuiltinType::Int8;
		case EBuiltinType::UInt16:			return EBuiltinType::Int16;
		case EBuiltinType::UInt32:			return EBuiltinType::Int32;
		case EBuiltinType::UInt64:			return EBuiltinType::Int64;
		case EBuiltinType::UInt128:			return EBuiltinType::Int128;
		case EBuiltinType::UNorm8:			return EBuiltinType::SNorm8;
		case EBuiltinType::UNorm16:			return EBuiltinType::SNorm16;
		case EBuiltinType::UNorm32:			return EBuiltinType::SNorm32;
		case EBuiltinType::UNorm64:			return EBuiltinType::SNorm64;
		}

		return T;
	}
	// Convert a given built-in type to its unsigned equivalent
	EBuiltinType MakeTypeUnsigned( EBuiltinType T )
	{
		switch( T )
		{
		case EBuiltinType::Int8:			return EBuiltinType::UInt8;
		case EBuiltinType::Int16:			return EBuiltinType::UInt16;
		case EBuiltinType::Int32:			return EBuiltinType::UInt32;
		case EBuiltinType::Int64:			return EBuiltinType::UInt64;
		case EBuiltinType::Int128:			return EBuiltinType::UInt128;
		case EBuiltinType::SNorm8:			return EBuiltinType::UNorm8;
		case EBuiltinType::SNorm16:			return EBuiltinType::UNorm16;
		case EBuiltinType::SNorm32:			return EBuiltinType::UNorm32;
		case EBuiltinType::SNorm64:			return EBuiltinType::UNorm64;
		}

		return T;
	}

	// Get the built-in cast operator for two built-in types
	//
	// Returns EBuiltinOp::None if no cast is necessary or EBuiltinOp::Invalid
	// if there is no direct cast between the types
	ECast GetCastForTypes( EBuiltinType FromT, EBuiltinType ToT, ECastMode Mode )
	{
		if( FromT == ToT || FromT == EBuiltinType::Any || ToT == EBuiltinType::Any ) {
			return ECast::None;
		}

		if( ToT == EBuiltinType::Boolean ) {
			if( IsIntNumber( FromT ) ) {
				return ECast::IntToBool;
			}

			if( IsRealNumber( FromT ) ) {
				return ECast::FloatToBool;
			}

			if( IsString( FromT ) ) {
				return ECast::PtrToBool;
			}

			if( FromT == EBuiltinType::ArrayObject || FromT == EBuiltinType::LinkedListObject || FromT == EBuiltinType::BinaryTreeObject ) {
				return ECast::PtrToBool;
			}
		}

		FromT = GetCastSafeType( FromT );
		ToT = GetCastSafeType( ToT );

		if( IsString( ToT ) ) {
			switch( FromT ) {
			case EBuiltinType::Int8:		return ECast::Int8ToStr;
			case EBuiltinType::Int16:		return ECast::Int16ToStr;
			case EBuiltinType::Int32:		return ECast::Int32ToStr;
			case EBuiltinType::Int64:		return ECast::Int64ToStr;
			case EBuiltinType::Int128:		return ECast::Int128ToStr;
			case EBuiltinType::UInt8:		return ECast::UInt8ToStr;
			case EBuiltinType::UInt16:		return ECast::UInt16ToStr;
			case EBuiltinType::UInt32:		return ECast::UInt32ToStr;
			case EBuiltinType::UInt64:		return ECast::UInt64ToStr;
			case EBuiltinType::UInt128:		return ECast::UInt128ToStr;
			case EBuiltinType::Float16:		return ECast::Float16ToStr;
			case EBuiltinType::Float32:		return ECast::Float32ToStr;
			case EBuiltinType::Float64:		return ECast::Float64ToStr;
			}

			if( Mode == ECastMode::Input && IsString( FromT ) ) {
				return ECast::None;
			}
		}

		if( ToT == EBuiltinType::StringObject ) {
			if( FromT == EBuiltinType::ConstUTF8Pointer ) {
				return ECast::UTF8PtrToStr;
			}

			if( FromT == EBuiltinType::ConstUTF16Pointer ) {
				return ECast::UTF16PtrToStr;
			}
		}

		if( IsIntNumber( FromT ) ) {
			if( IsIntNumber( ToT ) ) {
				const unsigned int FromTSize = GetTypeSize( FromT );
				const unsigned int ToTSize = GetTypeSize( ToT );

				if( IsUnsigned( ToT ) ) {
					if( FromTSize < ToTSize ) {
						return ECast::ZeroExtend;
					} else if( FromTSize > ToTSize ) {
						return ECast::ZeroTrunc;
					}
				} else {
					if( FromTSize < ToTSize ) {
						return ECast::SignExtend;
					} else if( FromTSize > ToTSize ) {
						return ECast::SignTrunc;
					}
				}

				return ECast::None;
			} else if( IsRealNumber( ToT ) ) {
				if( IsUnsigned( FromT ) ) {
					return ECast::UnsignedIntToFloat;
				} else {
					return ECast::SignedIntToFloat;
				}
			}
		} else if( IsRealNumber( FromT ) ) {
			if( IsIntNumber( ToT ) ) {
				if( IsUnsigned( ToT ) ) {
					return ECast::FloatToUnsignedInt;
				} else {
					return ECast::FloatToSignedInt;
				}
			} else if( IsRealNumber( ToT ) ) {
				const unsigned int FromTSize = GetTypeSize( FromT );
				const unsigned int ToTSize = GetTypeSize( ToT );

				if( FromTSize < ToTSize ) {
					return ECast::RealExtend;
				} else if( FromTSize > ToTSize ) {
					return ECast::RealTrunc;
				}

				return ECast::None;
			}
		}

		return ECast::Invalid;
	}

}}

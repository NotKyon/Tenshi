#pragma once

#include <Core/Types.hpp>
#include "Operator.hpp"

namespace Tenshi { namespace Compiler {

	// Whether a cast is specified explicitly or is implicit
	enum class ECastMode
	{
		// Cast was explicitly requested
		Explicit,

		// Implicit cast into input parameter
		Input,
		// Implicit cast from return value or into output parameter
		Output
	};

	// Base built-in type
	enum class EBuiltinType : Ax::uint8
	{
		// Invalid type
		Invalid,

		// Not a built-in type; custom user type
		UserDefined,
		// Nothing
		Void,
		// Anything
		Any,
		// Boolean value
		Boolean,
		// 8-bit signed integer
		Int8,
		// 16-bit signed integer
		Int16,
		// 32-bit signed integer
		Int32,
		// 64-bit signed integer
		Int64,
		// 128-bit signed integer
		Int128,
		// 8-bit unsigned integer
		UInt8,
		// 16-bit unsigned integer
		UInt16,
		// 32-bit unsigned integer
		UInt32,
		// 64-bit unsigned integer
		UInt64,
		// 128-bit unsigned integer
		UInt128,
		// 16-bit float
		Float16,
		// 32-bit float
		Float32,
		// 64-bit float
		Float64,
		// Unsigned-normalized 8-bit float
		UNorm8,
		// Unsigned-normalized 16-bit float
		UNorm16,
		// Unsigned-normalized 32-bit float
		UNorm32,
		// Unsigned-normalized 64-bit float
		UNorm64,
		// Signed-normalized 8-bit float
		SNorm8,
		// Signed-normalized 16-bit float
		SNorm16,
		// Signed-normalized 32-bit float
		SNorm32,
		// Signed-normalized 64-bit float
		SNorm64,
		// 128-bit SIMD vector (no specific format)
		SIMDVector128,
		// 256-bit SIMD vector
		SIMDVector256,
		// 128-bit SIMD vector of 4 Float32 items
		SIMDVector128_4Float32,
		// 128-bit SIMD vector of 2 Float64 items
		SIMDVector128_2Float64,
		// 128-bit SIMD vector of 4 Int32 items
		SIMDVector128_4Int32,
		// 128-bit SIMD vector of 2 Int64 items
		SIMDVector128_2Int64,
		// 256-bit SIMD vector of 8 Float32 items
		SIMDVector256_8Float32,
		// 256-bit SIMD vector of 4 Float64 items
		SIMDVector256_4Float64,
		// 256-bit SIMD vector of 8 Int32 items
		SIMDVector256_8Int32,
		// 256-bit SIMD vector of 4 Int64 items
		SIMDVector256_4Int64,
		// C string (const char *)
		ConstUTF8Pointer,
		// C string (const wchar_t *)
		ConstUTF16Pointer,
		// String object
		StringObject,
		// Array object
		ArrayObject,
		// Linked list object
		LinkedListObject,
		// Binary tree object
		BinaryTreeObject,
		
		// 2 x Float32 as a vector
		Vector2f,
		// 3 x Float32 as a vector
		Vector3f,
		// 4 x Float32 as a vector
		Vector4f,

		// 2x2 x Float32 as a matrix
		Matrix2f,
		// 3x3 x Float32 as a matrix
		Matrix3f,
		// 4x4 x Float32 as a matrix
		Matrix4f,
		// 2x3 x Float32 as a matrix
		Matrix23f,
		// 2x4 x Float32 as a matrix
		Matrix24f,
		// 3x2 x Float32 as a matrix
		Matrix32f,
		// 3x4 x Float32 as a matrix
		Matrix34f,
		// 4x2 x Float32 as a matrix
		Matrix42f,
		// 4x3 x Float32 as a matrix
		Matrix43f,
		
		// 4 x Float32 as a quaternion
		Quaternionf
	};

	// Details of a built-in type
	struct SBuiltinTypeInfo
	{
		bool						bIsSigned;
		bool						bIsUnsigned;
		bool						bIsFloat;
		bool						bIsNormalized;
		bool						bIsSIMD;
		bool						bIsPointer; //if set then cBits should be ignored
		unsigned					cRows;
		unsigned					cColumns;
		unsigned					cBits;
		bool						bNoPromote;
	};

	// Retrieve the built-in type information for a given type
	const SBuiltinTypeInfo &GetBuiltinTypeInfo( EBuiltinType T );

	// Convert a built-in type to a string
	const char *BuiltinTypeToString( EBuiltinType Type );

	// Check whether a built-in type is trivial
	bool IsTrivial( EBuiltinType T );
	// Check whether a type is a plain-old-data type
	bool IsPOD( EBuiltinType T );
	// Check whether a type is a string (of some form)
	bool IsString( EBuiltinType T );
	// Check whether a type is a number
	bool IsNumber( EBuiltinType T );
	// Check whether a type is an integer number
	bool IsIntNumber( EBuiltinType T );
	// Check whether a type is a real number
	bool IsRealNumber( EBuiltinType T );
	// Check whether a type is an SIMD type
	bool IsSIMD( EBuiltinType T );
	// Check whether a type is signed
	bool IsSigned( EBuiltinType T );
	// Check whether a type is unsigned
	bool IsUnsigned( EBuiltinType T );
	// Retrieve the number of rows in a type
	unsigned GetTypeRows( EBuiltinType T );
	// Retrieve the number of columns in a type
	unsigned GetTypeColumns( EBuiltinType T );
	// Retrieve the size of a built-in type, in bytes
	unsigned GetTypeSize( EBuiltinType T );
	// Determine the type promotion between two types (Invalid if one can't be converted to the other)
	EBuiltinType FindTypePromotion( EBuiltinType T1, EBuiltinType T2 );

	// Retrieve the component type of a vector/SIMD type
	EBuiltinType GetVectorTypeComponentType( EBuiltinType T );
	// Determine the swizzle type from the number of accesses and the core type
	EBuiltinType GetVectorSwizzleType( EBuiltinType ComponentType, unsigned cAxes );
	
	// Convert a type to its integer equivalent (for casting)
	EBuiltinType GetCastSafeType( EBuiltinType T );
	// Convert a given built-in type to its signed equivalent
	EBuiltinType MakeTypeSigned( EBuiltinType T );
	// Convert a given built-in type to its unsigned equivalent
	EBuiltinType MakeTypeUnsigned( EBuiltinType T );
	// Get the built-in cast operator for two built-in types
	//
	// Returns EBuiltinOp::None if no cast is necessary or EBuiltinOp::Invalid
	// if there is no direct cast between the types
	ECast GetCastForTypes( EBuiltinType FromT, EBuiltinType ToT, ECastMode Mode );

}}

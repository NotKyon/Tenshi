#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Collections/Dictionary.hpp>
#include <Collections/List.hpp>
#include <Collections/Array.hpp>

#include "Node.hpp"

namespace Tenshi { namespace Compiler {

	/*

		TYPE PATTERNS
		=============
		Type patterns are sequences of characters used to specify parameters or
		structures (or both). These are used primarily by plug-ins.

		The following is an excerpt from "TYPE PATTERNS" in
		TechDocs/Plugins.txt.
		
		Single-Character Sequences:
			0 = Void
			X = Any
			B = Boolean (B = Boolean)
			C = Int8 (C = Character)
			N = Int16 (N = Number)
			L = Int32 (L = Long number)
			R = Int64 (R = Really long number)
			Y = Uint8 (Y = bYte)
			W = Uint16 (W = Word)
			D = Uint32 (D = Double word)
			Q = Uint64 (Q = Quadruple word)
			A = Float16 (A = hAlf float)
			F = Float32 (F = single Float)
			O = Float64 (O = dOuble float)
			S = String (const char *; S = String)
			T = String (const wchar_t * (UTF-16); T = sTring)
			G = String object (G = strinG)
			H = Array (H = array Handle)
			M = Dictionary (M = Map)
			K = Linked List (K = linKed list)
			@ = Attribute

		Double-Character Sequences:
			I1 = Int8
			I2 = Int16
			I4 = Int32
			I8 = Int64
			IP = IntPtr
			U1 = Uint8
			U2 = Uint16
			U4 = Uint32
			U8 = Uint64
			UP = UintPtr
			#2 = Float16
			#4 = Float32
			#8 = Float64

		Triple-Character Sequences:
			V4X = SIMDVector128 (any type)
			V8X = SIMDVector256 (any type)
			V4F = SIMDVector128_4Float32
			V2O = SIMDVector128_2Float64
			V4L = SIMDVector128_4Int32
			V2R = SIMDVector128_2Int64
			V8F = SIMDVector256_8Float32
			V4O = SIMDVector256_4Float64
			V8L = SIMDVector256_8Int32
			V8R = SIMDVector256_4Int64
			... = Any number of parameters (c-style)

		Quadruple-Character Sequences:
			EF21 = Vector2f
			EF22 = Matrix2f
			EF23 = Matrix23f
			EF24 = Matrix24f
			EF31 = Vector3f
			EF32 = Matrix32f
			EF33 = Matrix3f
			EF34 = Matrix34f
			EF41 = Vector4f
			EF42 = Matrix42f
			EF43 = Matrix43f
			EF44 = Matrix4f
			EF4Q = Quaternionf
			EI16 = 128-bit signed integer
			EU16 = 128-bit unsigned integer
			EUN1 = 8-bits representing unsigned-normalized float (0.0 to 1.0; UNorm8)
			EUN2 = 16-bits representing unsigned-normalized float (0.0 to 1.0; UNorm16)
			EUN4 = 32-bits representing unsigned-normalized float (0.0 to 1.0; UNorm32)
			EUN8 = 64-bits representing unsigned-normalized float (0.0 to 1.0; UNorm64)
			ESN1 = 8-bits representing signed-normalized float (-1.0 to 1.0; SNorm8)
			ESN2 = 16-bits representing signed-normalized float (-1.0 to 1.0; SNorm16)
			ESN4 = 32-bits representing signed-normalized float (-1.0 to 1.0; SNorm32)
			ESN8 = 64-bits representing signed-normalized float (-1.0 to 1.0; SNorm64)

		Sequence Modifiers:
			< = Type is to be passed by reference for input (will not modify the data)
			> = Type is to be passed by reference for output (will not read the data)
			* = Type is to be passed by reference for input/output
			~ = Zero or more parameters passed as an array internally (not in user code)
			+ = One or more parameters passed as an array internally (not in user code)
			? = Zero or one parameter passed as an array internally (not in user code)
			= = Select a default value if one is not provided
			: = Labelled parameter

	*/

	struct SMemberInfo;
	struct STypeInfo;
	struct SFunctionOverload;
	struct SFunctionInfo;

	struct SModule;
	struct SPlatform;

	class CExpression;

	class CScope;

	// How an instance should be passed
	enum class EPassBy : Ax::uint8
	{
		// Not a parameter
		Direct,
		// Copy into the local stack directly
		Value,
		// Write the pointer to the stack
		Reference
	};
	// Instance passing modification
	enum class EPassMod : Ax::uint8
	{
		// Just pass the value through unmodified/as-is
		Direct,
		// Pass as an array of zero or more references (corresponds to "~")
		ArrayOfZeroOrMoreReferences,
		// Pass as an array of one or more references (corresponds to "+")
		ArrayOfOneOrMoreReferences,
		// Pass as a pointer that can be null (corresponds to "?")
		OptionalPointer
	};

	// Information about a single type member
	struct SMemberInfo
	{
		// Name of the member
		Ax::String					Name;
		// Reference to the type
		STypeRef					Type;
		// How the member should be passed
		EPassBy						PassBy;
		// How the member may be passed (when this describes a parameter)
		EPassMod					PassMod;
		// Offset in the stack/global-storage or type of this member
		Ax::uint32					uOffset;
		// The field index of this member
		Ax::intptr					iFieldIndex;

		inline SMemberInfo()
		: Name()
		, Type()
		, PassBy( EPassBy::Direct )
		, PassMod( EPassMod::Direct )
		, uOffset( 0 )
		, iFieldIndex( -1 )
		{
		}

		Ax::String ToString() const;
	};
	// Information about a custom user type
	struct STypeInfo
	{
		// Name of the type
		Ax::String					Name;
		// All members available in this type
		Ax::TArray< SMemberInfo >	Members;
		// Entire size of the type
		Ax::uint32					cBytes;
		// Alignment restriction of type instances
		Ax::uint32					uAlignment;
		// Whether constructing the type is trivial (zero out)
		bool						bIsInitTrivial;
		// Whether destructing the type is trivial (no destructor or memory to clean)
		bool						bIsFiniTrivial;
		// Whether copying the type is trivial (simple memcpy)
		bool						bIsCopyTrivial;
		// Whether moving the type is trivial (simple memcpy from src to dst, and zero out src)
		bool						bIsMoveTrivial;
		// Scope of the type (where all fields are stored for look-ups)
		CScope *					pScope;
		// LLVM type for this structure
		llvm::Type *				pLLVMTy;

		// Index to the function in the global type description table
		llvm::Constant *			pLLVMElementIndex;

		// Constructor function
		llvm::Function *			pLLVMInitFn;
		// Destructor function
		llvm::Function *			pLLVMFiniFn;
		// Copy function
		llvm::Function *			pLLVMCopyFn;
		// Move function
		llvm::Function *			pLLVMMoveFn;

		Ax::String ToString() const;
	};
	// Overload of a function
	struct SFunctionOverload
	{
		// Name of this function internally
		Ax::String					RealName;
		// Return value information (if present)
		SMemberInfo					ReturnInfo;
		// List of parameters to the function
		Ax::TArray< SMemberInfo >	Parameters;
		// Cached type pattern (used for checking function overloads)
		Ax::String					ParmTypePattern;
		// Where the function came from
		SModule *					pModule;

		// List of LLVM parameter types
		Ax::TArray< llvm::Type * >	LLVMTypes;
		// The return type of the function
		llvm::Type *				pLLVMReturnType;
		// The LLVM function type
		llvm::FunctionType *		pLLVMFuncType;
		// The LLVM function
		llvm::Function *			pLLVMFunc;

		inline SFunctionOverload()
		: RealName()
		, ReturnInfo()
		, Parameters()
		, ParmTypePattern()
		, pModule( nullptr )
		, LLVMTypes()
		, pLLVMReturnType( nullptr )
		, pLLVMFuncType( nullptr )
		, pLLVMFunc( nullptr )
		{
		}

		// Generate the declaration for LLVM
		bool GenDecl();
	};
	// Information about a function
	struct SFunctionInfo
	{
		// Name of the function
		Ax::String					Name;
		// Function overloads
		Ax::TList< SFunctionOverload > Overloads;

		inline SFunctionInfo()
		: Name()
		, Overloads()
		{
		}

		// Retrieve pointer to best matching function overload
		// Returns nullptr if no valid match could be found
		SFunctionOverload *FindMatch( const Ax::TArray< SExpr > &Subexpressions, Ax::TArray< ECast > &OutCasts ) const;

		// Generate the declarations
		bool GenDecls();
	};

	const char *PassByToString( EPassBy PassBy );
	const char *PassModToString( EPassMod PassMod );

	Ax::String GetTypePattern( const Ax::TArray< SMemberInfo > &InMembers );

	const char *ParseTypePatternPart( SMemberInfo &OutMember, const char *pszPattern, const SPlatform &Platform );
	bool ParseTypePattern( Ax::TArray< SMemberInfo > &OutMembers, const char *pszPattern, const SPlatform &Platform );
	bool ParseTypePattern( SMemberInfo &OutMember, const char *pszPattern, const SPlatform &Platform );

}}

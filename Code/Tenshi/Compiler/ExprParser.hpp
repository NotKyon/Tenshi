#pragma once

#include "Node.hpp"
#include "Parser.hpp"
#include "Symbol.hpp"
#include "Operator.hpp"

namespace Tenshi { namespace Compiler {

	struct SFunctionOverload;

	enum ESwizzleAxis : Ax::uint8
	{
		axis_x = 0,
		axis_y = 1,
		axis_z = 2,
		axis_w = 3
	};

	inline Ax::uint8 MakeSwizzleMask( ESwizzleAxis a, ESwizzleAxis b,  ESwizzleAxis c, ESwizzleAxis d )
	{
		return ( ( a & 3 )<<6 ) | ( ( b & 3 )<<4 ) | ( ( c & 3 )<<2 ) | ( ( d & 3 )<<0 );
	}
	inline const char *AxisToString( ESwizzleAxis Axis )
	{
		switch( Axis )
		{
		case axis_x:				return "x";
		case axis_y:				return "y";
		case axis_z:				return "z";
		case axis_w:				return "w";
		}

		return "\?";
	}
	inline bool SwizzleIsLValue( Ax::uint8 uMask, Ax::uint8 cAxes, Ax::uint8 cTypeAxes )
	{
		// If no axes in swizzle then not swizzling
		if( !cAxes ) {
			return true;
		}

		// If there are more axes than are available in the type then a different type is being made
		if( cAxes > cTypeAxes ) {
			return false;
		}

		// Store the mask (for easier reference)
		const Ax::uint8 v[ 4 ] = {
			Ax::uint8( ( uMask>>6 ) & 3 ),
			Ax::uint8( ( uMask>>4 ) & 3 ),
			Ax::uint8( ( uMask>>2 ) & 3 ),
			Ax::uint8( ( uMask>>0 ) & 3 )
		};

		// Check for any duplicates (would require a temporary vector)
		for( Ax::uint8 i = 0; i < cAxes; ++i ) {
			// Check for an ill-formed swizzle
			if( v[ i ] >= cTypeAxes ) {
				return false;
			}

			// Check for a duplicate
			for( Ax::uint8 j = i + 1; j < cAxes; ++j ) {
				if( v[ i ] == v[ j ] ) {
					return false;
				}
			}
		}

		// No duplicates found; this is an l-value
		return true;
	}

	inline char *SwizzleToString( char *pszDst, Ax::uintptr cDst, Ax::uint8 uMask, Ax::uint8 cAxes )
	{
		AX_ASSERT_NOT_NULL( pszDst );
		AX_ASSERT( cAxes <= 4 );

		Ax::StrCpy( pszDst, cDst, "" );
		if( !cAxes ) {
			return pszDst;
		}

		Ax::StrCat( pszDst, cDst, "." );
		for( Ax::uint8 i = 0; i < cAxes; ++i ) {
			const Ax::uint8 t = ( uMask>>( 6 - i*2 ) ) & 3;
			Ax::StrCat( pszDst, cDst, AxisToString( ( ESwizzleAxis )t ) );
		}

		return pszDst;
	}

	// Specifies the type of an expression list (CExpressionList)
	enum class EExprListType
	{
		// Parameters to a function call
		FunctionCall,
		// An array subscript
		ArraySubscript
	};

	//
	//	Expression List
	//	===============
	//	Stores a group of zero or more expressions. Each expression would
	//	syntactically be separated by a comma.
	//
	//	The expression list can either be in the form of a function call or an
	//	array subscript.
	//
	//	e.g., ()                        Empty function call
	//	e.g., ( 3, 7 + 2, "Hello" )     Function call with three expressions
	//	e.g., [ i ]                     Array subscript with one expression
	//	e.g., [ j + 1 ]                 Array subscript with one expression
	//
	class CExpressionList: public CExpression
	{
	friend class CParser;
	public:
		typedef Ax::TArray< SExpr >	ArrayType;
		typedef SExpr				ElementType;

		CExpressionList( const SToken &Token, CParser &Parser );
		virtual ~CExpressionList();

		template< typename tType >
		inline tType &AddExpr( const SToken &Token )
		{
			AX_EXPECT_MSG( m_Subexpressions.Append( nullptr ), "Out of memory" );

			tType *const pSubexpr = new tType( Token, Parser() );
			AX_EXPECT_MEMORY( pSubexpr );

			m_Subexpressions.Last() = pSubexpr;
			pSubexpr->SetParent( this );

			return *pSubexpr;
		}

		inline EExprListType ListType() const
		{
			return m_ListType;
		}

		inline ArrayType &Subexpressions() const
		{
			return const_cast< ArrayType & >( m_Subexpressions );
		}

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		const EExprListType			m_ListType;
		ArrayType					m_Subexpressions;

		AX_DELETE_COPYFUNCS(CExpressionList);
	};

	//
	//	Literal Expression
	//	==================
	//	Represents a numeric or string literal. e.g., 14 or "This is text"
	//
	//	This stores the encoded literal (without byte-swapping) either with the
	//	instance of this class, or somewhere in memory depending on how big the
	//	literal is. If the literal is <= 64-bits then it is stored in with the
	//	instance. Otherwise, the literal is > 64-bits and is allocated and
	//	deallocated dynamically.
	//
	class CLiteralExpr: public CExpression
	{
	friend class CParser;
	public:
		CLiteralExpr( const SToken &Token, CParser &Parser );
		virtual ~CLiteralExpr();

		Ax::uint32 SizeInBits() const;
		Ax::uint32 SizeInBytes() const;
		const Ax::uint8 *DataPointer() const;

		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual Ax::String ToString() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		union {
			Ax::uint64				m_ValueI;
			double					m_ValueF;
			Ax::uint8 *				m_pValue;
		};
		Ax::uint32					m_SizeInBits;
		STypeRef					m_Type;
		struct
		{
			llvm::Value *			pValue;
		}							m_CodeGen;

		AX_DELETE_COPYFUNCS(CLiteralExpr);
	};
	inline Ax::uint32 CLiteralExpr::SizeInBits() const
	{
		return m_SizeInBits;
	}
	inline Ax::uint32 CLiteralExpr::SizeInBytes() const
	{
		return m_SizeInBits/8 + ( +( m_SizeInBits%8 != 0 ) );
	}
	inline const Ax::uint8 *CLiteralExpr::DataPointer() const
	{
		return m_SizeInBits <= 64 ? ( const Ax::uint8 * )&m_ValueI : m_pValue;
	}
	
	//
	//	Name Expression
	//	===============
	//	Represents a variable or function identifier.
	//
	class CNameExpr: public CExpression
	{
	friend class CParser;
	public:
		CNameExpr( const SToken &Token, CParser &Parser );
		virtual ~CNameExpr();

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const SSymbol *GetSymbol() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		// Valid after successful call to Semant()
		struct
		{
			// Symbol for the name
			const SSymbol *			pSym;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CNameExpr);
	};
	//
	//	Member Expression [Postfix]
	//	===========================
	//	Represents access to a field (as through "<blah>.someField")
	//
	class CMemberExpr: public CExpression
	{
	friend class CParser;
	public:
		CMemberExpr( const SToken &Tok, CParser &Parser );
		virtual ~CMemberExpr();

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const SSymbol *GetSymbol() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		// Expression this operates on
		CExpression *				m_pLHS;

		// Valid after successful call to Semant()
		struct
		{
			// Symbol for this member (nullptr if this is a swizzle)
			const SSymbol *			pSym;

			// Swizzle ordering (only valid on vectors or quaternions)
			Ax::uint8				uSwizzleMask;
			// Number of components in swizzle (only valid on vectors or quaternions)
			Ax::uint8				cSwizzleMaxAxes;
			// Number of axes for the variable type / return type
			Ax::uint8				cSwizzleAxes;
			// The type this swizzle represents
			STypeRef				SwizzleType;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CMemberExpr);
	};
	//
	//	Array Subscript Expression [Postfix]
	//	====================================
	//	Represents an array being indexed (e.g., myArr[3])
	//
	class CArraySubscriptExpr: public CExpression
	{
	friend class CParser;
	public:
		CArraySubscriptExpr( const SToken &Tok, CParser &Parser );
		virtual ~CArraySubscriptExpr();

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		// Expression this operates on
		CExpression *				m_pLHS;
		// List of subscripts (these need to resolve to integers)
		CExpressionList *			m_pList;
		// Valid after successful call to Semant()
		struct
		{
			// Type of an individual element being referenced
			STypeRef				Type;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CArraySubscriptExpr);
	};
	//
	//	Function Call Expression [Postfix]
	//	==================================
	//	Represents a function call as through "<blah>( parms... )" e.g.,
	//	get_sum( 2, 2 )
	//
	class CFuncCallExpr: public CExpression
	{
	friend class CParser;
	public:
		CFuncCallExpr( const SToken &Token, CParser &Parser );
		virtual ~CFuncCallExpr();

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		// Expression this is operating on
		CExpression *				m_pLHS;
		// List of parameters for the function
		CExpressionList *			m_pList;

		// Valid after successful call to Semant()
		struct
		{
			// The specific function overload (if this is a function)
			SFunctionOverload *		pFuncOverload;
			// Casts needed for the call to the function (each index corresponds to a subexpression in list)
			Ax::TArray< ECast >		Casts;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CFuncCallExpr);
	};

	//
	//	Unary Expression
	//	================
	//	Represents an operation done on a single expression. For example, a
	//	negation or postfix-increment.
	//
	//	Unary expressions can either be prefix- or postfix-operations. A prefix
	//	operation works on the data before it is interpreted by other
	//	expressions. A postfix operation works on the data after it has been
	//	interpreted by other expressions.
	//
	//	For example:
	//
	//		-3 + 7 would be ( -3 ) + 7.                     "-" is prefix here
	//		x-- > 0 would be ( x > 0 ) then x = x - 1       "--" is postfix here
	//		--x > 0 would be x = x - 1 then ( x > 0 )	    "--" is prefix here
	//		y++ == z would be y == z then y = y + 1         "++" is postfix here
	//
	class CUnaryExpr: public CExpression
	{
	friend class CParser;
	public:
		CUnaryExpr( const SToken &Token, CParser &Parser );
		virtual ~CUnaryExpr();

		EBuiltinOp Operator() const;
		bool IsPostfix() const;

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		EBuiltinOp					m_Operator;
		bool						m_bIsPostfix;
		CExpression *				m_pSubexpr;
		struct
		{
			STypeRef				Type;
			ECast					RHSCast;
		}							m_Semanted;

		AX_DELETE_COPYFUNCS(CUnaryExpr);
	};
	//
	//	Binary Expression
	//	=================
	//	Represents an operation done on a left-hand-side (LHS) expression and a
	//	right-hand-side (RHS) expression. For example, adding or subtracting.
	//
	//	
	//
	class CBinaryExpr: public CExpression
	{
	friend class CParser;
	public:
		CBinaryExpr( const SToken &Token, CParser &Parser );
		virtual ~CBinaryExpr();

		EBuiltinOp Operator() const;

		virtual Ax::String ToString() const AX_OVERRIDE;
		virtual const STypeRef *GetType() const AX_OVERRIDE;

		virtual bool Semant() AX_OVERRIDE;
		virtual SValue CodeGen() AX_OVERRIDE;

	private:
		EBuiltinOp					m_Operator;
		CExpression *				m_pLHS;
		CExpression *				m_pRHS;
		struct
		{
			ECast					LHSCast;
			ECast					RHSCast;
			EBuiltinType			PromotionType;
			STypeRef				ResultType;
		}							m_Semanted;
		struct
		{
			llvm::Value *			pValue;
		}							m_CodeGen;

		AX_DELETE_COPYFUNCS(CBinaryExpr);
	};

}}

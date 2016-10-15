#include "_PCH.hpp"
#include "ExprParser.hpp"
#include "Program.hpp"
#include "TypeInformation.hpp"
#include "CodeGen.hpp"

using namespace Ax;
using namespace Ax::Parser;

namespace Tenshi { namespace Compiler {

	static const char *BuiltinOpToString( EBuiltinOp Op )
	{
		switch( Op ) {
		case EBuiltinOp::Invalid:	return "(invalid)";
		case EBuiltinOp::None:		return "(nop)";

		case EBuiltinOp::Add:		return "+";
		case EBuiltinOp::Sub:		return "-";
		case EBuiltinOp::Mul:		return "*";
		case EBuiltinOp::Div:		return "/";
		case EBuiltinOp::Mod:		return "mod";
		case EBuiltinOp::Pow:		return "pow";
		case EBuiltinOp::BitSL:		return "<<";
		case EBuiltinOp::BitSR:		return ">>";
		case EBuiltinOp::BitRL:		return "bitrl";
		case EBuiltinOp::BitRR:		return "bitrr";
		case EBuiltinOp::BitAnd:	return "&";
		case EBuiltinOp::BitOr:		return "|";
		case EBuiltinOp::BitXor:	return "xor";
		case EBuiltinOp::BitNot:	return "bitnot";
		case EBuiltinOp::CmpEq:		return "==";
		case EBuiltinOp::CmpNe:		return "!=";
		case EBuiltinOp::CmpLt:		return "<";
		case EBuiltinOp::CmpGt:		return ">";
		case EBuiltinOp::CmpLe:		return "<=";
		case EBuiltinOp::CmpGe:		return ">=";
		case EBuiltinOp::RelAnd:	return "&&";
		case EBuiltinOp::RelOr:		return "||";
		case EBuiltinOp::RelXor:	return "relxor";
		case EBuiltinOp::RelNot:	return "!";
		case EBuiltinOp::Neg:		return "-";
		case EBuiltinOp::Deref:		return "deref";
		case EBuiltinOp::Addr:		return "adrof";
		case EBuiltinOp::FieldRef:	return "fieldref";

		case EBuiltinOp::StrConcat:						return "[str]+";
		case EBuiltinOp::StrRemove:						return "[str]-";
		case EBuiltinOp::StrRepeat:						return "[str]*";
		case EBuiltinOp::StrAddPath:					return "[str]/";
		}

		return "(Op:Unknown)";
	}
	
	/*
	===========================================================================

		EXPRESSION LIST

	===========================================================================
	*/
	CExpressionList::CExpressionList( const SToken &Token, CParser &Parser )
	: CExpression( EExprType::ExpressionList, Token, Parser )
	, m_ListType( Token.IsPunctuation( "[" ) ? EExprListType::ArraySubscript : EExprListType::FunctionCall )
	, m_Subexpressions()
	{
	}
	CExpressionList::~CExpressionList()
	{
		for( CExpression *pExpr : m_Subexpressions ) {
			delete pExpr;
		}
		m_Subexpressions.Clear();
	}

	Ax::String CExpressionList::ToString() const
	{
		Ax::String Result;

		bool bNeedSpace = false;
		switch( m_ListType )
		{
		case EExprListType::FunctionCall:
			AX_EXPECT_MEMORY( Result.Append( "(" ) );
			for( const CExpression *pExpr : m_Subexpressions ) {
				AX_ASSERT_NOT_NULL( pExpr );

				if( bNeedSpace ) {
					AX_EXPECT_MEMORY( Result.Append( " " ) );
				} else {
					bNeedSpace = true;
				}

				AX_EXPECT_MEMORY( Result.Append( pExpr->ToString() ) );
			}
			AX_EXPECT_MEMORY( Result.Append( ")" ) );
			break;
		case EExprListType::ArraySubscript:
			AX_EXPECT_MEMORY( Result.Append( "[" ) );
			for( const CExpression *pExpr : m_Subexpressions ) {
				AX_ASSERT_NOT_NULL( pExpr );

				if( bNeedSpace ) {
					AX_EXPECT_MEMORY( Result.Append( ", " ) );
				} else {
					bNeedSpace = true;
				}

				AX_EXPECT_MEMORY( Result.Append( pExpr->ToString() ) );
			}
			AX_EXPECT_MEMORY( Result.Append( "]" ) );
			break;
		}

		return Result;
	}
	
	bool CExpressionList::Semant()
	{
		for( CExpression *pExpr : m_Subexpressions ) {
			if( !pExpr->Semant() ) {
				return false;
			}
		}

		return true;
	}
	SValue CExpressionList::CodeGen()
	{
		return SValue();
	}


	/*
	===========================================================================

		LITERAL EXPRESSION

	===========================================================================
	*/
	CLiteralExpr::CLiteralExpr( const SToken &Token, CParser &Parser )
	: CExpression( EExprType::Literal, Token, Parser )
	, m_ValueI( 0 )
	, m_SizeInBits( 0 )
	, m_Type()
	{
		memset( &m_CodeGen, 0, sizeof( m_CodeGen ) );
	}
	CLiteralExpr::~CLiteralExpr()
	{
	}

	const STypeRef *CLiteralExpr::GetType() const
	{
		return &m_Type;
	}

	Ax::String CLiteralExpr::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( Token().GetString() ) );

		return Result;
	}

	bool CLiteralExpr::Semant()
	{
		static const uint64 kBiggest32 = ( int64 )( ~( int32 )0 );
		static const uint64 kBiggest16 = ( int64 )( ~( int16 )0 );
		static const uint64 kBiggest08 = ( int64 )( ~( int8  )0 );

		m_Type.Access = EAccess::ReadOnly;
		m_Type.bCanReorderMemAccesses = true;
		m_Type.pCustomType = nullptr;
		m_Type.pRef = nullptr;

		if( Token().IsNumber() ) {
			if( Token().NumberType == ENumberTokenType::Real ) {
				if( Token().Qualifier == ENumberTokenQualifier::Long ) {
					m_Type.BuiltinType = EBuiltinType::Float64;
					m_SizeInBits = 64;
				} else {
					m_Type.BuiltinType = EBuiltinType::Float32;
					m_SizeInBits = 32;
				}

				m_ValueI = Token().uLiteral;
			} else {
				const uint64 u = Token().uLiteral;
				const int64 v_ = ( int64 )u;
				const uint64 s = v_ < 0 ? -v_ : v_;

				//
				//	TODO: Warn if the number will be cut off
				//

				switch( Token().Qualifier ) {
				case ENumberTokenQualifier::Unqualified:
					if( s > ( int64 )kBiggest32 ) {
						m_Type.BuiltinType = EBuiltinType::Int64;
						m_SizeInBits = 64;
					} else {
						m_Type.BuiltinType = EBuiltinType::Int32;
						m_SizeInBits = 32;
					}
					break;
				case ENumberTokenQualifier::Unsigned:
					if( u > kBiggest32 ) {
						m_Type.BuiltinType = EBuiltinType::UInt64;
						m_SizeInBits = 64;
					} else {
						m_Type.BuiltinType = EBuiltinType::UInt32;
						m_SizeInBits = 32;
					}
					break;
				case ENumberTokenQualifier::Long:
					m_Type.BuiltinType = EBuiltinType::Int32;
					m_SizeInBits = 32;
					break;
				case ENumberTokenQualifier::LongLong:
					m_Type.BuiltinType = EBuiltinType::Int64;
					m_SizeInBits = 64;
					break;
				case ENumberTokenQualifier::UnsignedLongLong:
					m_Type.BuiltinType = EBuiltinType::UInt64;
					m_SizeInBits = 64;
					break;
				case ENumberTokenQualifier::SignedInt8:
					m_Type.BuiltinType = EBuiltinType::Int8;
					m_SizeInBits = 8;
					break;
				case ENumberTokenQualifier::SignedInt16:
					m_Type.BuiltinType = EBuiltinType::Int16;
					m_SizeInBits = 16;
					break;
				case ENumberTokenQualifier::SignedInt32:
					m_Type.BuiltinType = EBuiltinType::Int32;
					m_SizeInBits = 32;
					break;
				case ENumberTokenQualifier::SignedInt64:
					m_Type.BuiltinType = EBuiltinType::Int64;
					m_SizeInBits = 64;
					break;
				case ENumberTokenQualifier::UnsignedInt8:
					m_Type.BuiltinType = EBuiltinType::UInt8;
					m_SizeInBits = 8;
					break;
				case ENumberTokenQualifier::UnsignedInt16:
					m_Type.BuiltinType = EBuiltinType::UInt16;
					m_SizeInBits = 16;
					break;
				case ENumberTokenQualifier::UnsignedInt32:
					m_Type.BuiltinType = EBuiltinType::UInt32;
					m_SizeInBits = 32;
					break;
				case ENumberTokenQualifier::UnsignedInt64:
					m_Type.BuiltinType = EBuiltinType::UInt64;
					m_SizeInBits = 64;
					break;
				case ENumberTokenQualifier::Float:
				default:
					AX_ASSERT_MSG( false, "Unreachable (ENumberTokenQualifier)" );
					Token().Error( "[INTERNAL-ERROR] Invalid ENumberTokenQualifier" );
					return false;
				}

				m_ValueI = u;
			}

			m_Type.cBytes = m_SizeInBits/8 + ( +( m_SizeInBits%8 != 0 ) );
		} else {
			//
			//	TODO: Ensure this is actually a UTF-8 string pointer
			//

			AX_ASSERT_NOT_NULL( Token().pSource );

			m_Type.BuiltinType = EBuiltinType::ConstUTF8Pointer;
			m_Type.cBytes = ( Ax::uint32 )Token().Data.cBytes;
			// NOTE: The data shouldn't be actively modified at this point
			m_pValue = ( Ax::uint8 * )Token().pSource->GetData().Pointer( Token().Data.uOffset );
		}

		return true;
	}
	SValue CLiteralExpr::CodeGen()
	{
		if( m_CodeGen.pValue != nullptr ) {
			return m_CodeGen.pValue;
		}

		llvm::Type *const pType = m_Type.CodeGen();
		if( !pType ) {
			return nullptr;
		}

		const SToken &Tok = Token();
		llvm::Value *pValue = nullptr;

		if( Tok.IsKeyword( kKeyword_False ) ) {
			pValue = llvm::ConstantInt::get( pType, llvm::APInt( 1, 0, false ) );
		} else if( Tok.IsKeyword( kKeyword_True ) ) {
			pValue = llvm::ConstantInt::get( pType, llvm::APInt( 1, 1, false ) );
		} else if( Tok.IsNumber() ) {
			if( Tok.NumberType == Parser::ENumberTokenType::Integer ) {
				pValue =
					llvm::ConstantInt::get
					(
						pType,
						llvm::APInt
						(
							m_Type.cBytes*8,
							Tok.uLiteral,
							IsSigned( m_Type.BuiltinType )
						)
					);
			} else {
				pValue = llvm::ConstantFP::get( pType, Tok.GetEncodedFloat() );
			}
		} else if( Tok.IsString() ) {
			AX_ASSERT_NOT_NULL( Tok.pSource );

			const Ax::uint8 *const p = Tok.pSource->GetData().begin();
			AX_ASSERT_NOT_NULL( p );

			const Ax::uint8 *const s = p + Tok.Data.uOffset;
			const Ax::uint8 *const e = s + Tok.Data.cBytes - 1;

			const llvm::StringRef textData( ( const char * )s, ( size_t )( e - s ) );

			char szName[ 64 ];
			Ax::Format( szName, ".str.%u", CG->GetStringId() );

			llvm::Value *const pValuePre = CG->Builder().CreateGlobalStringPtr( textData, szName );
			AX_EXPECT_NOT_NULL( pValuePre );

			pValue = pValuePre;
		} else {
			AX_EXPECT_MSG( false, "Unhandled literal type" );
		}

		AX_EXPECT_MSG( pValue != nullptr, "Literal could not be generated for LLVM" );

		m_CodeGen.pValue = pValue;
		return m_CodeGen.pValue;
	}


	/*
	===========================================================================

		NAME EXPRESSION

	===========================================================================
	*/
	
	CNameExpr::CNameExpr( const SToken &Token, CParser &Parser )
	: CExpression( EExprType::Name, Token, Parser )
	, m_Semanted()
	{
		m_Semanted.pSym = nullptr;
	}
	CNameExpr::~CNameExpr()
	{
	}
	
	Ax::String CNameExpr::ToString() const
	{
		return Token().GetString();
	}
	const SSymbol *CNameExpr::GetSymbol() const
	{
		return m_Semanted.pSym;
	}
	const STypeRef *CNameExpr::GetType() const
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );

		if( m_Semanted.pSym->pVar != nullptr ) {
			return &m_Semanted.pSym->pVar->Type;
		}

		return nullptr;
	}

	bool CNameExpr::Semant()
	{
		const SToken &Tok = Token();

		if( Tok.IsKeyword( kKeyword_User ) ) {
			AX_ASSERT_NOT_NULL( Tok.pKeyword );
			AX_ASSERT_NOT_NULL( Tok.pKeyword->Data );

			m_Semanted.pSym = ( SSymbol * )Tok.pKeyword->Data;
		} else {
			m_Semanted.pSym = g_Prog->FindSymbol( Token().GetString() );
		}

		if( !m_Semanted.pSym ) {
			//
			//	TODO: In non-strict mode support creation of the symbol based on
			//	-     the name of the variable (a variable ending with "#" is a
			//	-     float; "$" a string; others are pointer-sized integers)
			//
			Token().Error( "Variable not found: \"" + Tok.GetString() + "\"" );
			return false;
		}

		if( !m_Semanted.pSym->pVar && !m_Semanted.pSym->pFunc && !m_Semanted.pSym->pLabel ) {
			Token().Error( "Symbol is not a variable, function, or label: \"" + Tok.GetString() + "\"" );
			return false;
		}

		return true;
	}
	SValue CNameExpr::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );

		if( !m_Semanted.pSym->Translated.pValue ) {
			SSymbol *const pSym = const_cast< SSymbol * >( m_Semanted.pSym );

			if( pSym->pFunc != nullptr ) {
				AX_ASSERT( !pSym->pFunc->Overloads.IsEmpty() );
				pSym->Translated.pValue = pSym->pFunc->Overloads.First()->pLLVMFunc;
			}
		}

		//
		//	TODO: In non-strict mode, create the symbol value
		//

		AX_ASSERT_NOT_NULL( m_Semanted.pSym->Translated.pValue );
		llvm::Value *const pVal = m_Semanted.pSym->Translated.pValue;

		const bool bIsVolatile = m_Semanted.pSym->pVar != nullptr ? m_Semanted.pSym->pVar->Type.IsVolatile() : false;

		return SValue( pVal, bIsVolatile );
	}


	/*
	===========================================================================

		MEMBER EXPRESSION

	===========================================================================
	*/

	CMemberExpr::CMemberExpr( const SToken &Tok, CParser &Parser )
	: CExpression( EExprType::Member, Tok, Parser )
	, m_pLHS( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}
	CMemberExpr::~CMemberExpr()
	{
	}
	
	Ax::String CMemberExpr::ToString() const
	{
		Ax::String Result;

		if( m_pLHS != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pLHS->ToString() ) );
		}

		AX_EXPECT_MEMORY( Result.Append( "." ) );
		AX_EXPECT_MEMORY( Result.Append( Token().GetString() ) );

		return Result;
	}
	const SSymbol *CMemberExpr::GetSymbol() const
	{
		return m_Semanted.pSym;
	}
	const STypeRef *CMemberExpr::GetType() const
	{
		if( m_Semanted.pSym != nullptr ) {
			if( m_Semanted.pSym->pVar != nullptr ) {
				return &m_Semanted.pSym->pVar->Type;
			}

			return nullptr;
		}

		return &m_Semanted.SwizzleType;
	}

	bool CMemberExpr::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLHS );

		if( !m_pLHS->Semant() ) {
			return false;
		}

		const STypeRef *const pType = m_pLHS->GetType();
		if( !pType ) {
			Token().Error( "Left-hand expression does not yield a type" );
			return false;
		}

		// Handle swizzling
		m_Semanted.cSwizzleMaxAxes = GetTypeColumns( pType->BuiltinType );
		if( m_Semanted.cSwizzleMaxAxes != 0 ) {
			const SToken &Tok = Token();

			const EBuiltinType ComponentType = GetVectorTypeComponentType( pType->BuiltinType );
			const EBuiltinType SwizzleType = GetVectorSwizzleType( ComponentType, ( unsigned )Tok.cLength );

			if( SwizzleType == EBuiltinType::Invalid ) {
				Tok.Error( "Swizzle operations must not exceed four dimensions" );
				return false;
			}

			// Get the number of swizzle axes
			m_Semanted.cSwizzleAxes = ( Ax::uint8 )Tok.cLength;
			AX_ASSERT( m_Semanted.cSwizzleAxes > 0 );
			AX_ASSERT( m_Semanted.cSwizzleAxes <= 4 );

			// Direct text pointer to the swizzle axes
			const char *const pszText = Tok.GetPointer();
			AX_ASSERT_NOT_NULL( pszText );

			// Parse the axes
			ESwizzleAxis v[4] = { axis_x,axis_x,axis_x,axis_x };
			for( Ax::uint8 i = 0; i < m_Semanted.cSwizzleAxes; ++i ) {
				if( pszText[i]=='x' || pszText[i]=='X' || pszText[i]=='r' || pszText[i]=='R' ) {
					v[ i ] = axis_x;
				} else if( pszText[i]=='y' || pszText[i]=='Y' || pszText[i]=='g' || pszText[i]=='G' ) {
					v[ i ] = axis_y;
				} else if( pszText[i]=='z' || pszText[i]=='Z' || pszText[i]=='b' || pszText[i]=='B' ) {
					v[ i ] = axis_z;
				} else if( pszText[i]=='w' || pszText[i]=='W' || pszText[i]=='a' || pszText[i]=='A' ) {
					v[ i ] = axis_w;
				} else {
					Tok.Error( "Invalid swizzle component \'" + Ax::String( pszText[i] ) + "\'; expected x,y,z,w or r,g,b,a" );
					return false;
				}
			}

			// Store the parsed swizzle mask
			m_Semanted.uSwizzleMask = MakeSwizzleMask( v[0], v[1], v[2], v[3] );

			// Get the axis type
			m_Semanted.SwizzleType.BuiltinType = SwizzleType;
			m_Semanted.SwizzleType.Access =
				SwizzleIsLValue( m_Semanted.uSwizzleMask, m_Semanted.cSwizzleAxes, m_Semanted.cSwizzleMaxAxes ) ?
				EAccess::ReadWrite :
				EAccess::ReadOnly;
			m_Semanted.SwizzleType.cBytes = GetTypeSize( m_Semanted.SwizzleType.BuiltinType );
			m_Semanted.SwizzleType.pCustomType = nullptr;
			m_Semanted.SwizzleType.pRef = nullptr;

			// Done
			return true;
		}

		// Handle user-defined-type (UDT) field access
		if( pType->BuiltinType != EBuiltinType::UserDefined ) {
			Token().Error( "Left-hand expression is not an user-defined-type: \"" + m_pLHS->ToString() + "\"" );
			return false;
		}

		AX_ASSERT_NOT_NULL( pType->pCustomType );
		AX_ASSERT_NOT_NULL( pType->pCustomType->pScope );

		m_Semanted.pSym = pType->pCustomType->pScope->FindSymbol( Token(), ESearchArea::ThisScopeOnly );
		if( !m_Semanted.pSym ) {
			Token().Error( "No field by the name of \"" + Token().GetString() + "\" in type \"" + pType->pCustomType->Name + "\"" );
			return false;
		}

		AX_ASSERT_NOT_NULL( m_Semanted.pSym->pVar );
		AX_ASSERT( m_Semanted.pSym->pVar->iFieldIndex >= 0 );

		return true;
	}
	SValue CMemberExpr::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pSym->pVar );
		AX_ASSERT( m_Semanted.pSym->pVar->iFieldIndex >= 0 );

		const SSymbol &Sym = *m_Semanted.pSym;

		llvm::LLVMContext &Context = CG->Context();
		llvm::IRBuilder<> &Builder = CG->Builder();
		llvm::Type *const pInt32Ty = llvm::Type::getInt32Ty( Context );

		llvm::Value *const Values[2] = {
			llvm::ConstantInt::get( pInt32Ty, llvm::APInt( 32, 0, false ) ),
			llvm::ConstantInt::get( pInt32Ty, llvm::APInt( 32, Sym.pVar->iFieldIndex, false ) )
		};

		llvm::ArrayRef< llvm::Value * > ValuesRef( Values );

		SValue LeftVal = m_pLHS->CodeGen();
		if( !LeftVal ) {
			return nullptr;
		}

		if( !LeftVal.IsAddress() ) {
			m_pLHS->Token().Error( "[CodeGen] Left value of member expression is not an l-value" );
			return nullptr;
		}

		llvm::Value *const pGEP = Builder.CreateGEP( LeftVal.pLLVMValue, ValuesRef, LLVMStr( Token() ) );
		if( !pGEP ) {
			Token().Error( "[CodeGen] GEP instruction creation failed" );
			return nullptr;
		}

		return SValue( pGEP, LeftVal.IsVolatile() );
	}


	/*
	===========================================================================

		ARRAY SUBSCRIPT EXPRESSION

	===========================================================================
	*/

	CArraySubscriptExpr::CArraySubscriptExpr( const SToken &Tok, CParser &Parser )
	: CExpression( EExprType::ArraySubscript, Tok, Parser )
	, m_pLHS( nullptr )
	, m_pList( nullptr )
	, m_Semanted()
	{
	}
	CArraySubscriptExpr::~CArraySubscriptExpr()
	{
		delete m_pLHS;
		m_pLHS = nullptr;

		delete m_pList;
		m_pList = nullptr;
	}

	const STypeRef *CArraySubscriptExpr::GetType() const
	{
		return &m_Semanted.Type;
	}
	Ax::String CArraySubscriptExpr::ToString() const
	{
		Ax::String Result;

		if( m_pLHS != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pLHS->ToString() ) );
		}

		AX_EXPECT_MEMORY( Result.Append( "[" ) );
		if( m_pList != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pList->ToString() ) );
		}
		AX_EXPECT_MEMORY( Result.Append( "]" ) );

		return Result;
	}


	bool CArraySubscriptExpr::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLHS );
		AX_ASSERT_NOT_NULL( m_pList );

		if( !m_pLHS->Semant() ) {
			return false;
		}
		if( !m_pList->Semant() ) {
			return false;
		}

		const STypeRef *const pLHSType = m_pLHS->GetType();
		if( !pLHSType ) {
			m_pLHS->Token().Error( "Left-hand expression has no associated type: \"" + m_pLHS->ToString() + "\"" );
			return false;
		}

		if( pLHSType->IsVector() ) {
			m_Semanted.Type.BuiltinType = GetVectorTypeComponentType( pLHSType->BuiltinType );
			m_Semanted.Type.pCustomType = nullptr;
			m_Semanted.Type.pRef = nullptr;
			m_Semanted.Type.Access = pLHSType->Access;
			m_Semanted.Type.bCanReorderMemAccesses = pLHSType->bCanReorderMemAccesses;
			m_Semanted.Type.cBytes = GetTypeSize( m_Semanted.Type.BuiltinType );
		} else if( pLHSType->IsArray() ) {
			AX_ASSERT_NOT_NULL( pLHSType->pRef );

			m_Semanted.Type = *pLHSType->pRef;
		} else {
			m_pLHS->Token().Error( "Left-hand expression is neither an array or vector for subscript: \"" + m_pLHS->ToString() + "\"" );
			return false;
		}

		//
		//	NOTE: An empty array subscript (e.g., "myarr[]") means
		//	-     "last element," and is valid.
		//
		//	If it weren't valid we'd do:
		//
		//		if( m_pList->Subexpressions().IsEmpty() ) {
		//			m_pList->Token().Error( "Not valid!" );
		//			return false;
		//		}
		//
		//	But with a more helpful error message.
		//

		for( const CExpression *pSubexpr : m_pList->Subexpressions() ) {
			AX_ASSERT_NOT_NULL( pSubexpr );

			const STypeRef *const pType = pSubexpr->GetType();
			if( !pType ) {
				pSubexpr->Token().Error( "Subscript did not yield type" );
				return false;
			}

			if( !IsIntNumber( pType->BuiltinType ) ) {
				pSubexpr->Token().Error( "Subscript is not integer" );
				return false;
			}
		}

		return true;
	}
	SValue CArraySubscriptExpr::CodeGen()
	{
		//
		//	TODO: Safety code (make sure no index exceeds its bounds)
		//

		AX_ASSERT_NOT_NULL( m_pLHS );
		AX_ASSERT( m_Semanted.Type.BuiltinType != EBuiltinType::Invalid );

		SValue LHSVal = m_pLHS->CodeGen();
		if( !LHSVal ) {
			return nullptr;
		}

		const STypeRef *const pLHSRTy = m_pLHS->GetType();
		AX_ASSERT_NOT_NULL( pLHSRTy );
		AX_ASSERT( pLHSRTy->BuiltinType == EBuiltinType::ArrayObject );

		llvm::Type *const pArrTy = const_cast< STypeRef * >( pLHSRTy )->CodeGen();
		if( !pArrTy ) {
			return nullptr;
		}

		llvm::Value *const pArr = CG->Builder().CreateCast( llvm::CastInst::getCastOpcode( LHSVal.pLLVMValue, false, pArrTy, false ), LHSVal.pLLVMValue, pArrTy );
		AX_ASSERT_NOT_NULL( pArr );

		// previous needed resolution
		llvm::Value *pPrevRes = nullptr;
		// current index calculation so far
		llvm::Value *pIndex = nullptr;

		const unsigned cSubexprs = ( unsigned )m_pList->Subexpressions().Num();

		const unsigned cBits = ( unsigned )g_Env->GetPointerBits();
		llvm::Type *const pUIntPtrTy = llvm::Type::getIntNTy( CG->Context(), cBits );

		// Calculate the index: Y*X*z + X*y + x
		for( uintptr i = 0; i < cSubexprs; ++i ) {
			SExpr &Subexpr = m_pList->Subexpressions()[ i ];
			AX_ASSERT_NOT_NULL( Subexpr );

			SValue Subval = Subexpr->CodeGen();
			if( !Subval ) {
				return nullptr;
			}

			if( i == 0 ) {
				pIndex = Subval.Load();
			} else {
				static const char *const pszNames[] = {
					"arr.res.x", "arr.res.y", "arr.res.z", "arr.res.w",
					"arr.res.4", "arr.res.5", "arr.res.6", "arr.res.7",
					"arr.res.8"
				};
				static const unsigned kNumNames = sizeof( pszNames )/sizeof( pszNames[ 0 ] );

				const unsigned u = ( unsigned )i - 1;

				llvm::Value *const pDimNum = llvm::Constant::getIntegerValue( pUIntPtrTy, llvm::APInt( cBits, ( uint64_t )u, false ) );
				const char *const pszName = u < kNumNames ? pszNames[ u ] : "";

				llvm::Value *const pArgs[] = {
					pArr,
					pDimNum
				};

				llvm::Value *const pDimRes = CG->Builder().CreateCall( CG->InternalFuncs().pArrayGetDimRes, pArgs, pszName );

				pPrevRes = pPrevRes != nullptr ? CG->Builder().CreateMul( pDimRes, pPrevRes ) : pDimRes;
				pIndex = CG->Builder().CreateAdd( pIndex, CG->Builder().CreateMul( Subval.Load(), pPrevRes ) );
			}
		}

		if( !cSubexprs ) {
			pIndex = CG->Builder().CreateCall( CG->InternalFuncs().pArrayGetCurIdx, pArr );
		}

		llvm::Value *const pElementPtr = CG->Builder().CreateGEP( pArr, pIndex );

		return SValue( SValue::kLValue, pElementPtr );
	}


	/*
	===========================================================================

		FUNCTION CALL EXPRESSION

	===========================================================================
	*/

	CFuncCallExpr::CFuncCallExpr( const SToken &Tok, CParser &Parser )
	: CExpression( EExprType::FunctionCall, Tok, Parser )
	, m_pLHS( nullptr )
	, m_pList( nullptr )
	, m_Semanted()
	{
		m_Semanted.pFuncOverload = nullptr;
	}
	CFuncCallExpr::~CFuncCallExpr()
	{
	}

	const STypeRef *CFuncCallExpr::GetType() const
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pFuncOverload );
		return &m_Semanted.pFuncOverload->ReturnInfo.Type;
	}
	Ax::String CFuncCallExpr::ToString() const
	{
		Ax::String Result;

		if( m_pLHS != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pLHS->ToString() ) );
		}

		AX_EXPECT_MEMORY( Result.Append( "(" ) );
		if( m_pList != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pList->ToString() ) );
		}
		AX_EXPECT_MEMORY( Result.Append( ")" ) );

		return Result;
	}

	bool CFuncCallExpr::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLHS );
		AX_ASSERT_NOT_NULL( m_pList );

		if( !m_pLHS->Semant() ) {
			return false;
		}

		if( !m_pList->Semant() ) {
			return false;
		}

		const SSymbol *const pLeftSym = m_pLHS->GetSymbol();
		if( !pLeftSym ) {
			m_pLHS->Token().Error( "No symbol found for function call to \"" + m_pLHS->ToString() + "\"" );
			return false;
		}

		if( pLeftSym->pFunc == nullptr ) {
			m_pLHS->Token().Error( "Symbol is not a function: \"" + m_pLHS->Token().GetString() + "\"" );
			return false;
		}

		m_Semanted.pFuncOverload = pLeftSym->pFunc->FindMatch( m_pList->Subexpressions(), m_Semanted.Casts );
		if( !m_Semanted.pFuncOverload ) {
			//
			//	FIXME: This is a terrible way to tell the user about this.
			//
			m_pLHS->Token().Error( "No matching call to function: \"" + m_pLHS->Token().GetString() + "\"" );
			return false;
		}

		return true;
	}
	SValue CFuncCallExpr::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pList );
		AX_ASSERT_NOT_NULL( m_Semanted.pFuncOverload );
		AX_ASSERT( m_Semanted.Casts.Num() == m_pList->Subexpressions().Num() );

		if( !m_Semanted.pFuncOverload->GenDecl() ) {
			Token().Error( "[CodeGen] Failed to generate declaration for function" );
			return nullptr;
		}
		AX_ASSERT_NOT_NULL( m_Semanted.pFuncOverload->pLLVMFunc );

		static const uintptr kMaxArgs = 64;
		llvm::Value *pArgs[ kMaxArgs ];
		uintptr cArgs = 0;

		//
		//	TODO: Refine argument passing. Rather than checking whether a
		//	-     parameter is a user defined type, check if the parameter takes
		//	-     a reference. Then choose between Subval.AddressOf() and
		//	-     Subval.Load().
		//

		for( SExpr Subexpr : m_pList->Subexpressions() ) {
			AX_ASSERT_NOT_NULL( Subexpr );
			AX_ASSERT( cArgs < kMaxArgs );
			AX_ASSERT( m_Semanted.Casts[ cArgs ] != ECast::Invalid );

			SValue Subval = Subexpr->CodeGen();
			if( !Subval ) {
				return nullptr;
			}

			const STypeRef *const pArgRTy = Subexpr->GetType();
			if( !pArgRTy ) {
				Subexpr->Token().Error( "[CodeGen] Argument has no type" );
				return nullptr;
			}

			if( m_Semanted.pFuncOverload->Parameters[ cArgs ].PassBy == EPassBy::Reference ) {
				if( m_Semanted.Casts[ cArgs ] == ECast::None ) {
					if( !Subval.IsAddress() ) {
						Subexpr->Token().Error( "Expected variable, not constant" );
						return nullptr;
					}
					pArgs[ cArgs ] = Subval.Address();
					AX_ASSERT_NOT_NULL( pArgs[ cArgs ] );
				} else {
					Subexpr->Token().Error( "Cannot cast references" );
					return nullptr;
				}
			} else {
				if( m_Semanted.Casts[ cArgs ] == ECast::None ) {
					pArgs[ cArgs ] = Subval.Load();
					if( !pArgs[ cArgs ] ) {
						return nullptr;
					}
				} else {
					pArgs[ cArgs ] = CG->EmitCast( m_Semanted.Casts[ cArgs ], m_Semanted.pFuncOverload->Parameters[ cArgs ].Type.BuiltinType, Subval.Load() );
					if( !pArgs[ cArgs ] ) {
						return nullptr;
					}
				}
			}

			++cArgs;
		}

		llvm::Value *const pVal =
			CG->Builder().CreateCall
			(
				m_Semanted.pFuncOverload->pLLVMFunc,
				llvm::ArrayRef< llvm::Value * >( pArgs, cArgs )
			);
		if( !pVal ) {
			return nullptr;
		}

		const STypeRef *const pType = GetType();
		if( !!pType && pType->BuiltinType == EBuiltinType::StringObject ) {
			CG->AddCleanCall( CG->InternalFuncs().pStrReclaim, pVal );
		}

		return pVal;
	}


	/*
	===========================================================================
	
		UNARY OPERATOR EXPRESSION

	===========================================================================
	*/
	CUnaryExpr::CUnaryExpr( const SToken &Token, CParser &Parser )
	: CExpression( EExprType::UnaryOp, Token, Parser )
	, m_Operator( EBuiltinOp::None )
	, m_bIsPostfix( false )
	, m_pSubexpr( nullptr )
	{
	}
	CUnaryExpr::~CUnaryExpr()
	{
		delete m_pSubexpr;
		m_pSubexpr = nullptr;
	}

	EBuiltinOp CUnaryExpr::Operator() const
	{
		return m_Operator;
	}
	bool CUnaryExpr::IsPostfix() const
	{
		return m_bIsPostfix;
	}

	Ax::String CUnaryExpr::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "(" ) );

		if( !m_bIsPostfix ) {
			AX_EXPECT_MEMORY( Result.Append( BuiltinOpToString( m_Operator ) ) );
		}
		if( m_pSubexpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pSubexpr->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(Expr:null)" ) );
		}
		if( m_bIsPostfix ) {
			AX_EXPECT_MEMORY( Result.Append( BuiltinOpToString( m_Operator ) ) );
			AX_EXPECT_MEMORY( Result.Append( " [[postfix]]" ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( " [[prefix]]" ) );
		}

		AX_EXPECT_MEMORY( Result.Append( ")" ) );

		return Result;
	}
	const STypeRef *CUnaryExpr::GetType() const
	{
		AX_ASSERT_NOT_NULL( m_pSubexpr );

		return &m_Semanted.Type;
	}

	bool CUnaryExpr::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pSubexpr );

		if( !m_pSubexpr->Semant() ) {
			return false;
		}

		const STypeRef *const pType = m_pSubexpr->GetType();
		if( !pType && m_Operator != EBuiltinOp::Addr ) {
			m_pSubexpr->Token().Error( "No type for subexpression of unary operation" );
			return false;
		}

		m_Semanted.RHSCast = ECast::None;

		switch( m_Operator )
		{
		case EBuiltinOp::Neg:
			if( !IsNumber( pType->BuiltinType ) ) {
				Token().Error( "Negation only applies to numbers" );
				return false;
			}

			m_Semanted.Type = *pType;
			m_Semanted.Type.Access = EAccess::ReadOnly;
			break;

		case EBuiltinOp::Deref:
			if( !IsIntNumber( pType->BuiltinType ) ) {
				Token().Error( "Dereferencing non-integer" );
				return false;
			}

			//
			//	NOTE: The dereferenced value should be treated as whatever type
			//	-     it is being used as. e.g., *x = 1.23f would treat "*x" as
			//	-     a float32. While *x = uintptr( 1 ) would treat "*x" as an
			//	-     uintptr.
			//

			m_Semanted.Type.BuiltinType = EBuiltinType::Any;
			m_Semanted.Type.pCustomType = nullptr;
			m_Semanted.Type.Access = EAccess::ReadWrite;
			m_Semanted.Type.bCanReorderMemAccesses = false;
			m_Semanted.Type.cBytes = 0;
			m_Semanted.Type.pRef = nullptr;
			break;

		case EBuiltinOp::Addr:
			m_Semanted.Type.BuiltinType = g_Env->GetUIntPtrType();
			m_Semanted.Type.pCustomType = nullptr;
			m_Semanted.Type.Access = EAccess::ReadOnly;
			m_Semanted.Type.bCanReorderMemAccesses = true;
			m_Semanted.Type.cBytes = GetTypeSize( m_Semanted.Type.BuiltinType );
			m_Semanted.Type.pRef = nullptr;
			break;

		case EBuiltinOp::RelNot:
			m_Semanted.Type.BuiltinType = EBuiltinType::Boolean;
			m_Semanted.Type.pCustomType = nullptr;
			m_Semanted.Type.Access = EAccess::ReadOnly;
			m_Semanted.Type.bCanReorderMemAccesses = true;
			m_Semanted.Type.cBytes = GetTypeSize( m_Semanted.Type.BuiltinType );
			m_Semanted.Type.pRef = nullptr;

			m_Semanted.RHSCast = GetCastForTypes( pType->BuiltinType, EBuiltinType::Boolean, ECastMode::Explicit );
			if( m_Semanted.RHSCast == ECast::Invalid ) {
				return pType->CastError( Token(), *pType, m_Semanted.Type );
			}

			break;

		default:
			AX_ASSERT_MSG( false, "Unreachable (Invalid unary built-in op)" );
			Token().Error( "[INTERNAL-ERROR] Invalid unary built-in op" );
			return false;
		}

		return true;
	}
	SValue CUnaryExpr::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pSubexpr );
		AX_ASSERT( m_Semanted.RHSCast != ECast::Invalid );

		SValue SrcVal = m_pSubexpr->CodeGen();
		if( !SrcVal ) {
			return nullptr;
		}

		if( m_Operator == EBuiltinOp::Deref ) {
			Token().Error( "[CodeGen] Dereference not yet supported" );
			return nullptr;
		}
		if( m_Operator == EBuiltinOp::Addr ) {
			if( !SrcVal.IsAddress() ) {
				Token().Error( "[CodeGen] Attempted to retrieve address of r-value" );
				return nullptr;
			}

			llvm::Value *const pAddr = SrcVal.Address();
			AX_ASSERT_NOT_NULL( pAddr );
			
			llvm::Type *const pUIntPtrTy = llvm::Type::getIntNTy( CG->Context(), g_Env->GetPointerSizeInBytes()*8 );
			return CG->Builder().CreateCast( llvm::CastInst::getCastOpcode( pAddr, false, pUIntPtrTy, false ), pAddr, pUIntPtrTy );
		}

		llvm::Value *const pSrcValPre = SrcVal.Load();
		AX_ASSERT_NOT_NULL( pSrcValPre );

		llvm::Value *const pSrcVal = CG->EmitCast( m_Semanted.RHSCast, m_Semanted.Type.BuiltinType, pSrcValPre );
		if( !pSrcVal ) {
			return nullptr;
		}

		switch( m_Operator ) {
		case EBuiltinOp::Neg:
			if( IsIntNumber( m_Semanted.Type.BuiltinType ) ) {
				return CG->Builder().CreateNeg( pSrcVal );
			}
			if( IsRealNumber( m_Semanted.Type.BuiltinType ) ) {
				return CG->Builder().CreateFNeg( pSrcVal );
			}

			AX_ASSERT_MSG( false, "Unreachable (Corrupt unary operation)" );
			Token().Error( "[INTERNAL-ERROR] Non-number type for negate operation" );
			return nullptr;

		case EBuiltinOp::RelNot:
			return CG->Builder().CreateXor( pSrcVal, 1 );

		default:
			break;
		}
		
		AX_ASSERT_MSG( false, "Unreachable (Invalid unary built-in op)" );
		Token().Error( "[INTERNAL-ERROR] Invalid unary built-in op" );
		return nullptr;
	}

	/*
	===========================================================================
	
		BINARY OPERATOR EXPRESSION

	===========================================================================
	*/
	CBinaryExpr::CBinaryExpr( const SToken &Token, CParser &Parser )
	: CExpression( EExprType::BinaryOp, Token, Parser )
	, m_Operator( EBuiltinOp::None )
	, m_pLHS( nullptr )
	, m_pRHS( nullptr )
	, m_Semanted()
	, m_CodeGen()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
		memset( &m_CodeGen, 0, sizeof( m_CodeGen ) );
	}
	CBinaryExpr::~CBinaryExpr()
	{
		delete m_pLHS;
		m_pLHS = nullptr;

		delete m_pRHS;
		m_pRHS = nullptr;
	}

	EBuiltinOp CBinaryExpr::Operator() const
	{
		return m_Operator;
	}

	Ax::String CBinaryExpr::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "(" ) );
		AX_EXPECT_MEMORY( Result.Append( BuiltinOpToString( m_Operator ) ) );

		AX_EXPECT_MEMORY( Result.Append( " " ) );
		if( m_pLHS != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pLHS->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(LHSExpr:null)" ) );
		}

		AX_EXPECT_MEMORY( Result.Append( " " ) );
		if( m_pRHS != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pRHS->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(RHSExpr:null)" ) );
		}

		AX_EXPECT_MEMORY( Result.Append( ")" ) );

		return Result;
	}
	const STypeRef *CBinaryExpr::GetType() const
	{
		return &m_Semanted.ResultType;
	}

	bool CBinaryExpr::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLHS );
		AX_ASSERT_NOT_NULL( m_pRHS );

		if( !m_pLHS->Semant() ) {
			return false;
		}

		if( !m_pRHS->Semant() ) {
			return false;
		}

		const STypeRef *const pLHSType = m_pLHS->GetType();
		if( !pLHSType ) {
			m_pLHS->Token().Error( "Left-hand expression to binary operator ('" + Token().GetString() + "') has no type" );
			return false;
		}

		const STypeRef *const pRHSType = m_pRHS->GetType();
		if( !pRHSType ) {
			m_pRHS->Token().Error( "Right-hand expression to binary operator ('" + Token().GetString() + "') has no type" );
			return false;
		}

		if( pLHSType->BuiltinType == EBuiltinType::UserDefined ) {
			m_pLHS->Token().Error( "Operators on user-defined-types are not supported" );
			return false;
		}
		if( pRHSType->BuiltinType == EBuiltinType::UserDefined ) {
			m_pRHS->Token().Error( "Operators on user-defined-types are not supported" );
			return false;
		}

		EBuiltinType DstType = EBuiltinType::Invalid;
		EBuiltinType PromotionType = EBuiltinType::Invalid;

		if( IsRelOp( m_Operator ) ) {
			DstType = EBuiltinType::Boolean;
			PromotionType = EBuiltinType::Boolean;
		} else {
			PromotionType = FindTypePromotion( pLHSType->BuiltinType, pRHSType->BuiltinType );
			if( PromotionType == EBuiltinType::Invalid ) {
				Token().Error( "Incompatible types (no type promotion found) to binary operator ('" + Token().GetString() + "')" );
				return false;
			}
		}

		DstType = IsCmpOp( m_Operator ) ? EBuiltinType::Boolean : PromotionType;

		AX_ASSERT( DstType != EBuiltinType::Invalid );

		m_Semanted.ResultType.BuiltinType = DstType;
		m_Semanted.ResultType.pCustomType = nullptr;
		m_Semanted.ResultType.Access = EAccess::ReadOnly;
		m_Semanted.ResultType.bCanReorderMemAccesses = pLHSType->bCanReorderMemAccesses || pRHSType->bCanReorderMemAccesses;
		m_Semanted.ResultType.pRef = nullptr;
		m_Semanted.ResultType.cBytes = GetTypeSize( DstType );
		
		m_Semanted.PromotionType = PromotionType;

		m_Semanted.LHSCast = GetCastForTypes( pLHSType->BuiltinType, PromotionType, ECastMode::Input );
		m_Semanted.RHSCast = GetCastForTypes( pRHSType->BuiltinType, PromotionType, ECastMode::Input );

		if( m_Semanted.LHSCast == ECast::Invalid ) {
			m_pLHS->Token().Error( "Cannot cast from \"" + pLHSType->ToString() + "\" to \"" + m_Semanted.ResultType.ToString() + "\"" );
			return false;
		}

		if( IsString( m_Semanted.ResultType.BuiltinType ) ) {
			bool bIsValidOperator = false;
			switch( m_Operator )
			{
			case EBuiltinOp::Add:
				m_Operator = EBuiltinOp::StrConcat;
				bIsValidOperator = true;
				break;

			case EBuiltinOp::Sub:
				m_Operator = EBuiltinOp::StrRemove;
				if( IsIntNumber( pRHSType->BuiltinType ) ) {
					m_Semanted.RHSCast = GetCastForTypes( pRHSType->BuiltinType, g_Env->GetUIntPtrType(), ECastMode::Input );
				}
				bIsValidOperator = true;
				break;

			case EBuiltinOp::Mul:
				m_Operator = EBuiltinOp::StrRepeat;
				m_Semanted.RHSCast = GetCastForTypes( pRHSType->BuiltinType, g_Env->GetUIntPtrType(), ECastMode::Input );
				bIsValidOperator = true;
				break;

			case EBuiltinOp::Div:
				m_Operator = EBuiltinOp::StrAddPath;
				bIsValidOperator = true;
				break;
			}

			if( !bIsValidOperator ) {
				Token().Error( "Invalid operator ('" + Token().GetString() + "') for strings" );
				return false;
			}
		}

		if( m_Semanted.RHSCast == ECast::Invalid ) {
			m_pRHS->Token().Error( "Cannot cast from \"" + pRHSType->ToString() + "\" to \"" + m_Semanted.ResultType.ToString() + "\"" );
			return false;
		}

		return true;
	}
	SValue CBinaryExpr::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pLHS );
		AX_ASSERT_NOT_NULL( m_pRHS );

		AX_ASSERT( m_Semanted.LHSCast != ECast::Invalid );
		AX_ASSERT( m_Semanted.RHSCast != ECast::Invalid );
		AX_ASSERT( m_Semanted.ResultType.BuiltinType != EBuiltinType::Invalid );

		const EBuiltinType LHSType = m_Semanted.PromotionType != EBuiltinType::Invalid ?
										m_Semanted.PromotionType :
										m_Semanted.ResultType.BuiltinType;
		const EBuiltinType RHSType = m_Operator == EBuiltinOp::StrRepeat ? g_Env->GetUIntPtrType() : LHSType;

		const bool bIsRelOp = IsRelOp( m_Operator );

		llvm::BasicBlock *const pLHSEval = &CG->CurrentBlock();
		llvm::BasicBlock *const pRHSEval = bIsRelOp ? &CG->DeclareLabel() : nullptr;
		llvm::BasicBlock *const pEndEval = bIsRelOp ? &CG->DeclareLabel() : nullptr;

		SValue LHSValPre = m_pLHS->CodeGen();
		if( !LHSValPre ) {
			return nullptr;
		}

		llvm::Value *const pLHSVal = CG->EmitCast( m_Semanted.LHSCast, LHSType, LHSValPre.Load() );
		AX_EXPECT_MEMORY( pLHSVal );

		if( bIsRelOp ) {
			if( m_Operator == EBuiltinOp::RelAnd ) {
				CG->Builder().CreateCondBr( pLHSVal, pRHSEval, pEndEval );
			} else if( m_Operator == EBuiltinOp::RelOr ) {
				CG->Builder().CreateCondBr( pLHSVal, pEndEval, pRHSEval );
			}

			CG->SetCurrentBlock( *pRHSEval );
		}

		SValue RHSValPre = m_pRHS->CodeGen();
		if( !RHSValPre ) {
			return nullptr;
		}

		llvm::Value *const pRHSVal = CG->EmitCast( m_Semanted.RHSCast, RHSType, RHSValPre.Load() );
		AX_EXPECT_MEMORY( pRHSVal );

		llvm::Value *pResultVal = nullptr;

		llvm::Value *pArgs[] = { pLHSVal, pRHSVal };

		if( bIsRelOp ) {
			CG->SetCurrentBlock( *pEndEval );

			llvm::Type *const pBoolTy = llvm::Type::getInt1Ty( CG->Context() );
			llvm::Value *const pValue = llvm::ConstantInt::get( pBoolTy, ( uint64_t )+( m_Operator == EBuiltinOp::RelOr ) );

			const char *const pPHIName = m_Operator == EBuiltinOp::RelAnd ? "and" : "or";
			llvm::PHINode *const pPHI = CG->Builder().CreatePHI( pBoolTy, 2, pPHIName );

			AX_EXPECT_MEMORY( pPHI );

			pPHI->addIncoming( pValue, pLHSEval );
			pPHI->addIncoming( pRHSVal, pRHSEval );

			return pPHI;
		}

		const bool bIsInt = IsIntNumber( LHSType );
		const bool bIsFloat = IsRealNumber( LHSType );

		const bool bIsSigned = IsSigned( LHSType );

		switch( m_Operator )
		{
		case EBuiltinOp::Add:
			if( bIsInt ) {
				pResultVal = CG->Builder().CreateAdd( pLHSVal, pRHSVal, "addtmp" );
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFAdd( pLHSVal, pRHSVal, "f_addtmp" );
			}
			break;
		case EBuiltinOp::Sub:
			if( bIsInt ) {
				pResultVal = CG->Builder().CreateSub( pLHSVal, pRHSVal, "subtmp" );
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFSub( pLHSVal, pRHSVal, "f_subtmp" );
			}
			break;
		case EBuiltinOp::Mul:
			if( bIsInt ) {
				pResultVal = CG->Builder().CreateMul( pLHSVal, pRHSVal, "multmp" );
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFMul( pLHSVal, pRHSVal, "f_multmp" );
			}
			break;
		case EBuiltinOp::Div:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateSDiv( pLHSVal, pRHSVal, "divtmp" );
				} else {
					pResultVal = CG->Builder().CreateUDiv( pLHSVal, pRHSVal, "divtmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFDiv( pLHSVal, pRHSVal );
			}
			break;
		case EBuiltinOp::Mod:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateSRem( pLHSVal, pRHSVal, "modtmp" );
				} else {
					pResultVal = CG->Builder().CreateURem( pLHSVal, pRHSVal, "modtmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFRem( pLHSVal, pRHSVal, "f_modtmp" );
			}
			break;

		case EBuiltinOp::BitAnd:
			pResultVal = CG->Builder().CreateAnd( pLHSVal, pRHSVal, "bitandtmp" );
			break;
		case EBuiltinOp::BitOr:
			pResultVal = CG->Builder().CreateOr( pLHSVal, pRHSVal, "bitortmp" );
			break;
		case EBuiltinOp::BitXor:
			pResultVal = CG->Builder().CreateXor( pLHSVal, pRHSVal, "bitxortmp" );
			break;
		case EBuiltinOp::BitSL:
			pResultVal = CG->Builder().CreateShl( pLHSVal, pRHSVal, "bitsltmp" );
			break;
		case EBuiltinOp::BitSR:
			if( bIsSigned ) {
				pResultVal = CG->Builder().CreateAShr( pLHSVal, pRHSVal, "bitsrtmp" );
			} else {
				pResultVal = CG->Builder().CreateLShr( pLHSVal, pRHSVal, "bitsrtmp" );
			}
			break;

		case EBuiltinOp::StrConcat:
			pResultVal = CG->Builder().CreateCall( CG->InternalFuncs().pStrConcat, pArgs, "strcattmp" );
			CG->AddCleanCall( CG->InternalFuncs().pStrReclaim, pResultVal );
			break;
		case EBuiltinOp::StrRemove:
			pResultVal = CG->Builder().CreateCall( CG->InternalFuncs().pStrFindRm, pArgs, "strremtmp" );
			CG->AddCleanCall( CG->InternalFuncs().pStrReclaim, pResultVal );
			break;
		case EBuiltinOp::StrRepeat:
			pResultVal = CG->Builder().CreateCall( CG->InternalFuncs().pStrRepeat, pArgs, "strreptmp" );
			CG->AddCleanCall( CG->InternalFuncs().pStrReclaim, pResultVal );
			break;
		case EBuiltinOp::StrAddPath:
			pResultVal = CG->Builder().CreateCall( CG->InternalFuncs().pStrCatDir, pArgs, "strdirtmp" );
			CG->AddCleanCall( CG->InternalFuncs().pStrReclaim, pResultVal );
			break;

		case EBuiltinOp::CmpEq:
			if( bIsInt ) {
				pResultVal = CG->Builder().CreateICmpEQ( pLHSVal, pRHSVal, "icmpeqtmp" );
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpOEQ( pLHSVal, pRHSVal, "fcmpeqtmp" );
			}
			// TODO: String
			break;
		case EBuiltinOp::CmpNe:
			if( bIsInt ) {
				pResultVal = CG->Builder().CreateICmpNE( pLHSVal, pRHSVal, "icmpnetmp" );
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpONE( pLHSVal, pRHSVal, "fcmpnetmp" );
			}
			// TODO: String
			break;
		case EBuiltinOp::CmpLt:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateICmpSLT( pLHSVal, pRHSVal, "icmpslttmp" );
				} else {
					pResultVal = CG->Builder().CreateICmpULT( pLHSVal, pRHSVal, "icmpulttmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpOLT( pLHSVal, pRHSVal, "fcmplttmp" );
			}
			// TODO: String
			break;
		case EBuiltinOp::CmpLe:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateICmpSLE( pLHSVal, pRHSVal, "icmpsletmp" );
				} else {
					pResultVal = CG->Builder().CreateICmpULE( pLHSVal, pRHSVal, "icmpuletmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpOLE( pLHSVal, pRHSVal, "fcmpletmp" );
			}
			// TODO: String
			break;
		case EBuiltinOp::CmpGt:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateICmpSGT( pLHSVal, pRHSVal, "icmpsgttmp" );
				} else {
					pResultVal = CG->Builder().CreateICmpUGT( pLHSVal, pRHSVal, "icmpugttmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpOGT( pLHSVal, pRHSVal, "fcmpgttmp" );
			}
			// TODO: String
			break;
		case EBuiltinOp::CmpGe:
			if( bIsInt ) {
				if( bIsSigned ) {
					pResultVal = CG->Builder().CreateICmpSGE( pLHSVal, pRHSVal, "icmpsgetmp" );
				} else {
					pResultVal = CG->Builder().CreateICmpUGE( pLHSVal, pRHSVal, "icmpugetmp" );
				}
			} else if( bIsFloat ) {
				pResultVal = CG->Builder().CreateFCmpOGE( pLHSVal, pRHSVal, "fcmpgetmp" );
			}
			// TODO: String
			break;
		}

		AX_ASSERT_NOT_NULL( pResultVal );
		
		m_CodeGen.pValue = pResultVal;
		return m_CodeGen.pValue;
	}

}}

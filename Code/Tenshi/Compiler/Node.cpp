#include "_PCH.hpp"
#include "Node.hpp"
#include "Parser.hpp"
#include "TypeInformation.hpp"
#include "Environment.hpp"
#include "Program.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {
	
	llvm::Value *SValue::Address()
	{
		AX_ASSERT_NOT_NULL( pLLVMValue );

		if( Kind == kLValue ) {
			return pLLVMValue;
		}

		return nullptr;
	}
	llvm::Value *SValue::Load()
	{
		AX_ASSERT_NOT_NULL( pLLVMValue );

		if( Kind == kRValue ) {
			return pLLVMValue;
		}

		AX_ASSERT( Kind == kLValue );
		return CG->Builder().CreateLoad( pLLVMValue, bIsVolatile );
	}
	void SValue::Store( SValue FromVal )
	{
		AX_ASSERT_NOT_NULL( pLLVMValue );
		AX_ASSERT( IsAddress() );

		CG->Builder().CreateStore( FromVal.Load(), pLLVMValue, bIsVolatile );
	}

	/*
	===========================================================================

		STATEMENT SEQUENCE

	===========================================================================
	*/
	
	CStatementSequence::CStatementSequence()
	: m_Statements()
	, m_Type( EStmtSeqType::None )
	, m_bInFunction( false )
	, m_bInCase( false )
	, m_bInLoop( false )
	, m_pFunction( nullptr )
	, m_pScope( nullptr )
	{
	}
	CStatementSequence::~CStatementSequence()
	{
		for( CStatement *&pStmt : m_Statements ) {
			delete pStmt;
			pStmt = nullptr;
		}
	}

	void CStatementSequence::SetType( EStmtSeqType Type )
	{
		m_Type = Type;

		if( Type  == EStmtSeqType::Function ) {
			m_bInFunction = true;
		} else if( Type == EStmtSeqType::CaseBlock ) {
			m_bInCase = true;
		} else if( Type == EStmtSeqType::LoopBlock ) {
			m_bInLoop = true;
		}
	}
	EStmtSeqType CStatementSequence::Type() const
	{
		return m_Type;
	}
	bool CStatementSequence::Is( EStmtSeqType Type ) const
	{
		return m_Type == Type;
	}

	void CStatementSequence::Inherit( const CStatementSequence &OtherSeq )
	{
		m_bInFunction |= OtherSeq.m_bInFunction;
		m_bInCase |= OtherSeq.m_bInCase;
		m_bInLoop |= OtherSeq.m_bInLoop;

		if( m_Type == EStmtSeqType::Function || m_pFunction != nullptr ) {
			m_bInFunction = true;
		}
		if( m_Type == EStmtSeqType::CaseBlock ) {
			m_bInCase = true;
		}
		if( m_Type == EStmtSeqType::LoopBlock ) {
			m_bInLoop = true;
		}
	}
	bool CStatementSequence::InFunction() const
	{
		return m_bInFunction;
	}
	bool CStatementSequence::InCase() const
	{
		return m_bInCase;
	}
	bool CStatementSequence::InLoop() const
	{
		return m_bInLoop;
	}

	void CStatementSequence::SetFunction( SSymbol *pSym )
	{
		m_pFunction = pSym;
	}
	void CStatementSequence::SetScope( CScope *pScope )
	{
		m_pScope = pScope;
	}
	SSymbol *CStatementSequence::FunctionSym()
	{
		return m_pFunction;
	}
	CScope *CStatementSequence::Scope()
	{
		return m_pScope;
	}
	const SSymbol *CStatementSequence::FunctionSym() const
	{
		return m_pFunction;
	}
	const CScope *CStatementSequence::Scope() const
	{
		return m_pScope;
	}

	CStatement **CStatementSequence::begin()
	{
		return m_Statements.begin();
	}
	CStatement **CStatementSequence::end()
	{
		return m_Statements.end();
	}
	const CStatement *const *CStatementSequence::begin() const
	{
		return m_Statements.begin();
	}
	const CStatement *const *CStatementSequence::end() const
	{
		return m_Statements.end();
	}
	
	CStatement &CStatementSequence::AddStmt( CStatement &NewStmt )
	{
		AX_EXPECT_MEMORY( m_Statements.Append( &NewStmt ) );
		NewStmt.SetSequence( *this );
		return NewStmt;
	}

	Ax::String CStatementSequence::ToString() const
	{
		Ax::String Result;

		Result.Reserve( m_Statements.Num()*128 );
		
		AX_EXPECT_MEMORY( Result.Append( "{\n" ) );

		for( const CStatement *const &pStmt : m_Statements ) {
			if( !pStmt ) {
				AX_EXPECT_MEMORY( Result.Append( "\t(null-statement)\n" ) );
				continue;
			}

			const CStatement &Stmt = *pStmt;

			Ax::String TempResult = Stmt.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );
		}

		AX_EXPECT_MEMORY( Result.Append( "}" ) );

		return Result;
	}

	bool CStatementSequence::Semant()
	{
		for( CStatement *const &pStmt : m_Statements ) {
			AX_ASSERT_NOT_NULL( pStmt );

			pStmt->Parser().PushErrorToken( pStmt->Token() );
			const bool bResult = pStmt->Semant();
			pStmt->Parser().PopErrorToken();

			if( !bResult ) {
				return false;
			}
		}
		
		return true;
	}
	bool CStatementSequence::CodeGen()
	{
		for( CStatement *const &pStmt : m_Statements ) {
			AX_ASSERT_NOT_NULL( pStmt );

			pStmt->Parser().PushErrorToken( pStmt->Token() );
			const bool bResult = pStmt->CodeGen();
			pStmt->Parser().PopErrorToken();

			if( !bResult ) {
				return false;
			}
		}
		
		return true;
	}

	CParser &CStatement::Parser()
	{
		return m_Parser;
	}
	const CParser &CStatement::Parser() const
	{
		return m_Parser;
	}

	CLexer &CStatement::Lexer()
	{
		return m_Parser.Lexer();
	}
	const CLexer &CStatement::Lexer() const
	{
		return m_Parser.Lexer();
	}

	/*
	===========================================================================

		STATEMENT

	===========================================================================
	*/
	
	CStatement::CStatement( EStmtType Type, const SToken &Token, CParser &Parser )
	: m_Type( Type )
	, m_Token( Token )
	, m_Parser( Parser )
	{
	}
	CStatement::~CStatement()
	{
	}

	EStmtType CStatement::Type() const
	{
		return m_Type;
	}
	
	bool CStatement::IsSequenceSet() const
	{
		return m_pSequence != nullptr;
	}
	void CStatement::SetSequence( CStatementSequence &Seq )
	{
		m_pSequence = &Seq;
	}
	CStatementSequence &CStatement::GetSequence() const
	{
		AX_ASSERT_NOT_NULL( m_pSequence );
		return *m_pSequence;
	}

	Ax::String CStatement::ToString() const
	{
		switch( m_Type )
		{
		case EStmtType::None:							return "None";
			
		case EStmtType::LabelDecl:						return "LabelDecl";
		case EStmtType::VarDecl:						return "VarDecl";

		case EStmtType::ExitStmt:						return "ExitStmt";
		case EStmtType::ExitFunctionStmt:				return "ExitFunctionStmt";
		case EStmtType::ReturnStmt:						return "ReturnStmt";

		case EStmtType::AssignVar:						return "AssignVar";
		case EStmtType::InvokeFunction:					return "InvokeFunction";

		case EStmtType::IfBlock:						return "IfBlock";
		case EStmtType::SelectBlock:					return "SelectBlock";
		case EStmtType::CaseStmt:						return "CaseStmt";

		case EStmtType::DoLoopBlock:					return "DoLoopBlock";
		case EStmtType::WhileBlock:						return "WhileBlock";
		case EStmtType::RepeatBlock:					return "RepeatBlock";
		case EStmtType::ForNextBlock:					return "ForNextBlock";

		case EStmtType::GotoStmt:                       return "GotoStmt";
		case EStmtType::FallthroughStmt:                return "FallthroughStmt";
		}

		return "(unknown)";
	}

	bool CStatement::Semant()
	{
		return true;
	}
	bool CStatement::CodeGen()
	{
		return true;
	}

	
	/*
	===========================================================================

		EXPRESSION

	===========================================================================
	*/

	CExpression::CExpression( EExprType Type, const SToken &Token, CParser &Parser )
	: m_Type( Type )
	, m_Token( Token )
	, m_Parser( Parser )
	, m_pParent( nullptr )
	{
	}
	CExpression::~CExpression()
	{
	}

	CLexer &CExpression::Lexer()
	{
		return m_Parser.Lexer();
	}
	const CLexer &CExpression::Lexer() const
	{
		return m_Parser.Lexer();
	}
	CParser &CExpression::Parser()
	{
		return m_Parser;
	}

	void CExpression::SetParent( CExpression *pParent )
	{
		AX_ASSERT_IS_NULL( m_pParent );
		m_pParent = pParent;
	}
	
	static const char *ExprTypeToString( EExprType Type )
	{
		switch( Type )
		{
		case EExprType::None:				return "None";

		case EExprType::ExpressionList:		return "ExprList";
		case EExprType::Literal:			return "Literal";
		case EExprType::Name:				return "Name";
		case EExprType::Member:				return "Member";
		case EExprType::ArraySubscript:		return "ArraySubscript";
		case EExprType::FunctionCall:		return "FunctionCall";
		case EExprType::UnaryOp:			return "UnaryOp";
		case EExprType::BinaryOp:			return "BinaryOp";
		}

		static Ax::String Tmp;
		if( !Tmp.Format( "(unknown:%u)", (unsigned int)Type ) ) {
			return "(unknown)";
		}
		return Tmp;
	}
	Ax::String CExpression::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "(Expr:" ) );
		AX_EXPECT_MEMORY( Result.Append( ExprTypeToString( m_Type ) ) );
		AX_EXPECT_MEMORY( Result.Append( ")" ) );

		return Result;
	}
	const SSymbol *CExpression::GetSymbol() const
	{
		return nullptr;
	}
	const STypeRef *CExpression::GetType() const
	{
		return nullptr;
	}
	bool CExpression::IsLValue() const
	{
		const STypeRef *const pType = GetType();
		if( !pType ) {
			return false;
		}

		if( pType->Access == EAccess::ReadOnly ) {
			return false;
		}

		return true;
	}

	bool CExpression::Semant()
	{
		m_Token.Error( "INTERNAL ERROR: CExpression::Semant() not overloaded" );
		return false;
	}
	SValue CExpression::CodeGen()
	{
		m_Token.Error( "INTERNAL ERROR: CExpression::CodeGen() not overloaded" );
		return nullptr;
	}


	/*
	===========================================================================

		TYPE DECLARATIONS / PARTICLES

	===========================================================================
	*/
	
	CTypeDecl::CTypeDecl( const SToken &Tok, CParser &Parser )
	: m_Token( Tok )
	, m_Parser( Parser )
	, m_Lexer( Parser.Lexer() )
	, m_Chain()
	, m_pRef( nullptr )
	{
	}
	CTypeDecl::~CTypeDecl()
	{
	}

	bool CTypeDecl::Parse()
	{
		bool bHaveName = false;

		for( const SToken *pTok = &m_Token; pTok != nullptr; pTok = !!m_Lexer.LexLine() ? &m_Lexer.Token() : nullptr ) {
			const SToken &Tok = *pTok;

			if( Tok.IsName() ) {
				if( bHaveName ) {
					break;
				}

				if( Tok.IsKeyword() ) {
					AX_ASSERT_NOT_NULL( Tok.pKeyword );

					const auto Ident = Tok.pKeyword->Identifier;

					if( Ident < kKeyword_Type_Start__ || Ident >= kKeyword_Type_End__ ) {
						Tok.Error( "Unexpected keyword for type name" );
						return false;
					}
				}

				STypeParticle &Particle = AllocParticle();
				Particle.SetTypeName( pTok );

				bHaveName = true;
				continue;
			}

			if( m_Chain.IsEmpty() ) {
				Tok.Error( "Expected a type name" );
				return false;
			}

			if( Tok.IsPunctuation( "[" ) ) {
				if( !m_Lexer.Expect( Ax::Parser::ETokenType::Punctuation, "]" ) ) {
					return false;
				}

				STypeParticle &Particle = AllocParticle();
				Particle.SetArray();
				continue;
			}

			m_Lexer.Unlex();
			break;
		}

		AX_ASSERT( m_Chain.Num() > 0 );
		AX_ASSERT( bHaveName );

		return true;
	}
	bool CTypeDecl::ParseOne()
	{
		const SToken &Tok = m_Token;

		if( !Tok.IsName() ) {
			return false;
		}

		if( Tok.IsKeyword() ) {
			AX_ASSERT_NOT_NULL( Tok.pKeyword );

			const auto Ident = Tok.pKeyword->Identifier;

			if( Ident < kKeyword_Type_Start__ || Ident >= kKeyword_Type_End__ ) {
				Tok.Error( "Unexpected keyword for type name" );
				return false;
			}
		}

		STypeParticle &Particle = AllocParticle();
		Particle.SetTypeName( &Tok );

		return true;
	}
	
	Ax::String CTypeDecl::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "TypeDecl<" ) );

		for( const STypeParticle &Particle : m_Chain ) {
			switch( Particle.GetType() )
			{
			case STypeParticle::kTypeName:
				AX_ASSERT_NOT_NULL( Particle.pToken );
				AX_EXPECT_MEMORY( Result.Append( Particle.pToken->GetString() ) );
				break;

			case STypeParticle::kPointer:
				AX_EXPECT_MEMORY( Result.Append( "*" ) );
				break;
			case STypeParticle::kReference:
				AX_EXPECT_MEMORY( Result.Append( "&" ) );
				break;
			case STypeParticle::kArray:
				AX_EXPECT_MEMORY( Result.Append( "[[array]]" ) );
				break;
			case STypeParticle::kLinkedList:
				AX_EXPECT_MEMORY( Result.Append( "[[linked-list]]" ) );
				break;
			case STypeParticle::kBinaryTree:
				AX_EXPECT_MEMORY( Result.Append( "[[binary-tree]]" ) );
				break;
			case STypeParticle::kConstQualifier:
				AX_EXPECT_MEMORY( Result.Append( "[[const]]" ) );
				break;
			case STypeParticle::kMutableQualifier:
				AX_EXPECT_MEMORY( Result.Append( "[[mutable]]" ) );
				break;
			case STypeParticle::kVolatileQualifier:
				AX_EXPECT_MEMORY( Result.Append( "[[volatile]]" ) );
				break;

			default:
				AX_EXPECT_MEMORY( Result.Append( "(unknown)" ) );
				break;
			}
		}

		AX_EXPECT_MEMORY( Result.Append( ">" ) );
		return Result;
	}
	
	static EBuiltinType KeywordToType( const SToken &Tok )
	{
		if( !Tok.IsKeyword() ) {
			return EBuiltinType::Invalid;
		}

		const Ax::uint32 kw = Tok.pKeyword->Identifier;

		if( kw < kKeyword_Type_Start__ || kw >= kKeyword_Type_End__ ) {
			return EBuiltinType::Invalid;
		}

		switch( kw )
		{
		case kKeyword_Int8:			return EBuiltinType::Int8;
		case kKeyword_Int16:		return EBuiltinType::Int16;
		case kKeyword_Int32:		return EBuiltinType::Int32;
		case kKeyword_Int64:		return EBuiltinType::Int64;
		case kKeyword_Int128:		return EBuiltinType::Int128;
		case kKeyword_IntPtr:		return g_Env->GetIntPtrType();
		case kKeyword_UInt8:		return EBuiltinType::UInt8;
		case kKeyword_UInt16:		return EBuiltinType::UInt16;
		case kKeyword_UInt32:		return EBuiltinType::UInt32;
		case kKeyword_UInt64:		return EBuiltinType::UInt64;
		case kKeyword_UIntPtr:		return g_Env->GetUIntPtrType();
		case kKeyword_Float32:		return EBuiltinType::Float32;
		case kKeyword_Float64:		return EBuiltinType::Float64;
		case kKeyword_Float32x4:	return EBuiltinType::SIMDVector128_4Float32;
		case kKeyword_Float64x2:	return EBuiltinType::SIMDVector128_2Float64;
		case kKeyword_Int32x4:		return EBuiltinType::SIMDVector128_4Int32;
		case kKeyword_Int64x2:		return EBuiltinType::SIMDVector128_2Int64;
		case kKeyword_Float64x4:	return EBuiltinType::SIMDVector256_4Float64;
		case kKeyword_Int64x4:		return EBuiltinType::SIMDVector256_4Int64;
		case kKeyword_Boolean:		return EBuiltinType::Boolean;
		case kKeyword_String:		return EBuiltinType::StringObject;
		case kKeyword_Vector2:		return EBuiltinType::Vector2f;
		case kKeyword_Vector3:		return EBuiltinType::Vector3f;
		case kKeyword_Vector4:		return EBuiltinType::Vector4f;
		case kKeyword_Matrix2:		return EBuiltinType::Matrix2f;
		case kKeyword_Matrix3:		return EBuiltinType::Matrix3f;
		case kKeyword_Matrix4:		return EBuiltinType::Matrix4f;
		case kKeyword_Matrix23:		return EBuiltinType::Matrix23f;
		case kKeyword_Matrix24:		return EBuiltinType::Matrix24f;
		case kKeyword_Matrix32:		return EBuiltinType::Matrix32f;
		case kKeyword_Matrix34:		return EBuiltinType::Matrix34f;
		case kKeyword_Matrix42:		return EBuiltinType::Matrix42f;
		case kKeyword_Matrix43:		return EBuiltinType::Matrix43f;
		case kKeyword_Quaternion:	return EBuiltinType::Quaternionf;
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return EBuiltinType::Invalid;
	}

	bool CTypeDecl::Semant( const SToken &NameTok )
	{
		AX_ASSERT_IS_NULL( m_pRef );

		m_pRef = new STypeRef();
		AX_EXPECT_MEMORY( m_pRef );

		STypeRef *pRef = m_pRef;
		STypeRef *pParentRef = nullptr;

		for( const STypeParticle &Part : m_Chain ) {
			// Check for a type name
			if( Part.IsTypeName() ) {
				// Grab the token pointer for this type
				const SToken *const pTok = Part.TokenPointer();
				AX_ASSERT_NOT_NULL( pTok );

				// If the current reference is an array then we need to figure out what it's an array to
				if( pRef->BuiltinType == EBuiltinType::ArrayObject ) {
					// Create the "to" part of what the array is referencing
					pRef->pRef = new STypeRef();
					AX_EXPECT_MEMORY( pRef->pRef );

					// We're now working on the "to" part
					pParentRef = pRef;
					pRef = pRef->pRef;
				// Otherwise, if a type has yet to be named then the parsed chain is invalid
				} else if( pRef->BuiltinType != EBuiltinType::Invalid ) {
					pTok->Error( "Invalid type chain (type name followed by type name)" );

					delete m_pRef;
					m_pRef = nullptr;

					return false;
				}

				// Part.IsTypeName() implies pTok->IsName()
				AX_ASSERT_MSG( pTok->IsName(), "Token must be a name if this is a type name" );

				// If this is a keyword then it's a built-in type (no type aliases are supported yet)
				if( pTok->IsKeyword() ) {
					// Find the built-in type
					pRef->BuiltinType = KeywordToType( *pTok );

					// If no built-in type corresponds to the given keyword then this is an error
					if( pRef->BuiltinType == EBuiltinType::Invalid ) {
						pTok->Error( "Unknown type \"" + pTok->GetString() + "\"" );

						delete m_pRef;
						m_pRef = nullptr;

						return false;
					}

					// Calculate the size of the type we're referencing
					pRef->cBytes = GetTypeSize( pRef->BuiltinType );
				} else {
					// This must be a user-defined type
					pRef->BuiltinType = EBuiltinType::UserDefined;
					pRef->pCustomType = g_Prog->FindUDT( *pTok );

					if( !pRef->pCustomType ) {
						pTok->Error( "Unknown user defined type \"" + pTok->GetString() + "\"" );

						delete m_pRef;
						m_pRef = nullptr;

						return false;
					}

					// Store the size of the type
					pRef->cBytes = pRef->pCustomType->cBytes;
				}

				// Check the next type-chain particle
				continue;
			}

			// Check for an array
			if( Part.IsArray() ) {
				// If the array particle comes before the "of type" particle then just update the current reference
				if( pRef->BuiltinType == EBuiltinType::Invalid ) {
					pRef->BuiltinType = EBuiltinType::ArrayObject;
					pRef->cBytes = ( Ax::uint32 )g_Env->GetPointerSizeInBytes();
				// Otherwise we have the "type" particle and need to turn it into "array of type"
				} else {
					// Allocate the "array" reference
					STypeRef *pTmpRef = new STypeRef();
					AX_EXPECT_MEMORY( pTmpRef );

					pTmpRef->BuiltinType = EBuiltinType::ArrayObject;
					pTmpRef->cBytes = ( Ax::uint32 )g_Env->GetPointerSizeInBytes();

					// Re-link the references in the proper order
					pTmpRef->pRef = pRef;

					if( pParentRef != nullptr ) {
						pParentRef->pRef = pTmpRef;
					} else {
						m_pRef = pTmpRef;
					}
					pParentRef = pTmpRef;
				}

				continue;
			}

			// Check for const qualifier
			if( Part.IsConstQualifier() ) {
				if( pRef->Access == EAccess::ReadOnly ) {
					m_Token.Warn( "Redundant const-qualifier" );
				} else if( pRef->Access == EAccess::WriteOnly ) {
					m_Token.Warn( "Overriding write-only-qualifier with const-qualifier" );
				}

				pRef->Access = EAccess::ReadOnly;
				continue;
			}

			// Check for volatile qualifier
			if( Part.IsVolatileQualifier() ) {
				if( pRef->bCanReorderMemAccesses == false ) {
					m_Token.Warn( "Redundant volatile-qualifier" );
				}

				pRef->bCanReorderMemAccesses = false;
				continue;
			}

			// If this code is reached then the type chain is bad (or not yet implemented)
			m_Token.Error( "This type chain is either invalid or unsupported at this time" );
			
			delete m_pRef;
			m_pRef = nullptr;

			return false;
		}

		// Check whether we need to describe the "of" type (e.g., array of, linked list of, etc)
		if( pRef->BuiltinType == EBuiltinType::ArrayObject ) {
			pRef->pRef = new STypeRef();
			AX_EXPECT_MEMORY( pRef->pRef );

			pParentRef = pRef;
			pRef = pRef->pRef;
		}

		// Check whether the type needs to be inferred
		if( pRef->BuiltinType == EBuiltinType::Invalid ) {
			pRef->BuiltinType = g_Env->GetIntPtrType();

			if( NameTok.GetString().EndsWith( "#" ) ) {
				pRef->BuiltinType = EBuiltinType::Float32;
			} else if( NameTok.GetString().EndsWith( "$" ) ) {
				pRef->BuiltinType = EBuiltinType::StringObject;
			}

			pRef->cBytes = GetTypeSize( pRef->BuiltinType );
		}

		//
		//	TODO: Verify the type references generated
		//

		// Done
		return true;
	}

	STypeParticle &CTypeDecl::AllocParticle()
	{
		AX_ASSERT_MSG( m_Chain.Resize( m_Chain.Num() + 1 ), "Out of memory" );
		return m_Chain.Last();
	}
	
	Ax::String STypeRef::ToString() const
	{
		Ax::String Result;

		const char *const pszAccessor =
				Access == EAccess::ReadOnly ? "const " :
				Access == EAccess::WriteOnly ? "writeonly " :
				"";

		const char *const pszVolatile =
				bCanReorderMemAccesses ? "" :
				"volatile ";

		AX_EXPECT_MEMORY( Result.Append( pszAccessor ) );
		AX_EXPECT_MEMORY( Result.Append( pszVolatile ) );

		if( BuiltinType == EBuiltinType::UserDefined ) {
			AX_ASSERT_NOT_NULL( pCustomType );

			AX_EXPECT_MEMORY( Result.Append( "UDT:" ) );
			AX_EXPECT_MEMORY( Result.Append( pCustomType->Name ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( BuiltinTypeToString( BuiltinType ) ) );
		}

		if( pRef != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( "::" ) );
			AX_EXPECT_MEMORY( Result.Append( pRef->ToString() ) );
		}

		return Result;
	}

	llvm::Type *STypeRef::CodeGen()
	{
		if( Translated.bDidTranslate ) {
			AX_ASSERT_NOT_NULL( Translated.pType );
			return Translated.pType;
		}

		if( BuiltinType == EBuiltinType::ArrayObject ) {
			AX_ASSERT_NOT_NULL( pRef );

			llvm::Type *const pBaseTy = pRef->CodeGen();
			if( !pBaseTy ) {
				return nullptr;
			}

			Translated.pType = pBaseTy->getPointerTo();
			Translated.bDidTranslate = true;

			return Translated.pType;
		} else {
			AX_EXPECT_MSG( pRef == nullptr, "Complicated type not handled" );
		}

		if( BuiltinType == EBuiltinType::UserDefined ) {
			AX_ASSERT_NOT_NULL( pCustomType );
			AX_ASSERT_NOT_NULL( pCustomType->pLLVMTy );

			Translated.pType = pCustomType->pLLVMTy;
			if( !Translated.pType ) {
				return nullptr;
			}

			Translated.bDidTranslate = true;
			return Translated.pType;
		}

		Translated.pType = CG->GetBuiltinType( BuiltinType );
		if( !Translated.pType ) {
			return nullptr;
		}

		Translated.bDidTranslate = true;
		return Translated.pType;
	}
	llvm::Type *STypeRef::GetValueType() const
	{
		AX_ASSERT_NOT_NULL( Translated.pType );
		return Translated.pType;
	}
	llvm::Type *STypeRef::GetPointerType() const
	{
		AX_ASSERT_NOT_NULL( Translated.pType );
		return llvm::PointerType::get( Translated.pType, 0 );
	}

	bool STypeRef::IsEmbeddedAtHead( const STypeInfo &UDT ) const
	{
		if( BuiltinType != EBuiltinType::UserDefined ) {
			return false;
		}

		AX_ASSERT_NOT_NULL( pCustomType );

		for( const SMemberInfo &Member : pCustomType->Members ) {
			if( Member.uOffset != 0 ) {
				return false;
			}

			if( Member.Type.BuiltinType != EBuiltinType::UserDefined ) {
				continue;
			}
			
			AX_ASSERT_NOT_NULL( Member.Type.pCustomType );

			if( Member.Type.pCustomType == &UDT ) {
				return true;
			}
			if( Member.Type.IsEmbeddedAtHead( UDT ) ) {
				return true;
			}
		}

		return false;
	}

	bool STypeRef::Semant( STypeRef &OutRef, const Ax::Parser::SToken &NameTok, CTypeDecl *pDecl )
	{
		if( pDecl != nullptr ) {
			if( !pDecl->Semant( NameTok ) ) {
				return false;
			}

			OutRef = pDecl->Ref();
			return true;
		}

		const char *const s = NameTok.GetPointer();
		const char *const e = s + NameTok.cLength;
		AX_ASSERT( s != e );

		if( *( e - 1 ) == '$' ) {
			OutRef.BuiltinType = EBuiltinType::StringObject;
		} else if( *( e - 1 ) == '#' ) {
			OutRef.BuiltinType = EBuiltinType::Float32;
		} else {
			OutRef.BuiltinType = g_Env->GetIntPtrType();
		}
		OutRef.cBytes = GetTypeSize( OutRef.BuiltinType );

		OutRef.Access = EAccess::ReadWrite;
		OutRef.bCanReorderMemAccesses = true;
		OutRef.pCustomType = nullptr;
		OutRef.pRef = nullptr;

		return true;
	}
	STypeRef *STypeRef::Semant( const Ax::Parser::SToken &NameTok, CTypeDecl *pDecl)
	{
		STypeRef *const pRef = new STypeRef();
		AX_EXPECT_MEMORY( pRef );

		if( !Semant( *pRef, NameTok, pDecl ) ) {
			delete pRef;
			return nullptr;
		}

		return pRef;
	}

	ECast STypeRef::Cast( const STypeRef &From, const STypeRef &To )
	{
		ECast CastOp;
		if( From.BuiltinType == EBuiltinType::UserDefined && To.BuiltinType == EBuiltinType::UserDefined ) {
			AX_ASSERT_NOT_NULL( From.pCustomType );
			AX_ASSERT_NOT_NULL( To.pCustomType );

			CastOp = ECast::Invalid;
			if( From.pCustomType == To.pCustomType ) {
				CastOp = ECast::None;
			} else if( From.IsEmbeddedAtHead( *To.pCustomType ) ) {
				CastOp = ECast::None;
			}
		} else {
			CastOp = GetCastForTypes( From.BuiltinType, To.BuiltinType, To.Access != EAccess::ReadOnly ? ECastMode::Output : ECastMode::Input );
		}

		return CastOp;
	}
	bool STypeRef::CastError( const SToken &Tok, const STypeRef &From, const STypeRef &To )
	{
		Tok.Error( "Cannot cast from \"" + From.ToString() + "\" to \"" + To.ToString() + "\"" );
		return false;
	}

}}

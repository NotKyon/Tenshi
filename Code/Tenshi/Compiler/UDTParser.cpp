#include "_PCH.hpp"
#include "UDTParser.hpp"

#include "Environment.hpp"
#include "Program.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	using namespace Ax::Parser;

	CUserDefinedType::CUserDefinedType( const SToken &Tok, CParser &Parser )
	: m_Token( Tok )
	, m_Parser( Parser )
	, m_Lexer( Parser.Lexer() )
	, m_pNameTok( nullptr )
	, m_pEndtypeTok( nullptr )
	, m_pTypeInfo( nullptr )
	{
		memset( &m_CodeGen, 0, sizeof( m_CodeGen ) );
	}
	CUserDefinedType::~CUserDefinedType()
	{
	}

	bool CUserDefinedType::Parse()
	{
		AX_ASSERT( m_Token.IsKeyword( kKeyword_Type ) );

		const SToken &NameTok = m_Lexer.ExpectLine( ETokenType::Name );
		if( !NameTok ) {
			return false;
		}
		if( NameTok.IsKeyword() ) {
			m_Lexer.Expected( "Expected non-keyword for TYPE name" );
			return false;
		}

		m_pNameTok = &NameTok;

		for(;;) {
			const SToken &CheckTok = m_Lexer.Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				m_Lexer.Expected( "Expected a field on a new line in TYPE/ENDTYPE" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_EndType ) ) {
				m_pEndtypeTok = &CheckTok;
				break;
			}

			bool bIsFieldDone = false;
			bool bIsAsStmt = false;
			if( m_Lexer.CheckLine( ETokenType::Punctuation, "," ) ) {
				bIsFieldDone = true;
				m_Lexer.Unlex();
			} else {
				const SToken &DoneTok = m_Lexer.Lex();
				if( DoneTok.StartsLine() ) {
					bIsFieldDone = true;
					m_Lexer.Unlex();
				} else if( DoneTok.IsKeyword( kKeyword_As ) ) {
					bIsAsStmt = true;
				} else {
					m_Lexer.Unlex();
				}
			}

			// Parse <Field1> "," <Field2> "," ... <FieldN>
			if( bIsFieldDone ) {
				const SToken *pNameTok = &CheckTok;
				for(;;) {
					if( pNameTok->IsKeyword() ) {
						m_Lexer.Expected( "Expected non-keyword for field name in TYPE/ENDTYPE" );
						return false;
					}

					CUDTField &Field = AllocField( *pNameTok );
					Field.m_pNameTok = pNameTok;

					if( !m_Lexer.CheckLine( ETokenType::Punctuation, "," ) ) {
						break;
					}

					if( !m_Lexer.Expect( ETokenType::Name ) ) {
						return false;
					}

					pNameTok = &m_Lexer.Token();
				}

				continue;
			}
			// Parse <FieldName> "as" <TypeName>
			if( bIsAsStmt ) {
				CUDTField &Field = AllocField( CheckTok );

				if( !m_Lexer.ExpectLine( ETokenType::Name ) ) {
					return false;
				}

				Field.m_pType = new CTypeDecl( m_Lexer.Token(), m_Parser );
				AX_EXPECT_MEMORY( Field.m_pType );

				if( !Field.m_pType->Parse() ) {
					return false;
				}

				Field.m_pNameTok = &CheckTok;

				continue;
			}

			// Parse <TypeName> <Field1> "," <Field2> "," ... <FieldN>
			while( m_Lexer.ExpectLine( Ax::Parser::ETokenType::Name ) ) {
				CUDTField &Field = AllocField( m_Lexer.Token() );

				Field.m_pNameTok = &m_Lexer.Token();

				Field.m_pType = new CTypeDecl( CheckTok, m_Parser );
				AX_EXPECT_MEMORY( Field.m_pType );

				if( !Field.m_pType->ParseOne() ) {
					return false;
				}

				if( !m_Lexer.CheckLine( Ax::Parser::ETokenType::Punctuation, "," ) ) {
					break;
				}
			}
		}

		AX_ASSERT_NOT_NULL( m_pEndtypeTok );
		return true;
	}

	Ax::String CUserDefinedType::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "UDT <" ) );
		if( m_pNameTok != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pNameTok->GetString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		for( const CUDTField *const pField : m_Fields ) {
			AX_ASSERT_NOT_NULL( pField );
			const CUDTField &Field = *pField;

			AX_EXPECT_MEMORY( Result.Append( "\n\t" ) );
			AX_EXPECT_MEMORY( Result.Append( Field.ToString() ) );
		}

		return Result;
	}

	bool CUserDefinedType::Semant()
	{
		const uint32 cPtrBytes = ( uint32 )g_Env->GetPointerSizeInBytes();
		AX_ASSERT_NOT_NULL( m_pNameTok );
		AX_ASSERT_NOT_NULL( m_pEndtypeTok );

		Ax::String Name = m_pNameTok->GetString();

		SSymbol *const pSym = g_Prog->GlobalScope().AddSymbol( Name );
		if( !pSym ) {
			return false;
		}

		pSym->pDeclToken = &m_Token;
		pSym->pNameToken = m_pNameTok;

		m_pTypeInfo = new STypeInfo();
		AX_EXPECT_MEMORY( m_pTypeInfo );

		pSym->pUDT = m_pTypeInfo;

		m_pTypeInfo->cBytes = 0;
		m_pTypeInfo->uAlignment = cPtrBytes;
		m_pTypeInfo->bIsInitTrivial = true;
		m_pTypeInfo->bIsFiniTrivial = true;
		m_pTypeInfo->bIsCopyTrivial = true;
		m_pTypeInfo->bIsMoveTrivial = true;

		m_pTypeInfo->pLLVMTy = nullptr;
		m_pTypeInfo->pLLVMElementIndex = nullptr;
		m_pTypeInfo->pLLVMInitFn = nullptr;
		m_pTypeInfo->pLLVMFiniFn = nullptr;
		m_pTypeInfo->pLLVMCopyFn = nullptr;
		m_pTypeInfo->pLLVMMoveFn = nullptr;

		m_pTypeInfo->pScope = new CScope();
		AX_EXPECT_MEMORY( m_pTypeInfo->pScope );

		m_pTypeInfo->pScope->SetDictionary( g_Env->Dictionary(), "#T" + Name + "$" );

		g_Prog->PushScope( *m_pTypeInfo->pScope );
		for( CUDTField *const pField : m_Fields ) {
			AX_ASSERT_NOT_NULL( pField );
			CUDTField &Field = *pField;

			AX_ASSERT_NOT_NULL( Field.m_pNameTok );

			m_Parser.PushErrorToken( *Field.m_pNameTok );
			const bool bResult = Field.Semant( *m_pTypeInfo );
			m_Parser.PopErrorToken();

			if( !bResult ) {
				g_Prog->PopScope();
				return false;
			}
		}
		g_Prog->PopScope();

		AX_ASSERT_NOT_NULL( m_pTypeInfo );
		Ax::uintptr uFieldIndex = 0;
		for( CUDTField *const pField : m_Fields ) {
			AX_ASSERT_NOT_NULL( pField );
			AX_ASSERT_NOT_NULL( pField->m_pSym );
			AX_ASSERT( m_pTypeInfo->Members.Num() > uFieldIndex );

			pField->m_pSym->pVar = &m_pTypeInfo->Members[ uFieldIndex ];
			pField->m_pSym->pVar->iFieldIndex = uFieldIndex;
			++uFieldIndex;
		}
		
		m_pTypeInfo->Name.Swap( Name );
		return true;
	}

	static llvm::Function *BeginFunc( llvm::IRBuilder<> &IRBuilder, llvm::FunctionType *pFTy, llvm::StringRef Name )
	{
		llvm::Function *const pFunc =
			llvm::Function::Create
			(
				pFTy,
				llvm::GlobalValue::PrivateLinkage,
				Name,
				&CG->Module()
			);
		
		llvm::BasicBlock *const pEntryBlock =
			llvm::BasicBlock::Create( CG->Context(), "begin", pFunc );

		IRBuilder.SetInsertPoint( pEntryBlock );

		return pFunc;
	}

	bool CUserDefinedType::CodeGen()
	{
		Ax::TArray< llvm::Type * > Elements;

		AX_EXPECT_MEMORY( Elements.Reserve( m_Fields.Num() ) );
		for( CUDTField *const pField : m_Fields ) {
			AX_ASSERT_NOT_NULL( pField );
			AX_ASSERT_NOT_NULL( pField->m_pSym );
			AX_ASSERT_NOT_NULL( pField->m_pSym->pVar );
			AX_ASSERT_NOT_NULL( pField->m_pRef );
			AX_ASSERT( pField->m_pSym->pVar->iFieldIndex == Elements.Num() );

			llvm::Type *const pFieldTy = pField->m_pRef->CodeGen();
			if( !pFieldTy ) {
				pField->m_Token.Error( "[CodeGen] Failed to get type for field" );
				return false;
			}

			Elements.Append( pFieldTy );
		}

		AX_ASSERT_NOT_NULL( m_pTypeInfo );
		m_pTypeInfo->pLLVMTy = llvm::StructType::create( CG->Context(), LLVMArr( Elements ), LLVMStr( m_pNameTok ) );
		if( !m_pTypeInfo->pLLVMTy ) {
			m_pNameTok->Error( "[CodeGen] Failed to create structure" );
			return false;
		}

		const unsigned uElementIndex = CG->GetTypeId();
		m_pTypeInfo->pLLVMElementIndex =
			llvm::Constant::getIntegerValue
			(
				llvm::Type::getInt32Ty( CG->Context() ),
				llvm::APInt( 32, ( uint64_t )uElementIndex, false )
			);

		llvm::IRBuilder<> Builder( CG->Context() );

		Ax::String InitName, FiniName, CopyName, MoveName;
		Ax::String *Names[] = { &InitName, &FiniName, &CopyName, &MoveName };

		for( Ax::String *Name : Names ) {
			AX_EXPECT_MEMORY( Name->Assign( m_pNameTok->GetString() ) );
			AX_EXPECT_MEMORY( Name->Append( '.' ) );
		}
		AX_EXPECT_MEMORY( InitName.Append( "Init" ) );
		AX_EXPECT_MEMORY( FiniName.Append( "Fini" ) );
		AX_EXPECT_MEMORY( CopyName.Append( "Copy" ) );
		AX_EXPECT_MEMORY( MoveName.Append( "Move" ) );

		// Constructor
		if( !m_pTypeInfo->bIsInitTrivial ) {
			m_pTypeInfo->pLLVMInitFn =
				BeginFunc( Builder, CG->GetObjInitFnTy(), LLVMStr( InitName ) );

			llvm::ConstantInt *const pRetVal =
				llvm::ConstantInt::get
				(
					llvm::Type::getInt1Ty( CG->Context() ),
					uint64_t( 1 ),
					true
				);

			Builder.CreateRet( pRetVal );
		}

		// Destructor
		if( !m_pTypeInfo->bIsFiniTrivial ) {
			m_pTypeInfo->pLLVMFiniFn =
				BeginFunc( Builder, CG->GetObjFiniFnTy(), LLVMStr( FiniName ) );

			Builder.CreateRetVoid();
		}

		// Copy operation
		if( !m_pTypeInfo->bIsCopyTrivial ) {
			m_pTypeInfo->pLLVMCopyFn =
				BeginFunc( Builder, CG->GetObjCopyFnTy(), LLVMStr( CopyName ) );

			Builder.CreateRetVoid();
		}

		// Move operation
		if( !m_pTypeInfo->bIsMoveTrivial ) {
			m_pTypeInfo->pLLVMMoveFn =
				BeginFunc( Builder, CG->GetObjMoveFnTy(), LLVMStr( MoveName ) );

			Builder.CreateRetVoid();
		}

		CG->RegisterUDT( *m_pTypeInfo );

		return true;
	}
	llvm::Type *CUserDefinedType::GetValueType() const
	{
		AX_ASSERT_NOT_NULL( m_CodeGen.pLLVMTy );
		return m_CodeGen.pLLVMTy;
	}
	llvm::Type *CUserDefinedType::GetPointerType() const
	{
		AX_ASSERT_NOT_NULL( m_CodeGen.pLLVMTy );
		return llvm::PointerType::get( m_CodeGen.pLLVMTy, 0 );
	}

	CUDTField &CUserDefinedType::AllocField( const SToken &Tok )
	{
		CUDTField *const pField = new CUDTField( Tok, m_Parser );
		AX_EXPECT_MEMORY( pField );

		AX_EXPECT_MEMORY( m_Fields.Append( pField ) );

		return *pField;
	}
	
	CUDTField::CUDTField( const SToken &Tok, CParser &Parser )
	: m_Token( Tok )
	, m_pNameTok( nullptr )
	, m_pType( nullptr )
	, m_pSym( nullptr )
	{
		( void )Parser;
	}
	CUDTField::~CUDTField()
	{
	}

	Ax::String CUDTField::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "Field <" ) );
		if( m_pNameTok != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pNameTok->GetString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		if( m_pType != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( ":" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pType->ToString() ) );
		}

		return Result;
	}

	bool CUDTField::Semant( STypeInfo &DstInfo )
	{
		AX_ASSERT_NOT_NULL( m_pNameTok );

		AX_EXPECT_MEMORY( DstInfo.Members.Resize( DstInfo.Members.Num() + 1 ) );

		SMemberInfo &Info = DstInfo.Members.Last();

		Info.PassBy = EPassBy::Direct;
		Info.PassMod = EPassMod::Direct;
		Info.Name = m_pNameTok->GetString();

		m_pSym = g_Prog->AddSymbol( Info.Name );
		if( !m_pSym ) {
			return false;
		}

		m_pSym->pNameToken = m_pNameTok;
		m_pSym->pDeclToken = m_pNameTok;

		m_pRef = STypeRef::Semant( *m_pNameTok, m_pType );
		if( !m_pRef ) {
			return false;
		}

		Info.Type = *m_pRef;

		const uint32 cPtrBytes = ( uint32 )g_Env->GetPointerSizeInBytes();

		uint32 uAlign = cPtrBytes;
		if( Info.Type.BuiltinType == EBuiltinType::UserDefined ) {
			AX_ASSERT_NOT_NULL( Info.Type.pCustomType );

			DstInfo.bIsInitTrivial &= Info.Type.pCustomType->bIsInitTrivial;
			DstInfo.bIsFiniTrivial &= Info.Type.pCustomType->bIsFiniTrivial;
			DstInfo.bIsCopyTrivial &= Info.Type.pCustomType->bIsCopyTrivial;
			DstInfo.bIsMoveTrivial &= Info.Type.pCustomType->bIsMoveTrivial;

			uAlign = Info.Type.pCustomType->uAlignment;
		} else {
			const bool bIsTrivial = IsTrivial( Info.Type.BuiltinType );

			DstInfo.bIsInitTrivial &= bIsTrivial;
			DstInfo.bIsFiniTrivial &= bIsTrivial;
			DstInfo.bIsCopyTrivial &= bIsTrivial;
			DstInfo.bIsMoveTrivial &= bIsTrivial;
		}

		Info.uOffset = DstInfo.cBytes;
		if( Info.uOffset%uAlign != 0 ) {
			Info.uOffset += uAlign - Info.uOffset%uAlign;
		}

		DstInfo.cBytes = Info.uOffset + Info.Type.cBytes;

		return true;
	}

}}

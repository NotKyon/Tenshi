#include "_PCH.hpp"
#include "FunctionParser.hpp"
#include "StmtParser.hpp"
#include "Program.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	using namespace Ax::Parser;
	
	CFunctionDecl::CFunctionDecl( const SToken &Tok, CParser &Parser )
	: m_Token( Tok )
	, m_Parser( Parser )
	, m_Lexer( Parser.Lexer() )
	, m_pNameTok( nullptr )
	, m_Parms()
	, m_pReturnType( nullptr )
	, m_Stmts()
	, m_Semanted()
	{
		m_Stmts.SetType( EStmtSeqType::Function );
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}
	CFunctionDecl::~CFunctionDecl()
	{
	}

	bool CFunctionDecl::Parse()
	{
		const SToken &Tok = m_Token;

		AX_ASSERT( Tok.IsKeyword( kKeyword_Function ) );

		const SToken &NameTok = m_Lexer.ExpectLine( Ax::Parser::ETokenType::Name );
		if( !NameTok ) {
			return false;
		}
		if( NameTok.IsKeyword() ) {
			m_Lexer.Expected( "Expected non-keyword name for FUNCTION" );
			return false;
		}

		m_pNameTok = &NameTok;

		if( !ParseParameters() ) {
			return false;
		}

		if( m_Lexer.CheckLine( ETokenType::Name ) ) {
			if( m_Lexer.Token().IsKeyword( kKeyword_As ) ) {
				if( !m_Lexer.ExpectLine( ETokenType::Name ) ) {
					return false;
				}

				m_pReturnType = new CTypeDecl( m_Lexer.Token(), m_Parser );
				AX_EXPECT_MEMORY( m_pReturnType );

				if( !m_pReturnType->Parse() ) {
					return false;
				}
			} else {
				m_Lexer.Unlex();
			}
		}
		
		// Short-hand?
		// e.g., function getSum( x, y ) => x + y
		if( m_Lexer.Check( ETokenType::Punctuation, "=>" ) ) {
			const SToken &Tok = m_Lexer.Token();
			auto &Stmt = m_Stmts.NewStmt< CExitFunctionStmt >( Tok, m_Parser );
			return Stmt.Parse();
		}

		for(;;) {
			const SToken &CheckTok = m_Lexer.Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				m_Lexer.Expected( "Expected statement on a new line in FUNCTION \"" + NameTok.GetString() + "\"" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_EndFunction ) ) {
				auto &Stmt = m_Stmts.NewStmt< CExitFunctionStmt >( CheckTok, m_Parser );
				if( !Stmt.Parse() ) {
					return false;
				}

				break;
			}

			m_Lexer.Unlex();
			if( !m_Parser.ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		return true;
	}
	bool CFunctionDecl::ParseParameters()
	{
		if( !m_Lexer.Expect( ETokenType::Punctuation, "(" ) ) {
			return false;
		}

		if( m_Lexer.Check( ETokenType::Punctuation, ")" ) ) {
			return true;
		}

		bool bLastHadDefault = false;
		for(;;) {
			if( !ParseParameter() ) {
				return false;
			}

			AX_ASSERT_NOT_NULL( m_Parms.Last() );
			AX_ASSERT_NOT_NULL( m_Parms.Last()->m_pNameTok );
			if( m_Parms.Last()->HasDefault() ) {
				bLastHadDefault = true;
			} else if( bLastHadDefault ) {
				m_Parms.Last()->m_pNameTok->Error( "This parameter (and all after it) require default parameters" );
				return false;
			}

			if( !m_Lexer.Check( ETokenType::Punctuation, "," ) ) {
				break;
			}
		}

		if( !m_Lexer.Expect( ETokenType::Punctuation, ")" ) ) {
			return false;
		}

		return true;
	}
	bool CFunctionDecl::ParseParameter()
	{
		CParameterDecl &Parm = AllocParm();

		const SToken &NameTok = m_Lexer.Expect( ETokenType::Name );
		if( !NameTok ) {
			return false;
		}
		if( NameTok.IsKeyword() ) {
			m_Lexer.Expected( "Expected non-keyword parameter name" );
			return false;
		}

		Parm.m_pNameTok = &NameTok;

		if( m_Lexer.Check( ETokenType::Name ) ) {
			if( m_Lexer.Token().IsKeyword( kKeyword_As ) ) {
				if( !m_Lexer.ExpectLine( ETokenType::Name ) ) {
					return false;
				}

				Parm.m_pType = new CTypeDecl( m_Lexer.Token(), m_Parser );
				AX_EXPECT_MEMORY( Parm.m_pType );

				if( !Parm.m_pType->Parse() ) {
					return false;
				}
			} else {
				m_Lexer.Unlex();
			}
		}

		if( m_Lexer.Check( Ax::Parser::ETokenType::Punctuation, "=" ) ) {
			Parm.m_pDefault = m_Parser.ParseExpression();
			if( !Parm.m_pDefault ) {
				return false;
			}
		}

		return true;
	}

	Ax::String CFunctionDecl::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "Function <" ) );
		if( m_pNameTok != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pNameTok->GetString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		if( m_pReturnType != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " returns:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pReturnType->ToString() ) );
		}

		for( const CParameterDecl *const pParm : m_Parms ) {
			AX_ASSERT_NOT_NULL( pParm );
			const CParameterDecl &Parm = *pParm;

			AX_EXPECT_MEMORY( Result.Append( "\n\t" ) );
			AX_EXPECT_MEMORY( Result.Append( Parm.ToString() ) );
		}

		AX_EXPECT_MEMORY( Result.Append( "\n" ) );
		AX_EXPECT_MEMORY( Result.Append( m_Stmts.ToString() ) );

		return Result;
	}

	bool CFunctionDecl::PreSemant()
	{
		AX_ASSERT_NOT_NULL( m_pNameTok );

		// Create the symbol and allocate the function
		m_Semanted.pSym = g_Prog->GlobalScope().AddSymbol( *m_pNameTok );
		if( !m_Semanted.pSym ) {
			m_pNameTok->Error( "Symbol already exists; cannot name function \"" + m_pNameTok->GetString() + "\"" );
			return false;
		}
		m_Semanted.pSym->pDeclToken = m_pNameTok;

		m_Semanted.pSym->pFunc = new SFunctionInfo();
		AX_EXPECT_MEMORY( m_Semanted.pSym->pFunc );

		m_Semanted.pSym->pFunc->Name = m_pNameTok->GetString();
		AX_EXPECT_MEMORY( m_Semanted.pSym->pFunc->Name.Len() == m_pNameTok->cLength );

		auto Iter = m_Semanted.pSym->pFunc->Overloads.AddTail();
		AX_EXPECT_MEMORY( Iter != m_Semanted.pSym->pFunc->Overloads.end() );

		// Create the scope for this function overload
		m_Semanted.pOverload = Iter.Get();
		AX_ASSERT_NOT_NULL( m_Semanted.pOverload );

		SFunctionOverload &Fn = *m_Semanted.pOverload;

		Fn.pModule = nullptr;
		AX_EXPECT_MEMORY( Fn.RealName.Assign( "_" ) );
		AX_EXPECT_MEMORY( Fn.RealName.Append( m_pNameTok->GetPointer(), ( Ax::intptr )m_pNameTok->cLength ) );

		{
			Ax::String Prefix;

			AX_EXPECT_MEMORY( Prefix.Assign( Fn.RealName ) );
			AX_EXPECT_MEMORY( Prefix.Append( "#" ) );

			AX_ASSERT_NOT_NULL( m_Semanted.pSym->pDeclScope );
			m_Semanted.pScope = m_Semanted.pSym->pDeclScope->AddScope( Prefix );
			AX_EXPECT_MEMORY( m_Semanted.pScope );
		}

		m_Semanted.pScope->SetOwner( *m_Semanted.pSym );

		// Semant the parameters to the function
		const Ax::uint32 uAlign = ( Ax::uint32 )g_Env->GetPointerSizeInBytes();
		Ax::uint32 uOffset = 0;
		for( CParameterDecl *pParm : m_Parms ) {
			AX_ASSERT_NOT_NULL( pParm );
			AX_ASSERT_NOT_NULL( pParm->m_pNameTok );

			pParm->m_Semanted.pSym = m_Semanted.pScope->AddSymbol( *pParm->m_pNameTok );
			if( !pParm->m_Semanted.pSym ) {
				pParm->m_pNameTok->Error( "Parameter already exists or has invalid name: \"" + pParm->m_pNameTok->GetString() + "\"" );
				return false;
			}

			pParm->m_Semanted.pSym->pDeclToken = pParm->m_pNameTok;

			AX_EXPECT_MEMORY( Fn.Parameters.Append() );
			SMemberInfo &Parm = Fn.Parameters.Last();

			AX_EXPECT_MEMORY( Parm.Name.Assign( pParm->m_pNameTok->GetPointer(), ( Ax::intptr )pParm->m_pNameTok->cLength ) );
			if( !STypeRef::Semant( Parm.Type, *pParm->m_pNameTok, pParm->m_pType ) ) {
				return false;
			}
			if( Parm.Type.BuiltinType == EBuiltinType::UserDefined ) {
				Parm.Type.Access = EAccess::ReadOnly;
				Parm.PassBy = EPassBy::Reference;
			} else if( Parm.Type.BuiltinType == EBuiltinType::Void ) {
				pParm->m_pNameTok->Error( "Cannot use void as type for parameter: \"" + pParm->m_pNameTok->GetString() + "\"" );
				return false;
			} else {
				Parm.PassBy = EPassBy::Value;
			}
			Parm.PassMod = EPassMod::Direct;
			Parm.uOffset = uOffset;
			uOffset += Parm.Type.cBytes;
			uOffset += uOffset%uAlign != 0 ? uAlign - uOffset%uAlign : 0;
			if( pParm->m_pDefault != nullptr ) {
				pParm->m_pDefault->Token().Warn( "Default parameters are not yet implemented; ignoring" );
			}
		}

		// Get the pointers to the parameters for the symbols (now that the array is no longer being resized)
		Ax::uintptr uParm = 0;
		for( CParameterDecl *pParm : m_Parms ) {
			AX_ASSERT_NOT_NULL( pParm );
			AX_ASSERT_NOT_NULL( pParm->m_Semanted.pSym );

			AX_ASSERT( uParm < Fn.Parameters.Num() );

			pParm->m_Semanted.pSym->pVar = Fn.Parameters.Pointer( uParm );
			++uParm;
		}

		// Determine the return type
		if( !STypeRef::Semant( Fn.ReturnInfo.Type, *m_pNameTok, m_pReturnType ) ) {
			m_pNameTok->Error( "Invalid return type" );
			return false;
		}

		Fn.ReturnInfo.PassBy = EPassBy::Value;
		Fn.ReturnInfo.PassMod = EPassMod::Direct;
		Fn.ReturnInfo.uOffset = 0;

		// Everything seems to be in order
		return true;
	}
	bool CFunctionDecl::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pNameTok );
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pScope );
		AX_ASSERT_NOT_NULL( m_Semanted.pOverload );

		g_Prog->PushScope( *m_Semanted.pScope );
		const bool bDidSemant = m_Stmts.Semant();
		g_Prog->PopScope();

		if( !bDidSemant ) {
			return false;
		}

		return true;
	}
	bool CFunctionDecl::PreCodeGen()
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pScope );
		AX_ASSERT_NOT_NULL( m_Semanted.pOverload );
		AX_ASSERT_NOT_NULL( m_Semanted.pSym->pFunc );

		if( !m_Semanted.pSym->pFunc->GenDecls() ) {
			return false;
		}

		return true;
	}
	bool CFunctionDecl::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pScope );
		AX_ASSERT_NOT_NULL( m_Semanted.pOverload );
		AX_ASSERT_NOT_NULL( m_Semanted.pOverload->pLLVMFunc );

		// LLVM function -- already built from PreCodeGen() (the declaration)
		llvm::Function *const pFunc = m_Semanted.pOverload->pLLVMFunc;

		// Entry block to the function
		llvm::BasicBlock *const pEntry = llvm::BasicBlock::Create( CG->Context(), "entry", pFunc );
		if( !pEntry ) {
			return false;
		}

		CG->SetCurrentBlock( *pEntry );
		CG->Builder().SetInsertPoint( pEntry );

		// Prepare the clean-up scope
		const bool bIsTopLevel = CG->EnterScope();
		AX_ASSERT( bIsTopLevel );

		// Manage all of the parameters
		llvm::Function::arg_iterator arg = pFunc->arg_begin();
		for( uintptr i = 0; i < m_Parms.Num(); ++i, ++arg ) {
			AX_ASSERT( arg != pFunc->arg_end() );

			CParameterDecl *const pDecl = m_Parms[ i ];
			AX_ASSERT_NOT_NULL( pDecl );

			if( pDecl->m_pNameTok != nullptr ) {
				arg->setName( LLVMStr( pDecl->m_pNameTok ) );
			}

			AX_ASSERT_NOT_NULL( pDecl->m_Semanted.pSym );
			AX_ASSERT_NOT_NULL( pDecl->m_Semanted.pSym->pVar );
			AX_ASSERT( pDecl->m_Semanted.pSym->pVar->Type.BuiltinType != EBuiltinType::Invalid );

			// This just creates the alloca -- TODO: Just pull into here
			if( pDecl->m_Semanted.pSym->pVar->Type.BuiltinType == EBuiltinType::UserDefined ) {
				pDecl->m_Semanted.pSym->Translated.pValue = &*arg;
			} else {
				if( !pDecl->CodeGen() ) {
					CG->LeaveScope();
					return false;
				}

				// It's okay for functions to modify their local copies of
				// parameters, but we need to copy to their stack local first.
				//
				// Note that LLVM optimizes this away pretty well.
				AX_ASSERT_NOT_NULL( pDecl->m_Semanted.pSym );
				AX_ASSERT_NOT_NULL( pDecl->m_Semanted.pSym->Translated.pValue );
				CG->Builder().CreateStore( &*arg, pDecl->m_Semanted.pSym->Translated.pValue );
			}
		}

		// Build the main body of the function
		//
		// There's always an EXITFUNCTION call at the end of this list.
		// (TODO: Assert that.)
		if( !m_Stmts.CodeGen() ) {
			CG->LeaveScope();
			return false;
		}

		CG->LeaveScope();

		// Always verify the function that we generated -- make sure to change
		// this into an option that can be passed on the command-line that is
		// enabled by default for debug builds.
		if( llvm::verifyFunction( *m_Semanted.pOverload->pLLVMFunc ) ) {
			if( m_pNameTok != nullptr ) {
				m_pNameTok->Warn( "[CodeGen] Broken function" );
			}

#if AX_DEBUG_ENABLED
			m_Semanted.pOverload->pLLVMFunc->dump();
#else
			return false;
#endif
		}

		// Optimize the function
		if( CG->CanOptimize() ) {
			CG->FPM().run( *m_Semanted.pOverload->pLLVMFunc );
		}

		// Done
		return true;
	}
	
	CParameterDecl &CFunctionDecl::AllocParm()
	{
		CParameterDecl *const pParm = new CParameterDecl();
		AX_EXPECT_MEMORY( pParm );

		AX_EXPECT_MEMORY( m_Parms.Append( pParm ) );

		return *pParm;
	}

	CParameterDecl::CParameterDecl()
	: m_pNameTok( nullptr )
	, m_pType( nullptr )
	, m_pDefault( nullptr )
	{
	}
	CParameterDecl::~CParameterDecl()
	{
	}

	bool CParameterDecl::HasDefault() const
	{
		return m_pDefault != nullptr;
	}

	Ax::String CParameterDecl::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "Parm <" ) );
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

		if( m_pDefault != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " = " ) );
			AX_EXPECT_MEMORY( Result.Append( m_pDefault->ToString() ) );
		}

		return Result;
	}

	bool CParameterDecl::Semant()
	{
		// TODO

		AX_ASSERT_NOT_NULL( m_pNameTok );
		return false;
	}
	bool CParameterDecl::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_Semanted.pSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pSym->pVar );

		llvm::Type *const pType = m_Semanted.pSym->pVar->Type.CodeGen();
		if( !pType ) {
			return false;
		}

		m_Semanted.pSym->Translated.pValue = CG->Builder().CreateAlloca( pType, nullptr, LLVMStr( m_pNameTok ) );
		if( !m_Semanted.pSym->Translated.pValue ) {
			return false;
		}

		return true;
	}

}}

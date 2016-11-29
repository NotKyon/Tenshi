#include "_PCH.hpp"

#include "StmtParser.hpp"

#include "Parser.hpp"
#include "Node.hpp"
#include "Symbol.hpp"
#include "TypeInformation.hpp"
#include "Program.hpp"
#include "ExprParser.hpp"
#include "CodeGen.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	using namespace Ax::Parser;

	void CBlockStatement::SetSequence( CStatementSequence &Seq )
	{
		CStatement::SetSequence( Seq );
		m_Stmts.Inherit( Seq );
	}

	bool CBlockStatement::Semant()
	{
		return m_Stmts.Semant();
	}
	bool CBlockStatement::CodeGen()
	{
		return m_Stmts.CodeGen();
	}

	/*
	===========================================================================

		FUNCTION CALL STATEMENT

	===========================================================================
	*/

	CFunctionCallStmt::CFunctionCallStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::InvokeFunction, Tok, Parser )
	, m_pExpr( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}
	
	bool CFunctionCallStmt::Parse()
	{
		return false;
	}
	void CFunctionCallStmt::Parsed( CExpression &Expr )
	{
		m_pExpr = &Expr;
	}

	Ax::String CFunctionCallStmt::ToString() const
	{
		Ax::String Result;
		
		AX_EXPECT_MEMORY( Result.Append( "CallFunction <" ) );
		AX_EXPECT_MEMORY( Result.Append( Token().GetString() ) );
		AX_EXPECT_MEMORY( Result.Append( "> parms:" ) );
		AX_EXPECT_MEMORY( Result.Append( m_pExpr != nullptr ? m_pExpr->ToString() : " (null)" ) );

		return Result;
	}

	bool CFunctionCallStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pExpr );

		if( !m_pExpr->Semant() ) {
			return false;
		}

		if( Token().IsString() ) {
			m_Semanted.bIsAutoprint = true;
		}

		return true;
	}
	bool CFunctionCallStmt::CodeGen()
	{
		CG->EnterScope();

		SValue Val = m_pExpr->CodeGen();
		if( !Val ) {
			CG->LeaveScope();
			return false;
		}

		if( m_Semanted.bIsAutoprint ) {
			CG->Builder().CreateCall( CG->InternalFuncs().pAutoprint, Val.Load(), "" );
		}

		CG->CleanScope();
		CG->LeaveScope();

		return true;
	}


	/*
	===========================================================================

		LABEL DECLARATION STATEMENT

	===========================================================================
	*/

	CLabelDeclStmt::CLabelDeclStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::LabelDecl, Tok, Parser )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}

	Ax::String CLabelDeclStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "LabelDecl <" ) );
		AX_EXPECT_MEMORY( Result.Append( Token().GetString() ) );
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		return Result;
	}

	bool CLabelDeclStmt::Semant()
	{
		if( m_Semanted.pLabelSym != nullptr ) {
			return true;
		}

		m_Semanted.pLabelSym = g_Prog->AddSymbol( Token().GetString() );
		if( !m_Semanted.pLabelSym ) {
			return false;
		}

		m_Semanted.pLabelSym->pDeclToken = &Token();
		m_Semanted.pLabelSym->pLabel = this;

		m_Semanted.pLabelSym->Translated.pValue = llvm::BasicBlock::Create( CG->Context(), LLVMStr( Token() ) );
		AX_EXPECT_MEMORY( m_Semanted.pLabelSym->Translated.pValue );

		return true;
	}
	bool CLabelDeclStmt::CodeGen()
	{
		AX_ASSERT( CG->HasCurrentBlock() );
		AX_ASSERT_NOT_NULL( m_Semanted.pLabelSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pLabelSym->Translated.pValue );

		llvm::BasicBlock *const pBlock = llvm::dyn_cast_or_null< llvm::BasicBlock >( m_Semanted.pLabelSym->Translated.pValue );
		if( !pBlock ) {
			Token().Error( "[CodeGen] Label's translated value is not a LLVM BasicBlock" );
			return false;
		}

		CG->SetCurrentBlock( *pBlock );

		return true;
	}

	/*
	===========================================================================

		GOTO/GOSUB STATEMENT

	===========================================================================
	*/

	CGotoStmt::CGotoStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::GotoStmt, Tok, Parser )
	, m_bIsGosub( Token().IsKeyword( kKeyword_Gosub ) )
	, m_pLabelTok( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}
	
	bool CGotoStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Goto ) || Token().IsKeyword( kKeyword_Gosub ) );

		if( !Lexer().ExpectLine( ETokenType::Name ) ) {
			return false;
		}

		m_pLabelTok = &Lexer().Token();
		return true;
	}

	Ax::String CGotoStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( m_bIsGosub ? "Call <" : "Jump <" ) );
		AX_EXPECT_MEMORY( Result.Append( m_pLabelTok != nullptr ? m_pLabelTok->GetString() : "(null)" ) );
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		return Result;
	}

	bool CGotoStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLabelTok );

		m_Semanted.pLabelSym = g_Prog->FindSymbol( m_pLabelTok->GetString(), ESearchArea::ThisScopeOnly );
		if( !m_Semanted.pLabelSym || m_Semanted.pLabelSym->pLabel == nullptr ) {
			m_pLabelTok->Error( "Label does not exist: \"" + m_pLabelTok->GetString() + "\"" );
			return false;
		}

		return true;
	}
	bool CGotoStmt::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pLabelTok );
		AX_ASSERT_NOT_NULL( m_Semanted.pLabelSym );
		AX_ASSERT_NOT_NULL( m_Semanted.pLabelSym->pLabel );
		AX_ASSERT_NOT_NULL( m_Semanted.pLabelSym->Translated.pValue );

		llvm::BasicBlock *const pDestBlock = llvm::dyn_cast_or_null< llvm::BasicBlock >( m_Semanted.pLabelSym->Translated.pValue );
		if( !pDestBlock ) {
			m_pLabelTok->Error( "[CodeGen] Label's translated value is not a LLVM BasicBlock" );
			return false;
		}

		CG->Builder().CreateBr( pDestBlock );

		return true;
	}


	/*
	===========================================================================

		RETURN STATEMENT

	===========================================================================
	*/

	CGoBackStmt::CGoBackStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::ReturnStmt, Tok, Parser )
	{
	}

	bool CGoBackStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_GoBack ) );
		return true;
	}

	Ax::String CGoBackStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "ReturnFromGosub" ) );

		return Result;
	}

	bool CGoBackStmt::CodeGen()
	{
		// TODO
		Token().Error( "[CodeGen] Returning from GOSUB is not supported yet" );
		return false;
	}
	
	
	/*
	===========================================================================

		LOOP FLOW STATEMENT

	===========================================================================
	*/

	CLoopFlowStmt::CLoopFlowStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::ExitStmt, Tok, Parser )
	, m_Type( ELoopFlow::Break )
	{
	}

	bool CLoopFlowStmt::Parse()
	{
		// TODO: Perhaps include the possibility of a break count
		
		/*
			` Nested for-loops
			FOR x = 0 UNTIL res_x
				FOR y = 0 UNTIL res_y
					IF someData(x,y) < someMinimum THEN EXIT 2
				NEXT y
			NEXT x

			` "EXIT 2" from the above would land here
		*/

		const SToken &Tok = Token();

		if( Tok.IsKeyword( kKeyword_Continue ) ) {
			m_Type = ELoopFlow::Continue;
		} else {
			m_Type = ELoopFlow::Break;
		}

		AX_ASSERT( Tok.IsKeyword( kKeyword_Exit ) || Tok.IsKeyword( kKeyword_Break ) || Tok.IsKeyword( kKeyword_Continue ) );
		return true;
	}

	Ax::String CLoopFlowStmt::ToString() const
	{
		Ax::String Result;

		switch( m_Type )
		{
		case ELoopFlow::Break:
			AX_EXPECT_MEMORY( Result.Append( "BreakFromLoop" ) );
			break;

		case ELoopFlow::Continue:
			AX_EXPECT_MEMORY( Result.Append( "ContinueLoop" ) );
			break;
		}

		AX_ASSERT( !Result.IsEmpty() );

		return Result;
	}

	bool CLoopFlowStmt::CodeGen()
	{
		switch( m_Type )
		{
		case ELoopFlow::Break:
			CG->BreakLoop();
			return true;

		case ELoopFlow::Continue:
			CG->ContinueLoop();
			return true;
		}

		// TODO
		AX_ASSERT_MSG( false, "Unimplemented" );
		Token().Error( "[CodeGen] Unknown loop flow statement" );
		return false;
	}


	/*
	===========================================================================

		DO/LOOP STATEMENT

	===========================================================================
	*/
	
	CDoLoopStmt::CDoLoopStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::LoopBlock, EStmtType::DoLoopBlock, Tok, Parser )
	, m_pLoopToken( nullptr )
	{
	}

	bool CDoLoopStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Do ) );

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in DO/LOOP" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_Loop ) ) {
				m_pLoopToken = &CheckTok;
				break;
			}

			Lexer().Unlex();
			if( !Parser().ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		AX_ASSERT_NOT_NULL( m_pLoopToken );
		return true;
	}

	Ax::String CDoLoopStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "DoLoop" ) );
		
		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CDoLoopStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pLoopToken );
		return CBlockStatement::Semant();
	}
	bool CDoLoopStmt::CodeGen()
	{
		llvm::BasicBlock *const pLoopEnter = &CG->EmitLabel( "loop.body" );
		llvm::BasicBlock *const pLoopLeave = llvm::BasicBlock::Create( CG->Context(), "loop.end", &CG->CurrentFunction() );

		if( !pLoopEnter || !pLoopLeave ) {
			Token().Error( "[CodeGen] Failed to generate one of the BasicBlocks for DO/LOOP" );
			return false;
		}

		CG->EnterLoop( pLoopLeave, pLoopEnter );

		if( !CBlockStatement::CodeGen() ) {
			return false;
		}

		CG->LeaveLoop();

		llvm::ArrayRef<llvm::Value*> args;
		llvm::Value *const pCanContinue = CG->Builder().CreateCall( CG->InternalFuncs().pSafeSync, args, "cancontinue" );
		CG->Builder().CreateCondBr( pCanContinue, pLoopEnter, pLoopLeave );

		CG->SetCurrentBlock( *pLoopLeave );

		return true;
	}


	/*
	===========================================================================

		WHILE/ENDWHILE LOOP STATEMENT

	===========================================================================
	*/
	
	CWhileLoopStmt::CWhileLoopStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::LoopBlock, EStmtType::WhileBlock, Tok, Parser )
	, m_pEndwhileToken( nullptr )
	, m_pCondition( nullptr )
	{
	}

	bool CWhileLoopStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_While ) );

		m_pCondition = Parser().ParseExpression();
		if( !m_pCondition ) {
			return false;
		}

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in WHILE/ENDWHILE" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_EndWhile ) ) {
				m_pEndwhileToken = &CheckTok;
				break;
			}

			Lexer().Unlex();
			if( !Parser().ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		AX_ASSERT_NOT_NULL( m_pEndwhileToken );
		return true;
	}

	Ax::String CWhileLoopStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "WhileLoop expr:" ) );

		if( m_pCondition != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pCondition->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( " (null)" ) );
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CWhileLoopStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pCondition );
		AX_ASSERT_NOT_NULL( m_pEndwhileToken );

		if( !m_pCondition->Semant() ) {
			return false;
		}

		return CBlockStatement::Semant();
	}
	bool CWhileLoopStmt::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pCondition );

		llvm::BasicBlock *const pLoopEnter = &CG->EmitLabel( "while.begin" );
		llvm::BasicBlock *const pLoopMain = llvm::BasicBlock::Create( CG->Context(), "while.body", &CG->CurrentFunction() );
		llvm::BasicBlock *const pLoopLeave = llvm::BasicBlock::Create( CG->Context(), "while.end", &CG->CurrentFunction() );

		if( !pLoopEnter || !pLoopMain || !pLoopLeave ) {
			Token().Error( "[CodeGen] One or more BasicBlocks failed to generate for WHILE/ENDWHILE" );
			return false;
		}

		CG->EnterLoop( pLoopLeave, pLoopEnter );

		llvm::Value *const pCondVal = CG->EmitCondition( *m_pCondition );
		if( !pCondVal ) {
			return false;
		}

		CG->Builder().CreateCondBr( pCondVal, pLoopMain, pLoopLeave );

		CG->SetCurrentBlock( *pLoopMain );

		if( !CBlockStatement::CodeGen() ) {
			return false;
		}

		CG->Builder().CreateBr( pLoopEnter );
		CG->SetCurrentBlock( *pLoopLeave );

		CG->LeaveLoop();

		return true;
	}


	/*
	===========================================================================

		REPEAT/UNTIL LOOP STATEMENT

	===========================================================================
	*/
	
	CRepeatLoopStmt::CRepeatLoopStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::LoopBlock, EStmtType::RepeatBlock, Tok, Parser )
	, m_pUntilToken( nullptr )
	, m_pCondition( nullptr )
	{
	}

	bool CRepeatLoopStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Repeat ) );
		
		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in REPEAT/UNTIL" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_Until ) ) {
				m_pUntilToken = &CheckTok;
				break;
			}

			Lexer().Unlex();
			if( !Parser().ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		m_pCondition = Parser().ParseExpression();
		if( !m_pCondition ) {
			return false;
		}

		AX_ASSERT_NOT_NULL( m_pUntilToken );
		AX_ASSERT_NOT_NULL( m_pCondition );
		return true;
	}

	Ax::String CRepeatLoopStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "RepeatLoop expr:" ) );

		if( m_pCondition != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pCondition->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( " (null)" ) );
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CRepeatLoopStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pCondition );
		AX_ASSERT_NOT_NULL( m_pUntilToken );

		if( !m_pCondition->Semant() ) {
			return false;
		}

		if( !CBlockStatement::Semant() ) {
			return false;
		}

		return true;
	}
	bool CRepeatLoopStmt::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pCondition );
		AX_ASSERT_NOT_NULL( m_pUntilToken );

		llvm::BasicBlock *const pLoopEnter = &CG->EmitLabel( "repeat.body" );
		llvm::BasicBlock *const pLoopStep  = llvm::BasicBlock::Create( CG->Context(), "repeat.step", &CG->CurrentFunction() );
		llvm::BasicBlock *const pLoopLeave = llvm::BasicBlock::Create( CG->Context(), "repeat.end", &CG->CurrentFunction() );

		CG->EnterLoop( pLoopLeave, pLoopStep );

		if( !CBlockStatement::CodeGen() ) {
			return false;
		}

		CG->Builder().CreateBr( pLoopStep );
		CG->SetCurrentBlock( *pLoopStep );

		llvm::Value *const pCondVal = CG->EmitCondition( *m_pCondition );
		if( !pCondVal ) {
			return false;
		}

		CG->Builder().CreateCondBr( pCondVal, pLoopLeave, pLoopEnter );

		CG->SetCurrentBlock( *pLoopLeave );
		CG->LeaveLoop();

		return true;
	}


	/*
	===========================================================================

		FOR/NEXT LOOP STATEMENT

	===========================================================================
	*/
	
	CForLoopStmt::CForLoopStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::LoopBlock, EStmtType::ForNextBlock, Tok, Parser )
	, m_pVarToken( nullptr )
	, m_pToUntilToken( nullptr )
	, m_pStepToken( nullptr )
	, m_pNextToken( nullptr )
	, m_pInitExpr( nullptr )
	, m_pCondExpr( nullptr )
	, m_pStepExpr( nullptr )
	, m_Semant()
	{
		memset( &m_Semant, 0, sizeof( m_Semant ) );
	}

	bool CForLoopStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_For ) );

		if( !ParseInit() ) {
			return false;
		}
		if( !ParseCond() ) {
			return false;
		}
		if( !ParseStep() ) {
			return false;
		}

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in FOR/NEXT" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_Next ) ) {
				m_pNextToken = &CheckTok;

				if( Lexer().CheckLine( Ax::Parser::ETokenType::Name ) ) {
					AX_ASSERT_NOT_NULL( m_pVarToken );

					if( Lexer().Token() != *m_pVarToken ) {
						Lexer().Expected( "Expected FOR initializer name (" + m_pVarToken->GetString() + ")" );
						return false;
					}
				}

				break;
			}

			Lexer().Unlex();
			if( !Parser().ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		AX_ASSERT_NOT_NULL( m_pNextToken );
		return true;
	}

	bool CForLoopStmt::ParseInit()
	{
		const SToken &Tok = Lexer().ExpectLine( ETokenType::Name );
		if( !Tok ) {
			return false;
		}
		if( Tok.IsKeyword() ) {
			Lexer().Expected( "Expected non-keyword for variable name" );
			return false;
		}

		m_pVarToken = &Tok;

		if( !Lexer().Expect( ETokenType::Punctuation, "=" ) ) {
			return false;
		}

		m_pInitExpr = Parser().ParseExpression();
		if( !m_pInitExpr ) {
			return false;
		}

		return true;
	}
	bool CForLoopStmt::ParseCond()
	{
		const SToken &Tok = Lexer().Expect( ETokenType::Name );
		if( !Tok ) {
			return false;
		}

		if( !Tok.IsKeyword( kKeyword_To ) && !Tok.IsKeyword( kKeyword_Until ) ) {
			Lexer().Expected( "Expected TO or UNTIL" );
			return false;
		}

		m_pToUntilToken = &Tok;

		m_pCondExpr = Parser().ParseExpression();
		if( !m_pCondExpr ) {
			return false;
		}

		return true;
	}
	bool CForLoopStmt::ParseStep()
	{
		const SToken &Tok = Lexer().Check( Ax::Parser::ETokenType::Name );
		if( !Tok ) {
			return true;
		}
		if( !Tok.IsKeyword( kKeyword_Step ) ) {
			Lexer().Unlex();
			return true;
		}

		m_pStepToken = &Tok;

		m_pStepExpr = Parser().ParseExpression();
		if( !m_pStepExpr ) {
			return false;
		}

		return true;
	}

	Ax::String CForLoopStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "ForLoop var:" ) );
		AX_EXPECT_MEMORY( Result.Append( m_pVarToken != nullptr ? m_pVarToken->GetString() : "(null)" ) );
		if( m_pInitExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( "=") );
			AX_EXPECT_MEMORY( Result.Append( m_pInitExpr->ToString() ) );
		}
		if( m_pToUntilToken != nullptr ) {
			if( m_pToUntilToken->IsKeyword( kKeyword_To ) ) {
				AX_EXPECT_MEMORY( Result.Append( " <= " ) );
			} else if( m_pToUntilToken->IsKeyword( kKeyword_Until ) ) {
				AX_EXPECT_MEMORY( Result.Append( " < " ) );
			} else {
				AX_ASSERT_MSG( false, "Unreachable" );
			}
		}
		if( m_pCondExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( "expr:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pCondExpr->ToString() ) );
		} else {
			if( m_pToUntilToken != nullptr ) {
				AX_EXPECT_MEMORY( Result.Append( "expr:(null)" ) );
			}
		}
		if( m_pStepExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " step:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pStepExpr->ToString() ) );
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CForLoopStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pVarToken );
		AX_ASSERT_NOT_NULL( m_pToUntilToken );
		AX_ASSERT_NOT_NULL( m_pNextToken );
		AX_ASSERT_NOT_NULL( m_pInitExpr );
		AX_ASSERT_NOT_NULL( m_pCondExpr );

		SSymbol *pVarSym = nullptr;
		Ax::String VarName;

		AX_EXPECT_MEMORY( VarName.Assign( m_pVarToken->GetString() ) );

		m_Semant.bOwnsVar = false;
		pVarSym = const_cast< SSymbol * >( g_Prog->FindSymbol( VarName ) );
		if( !pVarSym ) {
			pVarSym = g_Prog->AddSymbol( VarName );
			if( !pVarSym ) {
				return false;
			}

			pVarSym->pDeclToken = m_pVarToken;
			pVarSym->pVar = new SMemberInfo();
			AX_EXPECT_MEMORY( pVarSym->pVar );

			m_Semant.bOwnsVar = true;

			pVarSym->pVar->Name.Swap( VarName );
			pVarSym->pVar->PassBy = EPassBy::Direct;
			pVarSym->pVar->PassMod = EPassMod::Direct;
			pVarSym->pVar->uOffset = 0;
			if( !STypeRef::Semant( pVarSym->pVar->Type, *m_pVarToken, nullptr ) ) {
				return false;
			}
		} else if( !pVarSym->pVar ) {
			m_pVarToken->Error( "Non-variable used as iterator in FOR loop" );
			return false;
		}

		if( !IsNumber( pVarSym->pVar->Type.BuiltinType ) ) {
			m_pVarToken->Error( "Non-numeric type used for iterator in FOR loop" );
			return false;
		}

		m_Semant.pVar = pVarSym;

		if( !m_pInitExpr->Semant() ) {
			return false;
		}

		const STypeRef *const pInitRTy = m_pInitExpr->GetType();
		if( !pInitRTy ) {
			m_pInitExpr->Token().Error( "No type" );
			return false;
		}

		const STypeRef *const pIterRTy = &pVarSym->pVar->Type;

		m_Semant.InitCast = STypeRef::Cast( *pInitRTy, *pIterRTy );
		if( m_Semant.InitCast == ECast::Invalid ) {
			return STypeRef::CastError( m_pInitExpr->Token(), *pInitRTy, *pIterRTy );
		}

		if( !m_pCondExpr->Semant() ) {
			return false;
		}

		const STypeRef *const pCondRTy = m_pCondExpr->GetType();
		if( !pCondRTy ) {
			m_pCondExpr->Token().Error( "No type" );
			return false;
		}

		m_Semant.CondCast = STypeRef::Cast( *pCondRTy, *pIterRTy );
		if( m_Semant.CondCast == ECast::Invalid ) {
			return STypeRef::CastError( m_pCondExpr->Token(), *pCondRTy, *pIterRTy );
		}

		if( m_pStepExpr != nullptr ) {
			if( !m_pStepExpr->Semant() ) {
				return false;
			}

			const STypeRef *const pStepRTy = m_pStepExpr->GetType();
			if( !pStepRTy ) {
				m_pStepExpr->Token().Error( "No type" );
				return false;
			}

			m_Semant.StepCast = STypeRef::Cast( *pStepRTy, *pIterRTy );
			if( m_Semant.StepCast == ECast::Invalid ) {
				return STypeRef::CastError( m_pStepExpr->Token(), *pStepRTy, *pIterRTy );
			}
		} else {
			m_Semant.StepCast = ECast::Invalid;
		}

		if( !CBlockStatement::Semant() ) {
			return false;
		}

		return true;
	}
	bool CForLoopStmt::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pVarToken );
		AX_ASSERT_NOT_NULL( m_pToUntilToken );
		AX_ASSERT_NOT_NULL( m_pNextToken );
		AX_ASSERT_NOT_NULL( m_pInitExpr );
		AX_ASSERT_NOT_NULL( m_pCondExpr );
		AX_ASSERT_NOT_NULL( m_Semant.pVar );
		AX_ASSERT( m_Semant.InitCast != ECast::Invalid );
		AX_ASSERT( m_Semant.CondCast != ECast::Invalid );

		const bool bIsUntil = m_pToUntilToken->IsKeyword( kKeyword_Until );

		llvm::LLVMContext &Context = CG->Context();
		llvm::Function *const pCurrFunc = &CG->CurrentFunction();
		llvm::IRBuilder<> &Builder = CG->Builder();

		AX_ASSERT_NOT_NULL( pCurrFunc );

		SSymbol *const pVarSym = m_Semant.pVar;
		if( m_Semant.bOwnsVar ) {
			llvm::Type *const pVarTy = pVarSym->pVar->Type.CodeGen();
			if( !pVarTy ) {
				return false;
			}

			llvm::IRBuilder<> EntryBlockBuilder( &pCurrFunc->getEntryBlock(), pCurrFunc->getEntryBlock().begin() );

			pVarSym->Translated.pValue = EntryBlockBuilder.CreateAlloca( pVarTy, nullptr, LLVMStr( m_pVarToken ) );
			if( !pVarSym->Translated.pValue ) {
				m_pVarToken->Error( "[CodeGen] Failed to generate 'alloca' instruction for loop iterator" );
				return false;
			}
		}

		AX_ASSERT_NOT_NULL( pVarSym->pVar );
		AX_ASSERT_NOT_NULL( pVarSym->Translated.pValue );

		const STypeRef &VarRTy = pVarSym->pVar->Type;
		AX_ASSERT( VarRTy.BuiltinType != EBuiltinType::Invalid );

		SValue PreInitVal = m_pInitExpr->CodeGen();
		if( !PreInitVal ) {
			return false;
		}

		llvm::Value *const pInitVal = CG->EmitCast( m_Semant.InitCast, VarRTy.BuiltinType, PreInitVal.Load() );
		if( !pInitVal ) {
			return false;
		}

		Builder.CreateStore( pInitVal, pVarSym->Translated.pValue, pVarSym->pVar->Type.IsVolatile() );

		llvm::BasicBlock *const pEnterLabel = llvm::BasicBlock::Create( Context, "for.begin", pCurrFunc );
		llvm::BasicBlock *const pLoopOrCondLabel = llvm::BasicBlock::Create( Context, "for.body", pCurrFunc );
		llvm::BasicBlock *const pStepLabel = llvm::BasicBlock::Create( Context, "for.step", pCurrFunc );
		llvm::BasicBlock *const pLeaveLabel = llvm::BasicBlock::Create( Context, "for.end", pCurrFunc );

		llvm::BasicBlock *const pLabels[ 2 ] = {
			pEnterLabel,
			pLoopOrCondLabel //bIsUntil ? pStepLabel : pLoopOrCondLabel
		};

		const bool bIsFP = pVarSym->Translated.pValue->getType()->isFloatingPointTy();

		CG->EnterLoop( pLeaveLabel, pStepLabel );

		for( unsigned int i = 0; i < 2; ++i ) {
			AX_ASSERT( i < sizeof( pLabels )/sizeof( pLabels[0] ) );
			AX_ASSERT_NOT_NULL( pLabels[ i ] );
			CG->SetCurrentBlock( *pLabels[ i ] );

			// Generate loop statements if this is the first iteration of
			// FOR/UNTIL, or the second iteration of FOR/TO
			const bool bGenBlock = bIsUntil ^ !i;

			// Generate the loop statements block
			if( bGenBlock ) {
				if( !CBlockStatement::CodeGen() ) {
					return false;
				}

				continue;
			}

			// Generate the conditional
			llvm::Value *const pVarVal = Builder.CreateLoad( pVarSym->Translated.pValue );
			SValue PreCmpVal = m_pCondExpr->CodeGen();
			if( !PreCmpVal ) {
				return false;
			}
			llvm::Value *const pCmpVal = CG->EmitCast( m_Semant.CondCast, VarRTy.BuiltinType, PreCmpVal.Load() );
			if( !pCmpVal ) {
				return false;
			}

			llvm::Value *const pCondVal =
				( bIsFP )
				? Builder.CreateFCmpOEQ( pVarVal, pCmpVal )
				: Builder.CreateICmpEQ( pVarVal, pCmpVal )
				;
			if( !pCondVal ) {
				return false;
			}

			Builder.CreateCondBr( pCondVal, pLeaveLabel, bIsUntil ? pLoopOrCondLabel : pStepLabel );
		}

		CG->SetCurrentBlock( *pStepLabel );

		EBuiltinType StepBTy = EBuiltinType::Invalid; //delete this line
		SValue PreStepVal;
		if( m_pStepExpr != nullptr ) {
			PreStepVal = m_pStepExpr->CodeGen();
			if( !PreStepVal ) {
				return false;
			}
		} else {
			PreStepVal = llvm::ConstantInt::get( llvm::Type::getInt32Ty( Context ), llvm::APInt( 32, 1, true ) );
			if( !PreStepVal ) {
				Token().Error( "[CodeGen] Failed to generate step constant" );
				return false;
			}

			m_Semant.StepCast = GetCastForTypes( EBuiltinType::Int32, VarRTy.BuiltinType, ECastMode::Input );
			if( m_Semant.StepCast == ECast::Invalid ) {
				Token().Error( "[CodeGen] Invalid implicit cast for step expression" );
				return false;
			}
		}

		AX_ASSERT( m_Semant.StepCast != ECast::Invalid );

		llvm::Value *const pStepVal = CG->EmitCast( m_Semant.StepCast, VarRTy.BuiltinType, PreStepVal.Load() );
		if( !pStepVal ) {
			Token().Error( "[CodeGen] Failed to generate step cast" );
			return false;
		}

		llvm::Value *const pLoadedVarVal = Builder.CreateLoad( pVarSym->Translated.pValue );
		llvm::Value *const pSteppedVal =
			( bIsFP )
			? Builder.CreateFAdd( pLoadedVarVal, pStepVal )
			: Builder.CreateAdd( pLoadedVarVal, pStepVal )
			;
		Builder.CreateStore( pSteppedVal, pVarSym->Translated.pValue );
		Builder.CreateBr( pEnterLabel );

		CG->SetCurrentBlock( *pLeaveLabel );
		if( pLeaveLabel != &CG->CurrentFunction().back() ) {
			pLeaveLabel->moveAfter( &CG->CurrentFunction().back() );
		}
		pStepLabel->moveBefore( pLeaveLabel );

		CG->LeaveLoop();
		return true;
	}


	/*
	===========================================================================

		SELECT/ENDSELECT STATEMENT

	===========================================================================
	*/
	
	CSelectStmt::CSelectStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::SelectBlock, EStmtType::SelectBlock, Tok, Parser )
	, m_pCaseDefaultToken( nullptr )
	, m_pEndselectToken( nullptr )
	, m_pSelectExpr( nullptr )
	{
	}

	bool CSelectStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Select ) );

		m_pSelectExpr = Parser().ParseExpression();
		if( !m_pSelectExpr ) {
			return false;
		}

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in SELECT/ENDSELECT" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_Case ) ) {
				if( !Parser().ParseCase( CheckTok, m_Stmts ) ) {
					return false;
				}

				continue;
			}
			if( CheckTok.IsKeyword( kKeyword_CaseDefault ) ) {
				if( m_pCaseDefaultToken != nullptr ) {
					CheckTok.Error( "Already have CASE DEFAULT" );
					m_pCaseDefaultToken->Report( ESeverity::Normal, "Declared here" );
					return false;
				}

				if( !Parser().ParseCase( CheckTok, m_Stmts ) ) {
					return false;
				}

				m_pCaseDefaultToken = &CheckTok;
				continue;
			}
			if( CheckTok.IsKeyword( kKeyword_EndSelect ) ) {
				m_pEndselectToken = &CheckTok;
				break;
			}

			Lexer().Expected( "Expected CASE, CASE DEFAULT, or ENDSELECT" );
			return false;
		}

		AX_ASSERT_NOT_NULL( m_pEndselectToken );
		return true;
	}

	Ax::String CSelectStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "Switch expr:" ) );
		if( m_pSelectExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pSelectExpr->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CSelectStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pSelectExpr );
		AX_ASSERT_NOT_NULL( m_pEndselectToken );

		if( !m_pSelectExpr->Semant() ) {
			return false;
		}

		if( !CBlockStatement::Semant() ) {
			return false;
		}

		return false;
	}
	bool CSelectStmt::CodeGen()
	{
		// TODO

		AX_ASSERT_NOT_NULL( m_pSelectExpr );
		AX_ASSERT_NOT_NULL( m_pEndselectToken );

		if( !m_pSelectExpr->CodeGen() ) {
			return false;
		}

		if( !CBlockStatement::CodeGen() ) {
			return false;
		}

		return false;
	}


	/*
	===========================================================================

		CASE/ENDCASE STATEMENT

	===========================================================================
	*/

	CSelectCaseStmt::CSelectCaseStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::CaseBlock, EStmtType::CaseStmt, Tok, Parser )
	, m_bIsDefault( false )
	, m_pExpr( nullptr )
	{
	}
	
	bool CSelectCaseStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Case ) || Token().IsKeyword( kKeyword_CaseDefault ) );

		if( Token().IsKeyword( kKeyword_Case ) ) {
			m_pExpr = Parser().ParseExpression();
			if( !m_pExpr ) {
				return false;
			}
		} else {
			m_bIsDefault = true;
		}

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in CASE/ENDCASE" );
				return false;
			}

			// Support the traditional "endcase"
			if( CheckTok.IsKeyword( kKeyword_EndCase ) ) {
				break;
			}
			// Allow "case" and "case default" to be used to end the current case
			if( CheckTok.IsKeyword( kKeyword_Case ) || CheckTok.IsKeyword( kKeyword_CaseDefault ) ) {
				Lexer().Unlex();
				break;
			}

			Lexer().Unlex();
			if( !Parser().ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		if( !Token().IsKeyword( kKeyword_CaseDefault ) ) {
			AX_ASSERT_NOT_NULL( m_pExpr );
		}
		return true;
	}

	Ax::String CSelectCaseStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "Case expr:" ) );
		if( m_pExpr != nullptr ) {
			AX_ASSERT( m_bIsDefault == false );
			AX_EXPECT_MEMORY( Result.Append( m_pExpr->ToString() ) );
		} else {
			AX_ASSERT( m_bIsDefault == true );
			AX_EXPECT_MEMORY( Result.Append( "default" ) );
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CSelectCaseStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pExpr );

		if( !m_pExpr->Semant() ) {
			return false;
		}

		if( !CBlockStatement::Semant() ) {
			return false;
		}

		//
		//	TODO: Does the parent select statement already have this?
		//	-     Maybe this should be done in the parent select though
		//

		return false;
	}
	bool CSelectCaseStmt::CodeGen()
	{
		// TODO

		AX_ASSERT_NOT_NULL( m_pExpr );

		if( !m_pExpr->CodeGen() ) {
			return false;
		}

		if( !CBlockStatement::CodeGen() ) {
			return false;
		}

		return false;
	}


	/*
	===========================================================================

		FALLTHROUGH STATEMENT

	===========================================================================
	*/

	CCaseFallthroughStmt::CCaseFallthroughStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::FallthroughStmt, Tok, Parser )
	{
	}

	bool CCaseFallthroughStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Fallthrough ) );
		return true;
	}

	Ax::String CCaseFallthroughStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "CaseFallthrough" ) );

		return Result;
	}

	bool CCaseFallthroughStmt::CodeGen()
	{
		// TODO
		return false;
	}


	/*
	===========================================================================

		IF/ELSEIF/ELSE/ENDIF STATEMENT

	===========================================================================
	*/

	CIfStmt::CIfStmt( const SToken &Tok, CParser &Parser )
	: CBlockStatement( EStmtSeqType::None, EStmtType::IfBlock, Tok, Parser )
	, m_bIsElse( false )
	, m_pCondition( nullptr )
	, m_ElseStmts()
	{
	}

	bool CIfStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_If ) );

		m_pCondition = Parser().ParseExpression();
		if( !m_pCondition ) {
			return false;
		}

		const SToken *pElseTok = nullptr;
		CStatementSequence *pCurrentStmts = &m_Stmts;

		for(;;) {
			const SToken &CheckTok = Lexer().Lex();
			if( !CheckTok || !CheckTok.StartsLine() ) {
				Lexer().Expected( "Expected a statement on a new line in IF/ENDIF" );
				return false;
			}

			if( CheckTok.IsKeyword( kKeyword_ElseIf ) ) {
				if( pElseTok != nullptr ) {
					Lexer().Error( "Unexpected ELSE IF after ELSE in IF/ENDIF" );
					pElseTok->Report( ESeverity::Normal, "From here" );
					return false;
				}

				//CIfStmt &ElseIfNode = m_Stmts.NewStmt< CIfStmt >( CheckTok, Parser() );
				CIfStmt *const pElseIfNode = new CIfStmt( CheckTok, Parser() );
				AX_EXPECT_MEMORY( pElseIfNode );
				pElseIfNode->SetSequence( m_Stmts );
				AX_EXPECT_MEMORY( m_ElseStmts.Append( pElseIfNode ) );

				CIfStmt &ElseIfNode = *pElseIfNode;

				ElseIfNode.m_bIsElse = true;

				ElseIfNode.m_pCondition = Parser().ParseExpression();
				if( !ElseIfNode.m_pCondition ) {
					return false;
				}

				AX_EXPECT_MEMORY( m_ElseStmts.Append( &ElseIfNode ) );
				pCurrentStmts = &ElseIfNode.m_Stmts;
				continue;
			}
			if( CheckTok.IsKeyword( kKeyword_Else ) ) {
				if( pElseTok != nullptr ) {
					Lexer().Error( "Already have ELSE in IF/ENDIF" );
					pElseTok->Report( ESeverity::Normal, "From here" );
					return false;
				}

				pElseTok = &CheckTok;

				//CIfStmt &ElseNode = m_Stmts.NewStmt< CIfStmt >( CheckTok, Parser() );
				CIfStmt *const pElseNode = new CIfStmt( CheckTok, Parser() );
				AX_EXPECT_MEMORY( pElseNode );
				pElseNode->SetSequence( m_Stmts );
				AX_EXPECT_MEMORY( m_ElseStmts.Append( pElseNode ) );

				CIfStmt &ElseNode = *pElseNode;

				ElseNode.m_bIsElse = true;
				pCurrentStmts = &ElseNode.m_Stmts;
				continue;
			}
			if( CheckTok.IsKeyword( kKeyword_EndIf ) ) {
				break;
			}

			AX_ASSERT_NOT_NULL( pCurrentStmts );

			Lexer().Unlex();
			if( !Parser().ParseStatement( *pCurrentStmts ) ) {
				return false;
			}
		}

		AX_ASSERT_NOT_NULL( m_pCondition );
		return true;
	}

	Ax::String CIfStmt::ToString() const
	{
		Ax::String Result;

		if( m_bIsElse ) {
			if( m_pCondition != nullptr ) {
				AX_EXPECT_MEMORY( Result.Append( "ElseIf expr:" ) );
			} else {
				AX_EXPECT_MEMORY( Result.Append( "Else" ) );
			}
		} else {
			AX_EXPECT_MEMORY( Result.Append( "If expr:" ) );
		}
		if( m_pCondition != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pCondition->ToString() ) );
		} else {
			if( !m_bIsElse ) {
				AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
			}
		}

		if( m_Stmts.begin() != m_Stmts.end() ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			Ax::String TempResult = m_Stmts.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		for( const CIfStmt *pElseStmt : m_ElseStmts ) {
			AX_EXPECT_MEMORY( Result.Append( "\n" ) );

			AX_ASSERT_NOT_NULL( pElseStmt );
			const CIfStmt &Stmt = *pElseStmt;

			Ax::String TempResult = Stmt.ToString();
			AX_EXPECT_MEMORY( TempResult.TabSelf() );

			AX_EXPECT_MEMORY( Result.Append( TempResult ) );
		}

		return Result;
	}

	bool CIfStmt::Semant()
	{
		AX_ASSERT( m_bIsElse || m_pCondition != nullptr );

		if( m_pCondition != nullptr && !m_pCondition->Semant() ) {
			return false;
		}

		if( !CBlockStatement::Semant() ) {
			return false;
		}

		for( CIfStmt *pElse : m_ElseStmts ) {
			AX_ASSERT_NOT_NULL( pElse );

			if( !pElse->Semant() ) {
				return false;
			}
		}

		return true;
	}
	bool CIfStmt::CodeGen()
	{
		AX_ASSERT( m_bIsElse || m_pCondition != nullptr );
		AX_ASSERT( CG->HasCurrentFunction() );

		// Create this block
		m_CodeGen.pDestBlock = llvm::BasicBlock::Create( CG->Context(), "if.then", &CG->CurrentFunction() );
		if( !m_CodeGen.pDestBlock ) {
			Token().Error( "[CodeGen] Failed to generate if block" );
			return false;
		}

		// Generate the else blocks following this (for LLVM to generate code into)
		if( !m_bIsElse ) {
			for( CIfStmt *pStmt : m_ElseStmts ) {
				AX_ASSERT_NOT_NULL( pStmt );
				AX_ASSERT( pStmt->m_bIsElse );

				pStmt->m_CodeGen.pDestBlock = llvm::BasicBlock::Create( CG->Context(), "if.else", &CG->CurrentFunction() );
				if( !pStmt->m_CodeGen.pDestBlock ) {
					Token().Error( "[CodeGen] Failed to generate else block" );
					return false;
				}
			}
		}

		// 
		llvm::BasicBlock *const pEndifBlock = llvm::BasicBlock::Create( CG->Context(), "if.end", &CG->CurrentFunction() );
		if( !pEndifBlock ) {
			Token().Error( "[CodeGen] Failed to generate endif merge block" );
			return false;
		}

		// For each if statement (including this and any following else statements)
		CIfStmt *pGenStmt = this;
		CIfStmt **ppNext = m_ElseStmts.begin();
		while( pGenStmt != nullptr ) {
			// Prepare the next if statement in advance
			CIfStmt *const pNextStmt = ppNext != m_ElseStmts.end() ? *ppNext : nullptr;
			++ppNext;

			// The next block represents where execution jumps to if this conditional fails
			llvm::BasicBlock *const pNextBlock = pNextStmt != nullptr ? pNextStmt->m_CodeGen.pDestBlock : pEndifBlock;
			AX_ASSERT_NOT_NULL( pNextBlock );

			// Generate code in this statement's local block
			AX_ASSERT_NOT_NULL( pGenStmt->m_CodeGen.pDestBlock );
			CG->SetCurrentBlock( *pGenStmt->m_CodeGen.pDestBlock );

			// Generate the conditional for this statement
			if( pGenStmt->m_pCondition != nullptr ) {
				llvm::Value *const pCondVal = CG->EmitCondition( *pGenStmt->m_pCondition );
				if( !pCondVal ) {
					Token().Error( "[CodeGen] Condition was not emitted for IF/ELSEIF" );
					return false;
				}

				llvm::BasicBlock *const pAfterCondBlock = llvm::BasicBlock::Create( CG->Context(), "stmtseq", &CG->CurrentFunction(), pNextBlock );
				if( !pAfterCondBlock ) {
					Token().Error( "[CodeGen] Failed to generate unconditional block for IF/ELSE" );
					return false;
				}

				CG->Builder().CreateCondBr( pCondVal, pAfterCondBlock, pNextBlock );
				CG->SetCurrentBlock( *pAfterCondBlock );
			}

			// Emit the primary code into this block
			if( !pGenStmt->m_Stmts.CodeGen() ) {
				return false;
			}

			// Insert a branch from this statement block to the end-if if
			// there's no terminator
			if( !CG->CurrentBlock().getTerminator() ) {
				CG->Builder().CreateBr( pEndifBlock );
			}

#if 0		// If statement generation debugging -- remove if this seems irrelevant
			printf( "[dest]\n" );
			pGenStmt->m_CodeGen.pDestBlock->dump();
			if( pGenStmt->m_CodeGen.pDestBlock != &CG->CurrentBlock() ) {
				printf( "[main]\n" );
				CG->CurrentBlock().dump();
			}
			printf( "\n" );
#endif

			// Go to the next if statement
			pGenStmt = pNextStmt;
		}

		// New code should emit after the ENDIF
		CG->SetCurrentBlock( *pEndifBlock );

		// Done
		return true;
	}


	/*
	===========================================================================

		VARIABLE DECLARATION STATEMENT

	===========================================================================
	*/
	
	CVarDeclStmt::CVarDeclStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::VarDecl, Tok, Parser )
	, m_DeclScope( EVarScope::Local )
	, m_pVarTok( nullptr )
	, m_pDimExprs( nullptr )
	, m_pType( nullptr )
	, m_pDefExpr( nullptr )
	, m_pVarSym( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}
	CVarDeclStmt::~CVarDeclStmt()
	{
		delete m_pType;
		m_pType = nullptr;

		delete m_pDimExprs;
		m_pDimExprs = nullptr;

		delete m_pDefExpr;
		m_pDefExpr = nullptr;
	}

	bool CVarDeclStmt::Parse()
	{
		//
		//	TODO: Handle "local Var1, Var2=7, Var3 as integer, Var4#=3.14"
		//
		
		const bool bDeclTokIsDim = Token().IsKeyword( kKeyword_RT_Dim );
		AX_ASSERT( Token().IsKeyword( kKeyword_Local ) || Token().IsKeyword( kKeyword_Global ) || bDeclTokIsDim );

		const SToken &PreTok = Lexer().ExpectLine( ETokenType::Name );
		if( !PreTok ) {
			return false;
		}

		const bool bPreTokIsDim = PreTok.IsKeyword( kKeyword_RT_Dim );
		const bool bIsDim = bDeclTokIsDim || bPreTokIsDim;

		if( bDeclTokIsDim && bPreTokIsDim ) {
			PreTok.Error( "Cannot name array 'dim'" );
			return false;
		}

		const SToken &FirstTok = bPreTokIsDim ? Lexer().ExpectLine( ETokenType::Name ) : PreTok;
		if( !FirstTok ) {
			return false;
		}

		m_DeclScope = Token().IsKeyword( kKeyword_Local ) ? EVarScope::Local : EVarScope::Global;

		if( bIsDim ) {
			if( FirstTok.IsKeyword() ) {
				FirstTok.Error( "Cannot use a keyword for an array name" );
				return false;
			}

			const SToken &LBrack = Lexer().Expect( ETokenType::Punctuation, "[", ETokenType::Punctuation, "(" );
			const char *const pszClose = LBrack.IsPunctuation( "[" ) ? "]" : ")";

			m_pDimExprs = Parser().ParseExpressionList();
			if( !m_pDimExprs ) {
				return false;
			}

			if( !Lexer().Expect( ETokenType::Punctuation, pszClose ) ) {
				return false;
			}

			if( Lexer().CheckKeyword( kKeyword_As ) ) {
				const SToken &TypeTok = Lexer().Expect( ETokenType::Name );
				if( !TypeTok ) {
					return false;
				}

				m_pType = new CTypeDecl( TypeTok, Parser() );
				AX_EXPECT_MEMORY( m_pType );

				if( !m_pType->Parse() ) {
					return false;
				}
			} else {
				m_pType = nullptr;
			}

			m_pVarTok = &FirstTok;
			return true;
		}

		if( Lexer().CheckLine( ETokenType::Name ) ) {
			if( Lexer().Token().IsKeyword( kKeyword_As ) ) {
				if( FirstTok.IsKeyword() ) {
					Lexer().Unlex(); //for correct error position
					Lexer().Expected( "Expected non-keyword variable name" );
					return false;
				}

				m_pVarTok = &FirstTok;
				if( !ParseType() ) {
					return false;
				}

				if( Lexer().CheckLine( ETokenType::Punctuation, "=" ) ) {
					m_pDefExpr = Parser().ParseExpression();
					if( !m_pDefExpr ) {
						return false;
					}
				}

				return true;
			}

			Lexer().Unlex();
			for(;;) {
				const SToken &NameTok = Lexer().ExpectLine( ETokenType::Name );
				if( !NameTok ) {
					return false;
				}
				if( NameTok.IsKeyword() ) {
					Lexer().Expected( "Expected non-keyword variable name" );
					return false;
				}

				// FIXME: Isn't this broken? Wtf?

				m_pType = new CTypeDecl( FirstTok, Parser() );
				AX_EXPECT_MEMORY( m_pType );

				if( !m_pType->Parse() ) {
					return false;
				}

				m_pVarTok = &NameTok;

				if( Lexer().CheckLine( ETokenType::Punctuation, "=" ) ) {
					m_pDefExpr = Parser().ParseExpression();
					if( !m_pDefExpr ) {
						return false;
					}
				}

				if( !Lexer().CheckLine( ETokenType::Punctuation, "," ) ) {
					break;
				}
			}

			return true;
		} else if( Lexer().CheckLine( ETokenType::Punctuation, "=" ) ) {
			m_pVarTok = &FirstTok;

			m_pDefExpr = Parser().ParseExpression();
			if( !m_pDefExpr ) {
				return false;
			}

			return true;
		}

		m_pVarTok = &FirstTok;
		return true;
	}
	bool CVarDeclStmt::Parse( const SToken &NameTok )
	{
		AX_ASSERT( Token().IsKeyword( "as" ) );
		AX_ASSERT( NameTok.IsName() );
		AX_ASSERT( !NameTok.IsKeyword() );

		m_pVarTok = &NameTok;

		if( !ParseType() ) {
			return false;
		}
		
		if( !!Lexer().Check( ETokenType::Punctuation, "=" ) ) {
			m_pDefExpr = Parser().ParseExpression();
			if( !m_pDefExpr ) {
				return false;
			}
		}

		return true;
	}
	bool CVarDeclStmt::ParseType()
	{
		const SToken &Tok = Lexer().CheckLine( ETokenType::Name );
		if( !Tok ) {
			return false;
		}

		m_pType = new CTypeDecl( Tok, Parser() );
		AX_EXPECT_MEMORY( m_pType );

		return m_pType->Parse();
	}

	Ax::String CVarDeclStmt::ToString() const
	{
		Ax::String Result;

		switch( m_DeclScope )
		{
		case EVarScope::Local:
			AX_EXPECT_MEMORY( Result.Append( "LocalVarDecl <" ) );
			break;
		case EVarScope::Global:
			AX_EXPECT_MEMORY( Result.Append( "GlobalVarDecl <" ) );
			break;
		}

		if( m_pVarTok != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pVarTok->GetString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}
		AX_EXPECT_MEMORY( Result.Append( ">" ) );

		if( m_pType != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " as:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pType->ToString() ) );
		}

		if( m_pDefExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " value:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pDefExpr->ToString() ) );
		}

		return Result;
	}

	bool CVarDeclStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pVarTok );

		if( m_pDimExprs != nullptr ) {
			STypeRef &UIntPtrRTy = g_Env->GetUIntPtr();

			if( m_pDimExprs->Subexpressions().Num() > 9 ) {
				m_pDimExprs->Token().Error( "Too many dimensions for array (limit=9)" );
				return false;
			}

			for( SExpr &Subexpr : m_pDimExprs->Subexpressions() ) {
				AX_ASSERT_NOT_NULL( Subexpr );

				if( !Subexpr->Semant() ) {
					return false;
				}

				const STypeRef *const pSubexprRTy = Subexpr->GetType();
				if( !pSubexprRTy ) {
					Subexpr->Token().Error( "Subexpression for array dimension has no type" );
					return false;
				}

				const ECast SubexprCastOp = STypeRef::Cast( *pSubexprRTy, UIntPtrRTy );
				if( SubexprCastOp == ECast::Invalid ) {
					return STypeRef::CastError( Subexpr->Token(), *pSubexprRTy, UIntPtrRTy );
				}

				Subexpr.Cast = SubexprCastOp;
			}
		}

		const STypeRef *pExprType = nullptr;
		if( m_pDefExpr != nullptr ) {
			if( !m_pDefExpr->Semant() ) {
				return false;
			}

			pExprType = m_pDefExpr->GetType();
			if( !pExprType ) {
				m_pDefExpr->Token().Error( "Expression does not have type" );
				return false;
			}
		}

		if( !m_pType && pExprType != nullptr ) {
			m_pTypeRef = new STypeRef();
			AX_EXPECT_MEMORY( m_pTypeRef );

			*m_pTypeRef = *pExprType;
			m_pTypeRef->Access = EAccess::ReadWrite;
			m_Semanted.CastOp = ECast::None;
		} else {
			m_pTypeRef = STypeRef::Semant( *m_pVarTok, m_pType );
			if( !m_pTypeRef ) {
				return false;
			}

			if( pExprType != nullptr ) {
				AX_ASSERT_NOT_NULL( m_pDefExpr );

				m_Semanted.CastOp = STypeRef::Cast( *pExprType, *m_pTypeRef );
				if( m_Semanted.CastOp == ECast::Invalid ) {
					return STypeRef::CastError( m_pDefExpr->Token(), *pExprType, *m_pTypeRef );
				}
			} else {
				m_Semanted.CastOp = ECast::None;
			}
		}

		if( m_pDimExprs != nullptr ) {
			STypeRef *const pRef = m_pTypeRef;
			AX_ASSERT_NOT_NULL( pRef );

			m_pTypeRef = new STypeRef();
			AX_EXPECT_MEMORY( m_pTypeRef );

			m_pTypeRef->Access = EAccess::ReadWrite;
			m_pTypeRef->bCanReorderMemAccesses = true;
			m_pTypeRef->BuiltinType = EBuiltinType::ArrayObject;
			m_pTypeRef->cBytes = g_Env->GetPointerSizeInBytes();
			m_pTypeRef->pCustomType = nullptr;
			m_pTypeRef->pRef = pRef;
		}

		m_Info.Name = m_pVarTok->GetString();
		m_Info.Type = *m_pTypeRef;
		m_Info.uOffset = ~uint32( 0 );

		m_pVarSym = g_Prog->AddSymbol( m_Info.Name );
		if( !m_pVarSym ) {
			return false;
		}

		m_pVarSym->pDeclToken = m_pVarTok;
		m_pVarSym->pVar = &m_Info;

		return true;
	}

	static int GetBuiltinTypeId( EBuiltinType BTy )
	{
		switch( BTy )
		{
		case EBuiltinType::UserDefined:		return 0;

		case EBuiltinType::Int8:			return 1;
		case EBuiltinType::Int16:			return 2;
		case EBuiltinType::Int32:			return 3;
		case EBuiltinType::Int64:			return 4;
		case EBuiltinType::Int128:			return 5;

		case EBuiltinType::UInt8:			return 7;
		case EBuiltinType::UInt16:			return 8;
		case EBuiltinType::UInt32:			return 9;
		case EBuiltinType::UInt64:			return 10;
		case EBuiltinType::UInt128:			return 11;

		case EBuiltinType::Float16:			return 13;
		case EBuiltinType::Float32:			return 14;
		case EBuiltinType::Float64:			return 15;

		case EBuiltinType::Boolean:			return 16;

		case EBuiltinType::StringObject:	return 17;
		case EBuiltinType::ArrayObject:		return 18;
		case EBuiltinType::LinkedListObject:return 19;
		case EBuiltinType::BinaryTreeObject:return 20;

		default:
			break;
		}

		return -1;
	}
	static llvm::Value *RTyToTypePtr( const STypeRef &RTy )
	{
		const int TypeId = GetBuiltinTypeId( RTy.BuiltinType );
		if( TypeId <= 0 ) {
			return nullptr;
		}

		return
			llvm::Constant::getIntegerValue
			(
				llvm::Type::getInt8PtrTy( CG->Context() ),
				llvm::APInt( 32, ( uint64_t )TypeId, false )
			);
	}

	bool CVarDeclStmt::CodeGen()
	{
		// TODO

		AX_ASSERT_NOT_NULL( m_pVarTok );
		AX_ASSERT_NOT_NULL( m_pVarSym );

		AX_ASSERT( m_Semanted.CastOp != ECast::Invalid );

		AX_ASSERT_NOT_NULL( m_pTypeRef );
		llvm::Type *const pType = m_pTypeRef->CodeGen();
		if( !pType ) {
			return false;
		}

		if( m_DeclScope == EVarScope::Global ) {
			llvm::GlobalVariable *const pGlobVar =
				new llvm::GlobalVariable
				(
					CG->Module(),
					pType,
					false,
					llvm::GlobalValue::LinkageTypes::ExternalLinkage,
					llvm::Constant::getNullValue( pType )
				);
			AX_EXPECT_MEMORY( pGlobVar );

			pGlobVar->setName( LLVMStr( m_pVarTok ) );

			m_pVarSym->Translated.pValue = pGlobVar;
		} else {
			AX_ASSERT( m_DeclScope == EVarScope::Local );

			llvm::Function &CurrFunc = CG->CurrentFunction();
			llvm::IRBuilder<> EntryBlockBuilder( &CurrFunc.getEntryBlock(), CurrFunc.getEntryBlock().begin() );

			llvm::AllocaInst *const pAlloca =
				EntryBlockBuilder.CreateAlloca( pType, nullptr, LLVMStr( *m_pVarTok ) );

			m_pVarSym->Translated.pValue = pAlloca;
		}

		AX_ASSERT_NOT_NULL( m_pVarSym->Translated.pValue );
		AX_ASSERT_NOT_NULL( m_pVarSym->pVar );

		AX_ASSERT_MSG( m_pTypeRef != &m_pVarSym->pVar->Type, "[INTERNAL] If this is true then remove the assignment after this line" );
		m_pVarSym->pVar->Type = *m_pTypeRef;
		
		if( m_pDefExpr != nullptr ) {
			CG->EnterScope();
			if( !CG->EmitStore( *m_pVarSym, *m_pDefExpr ) ) {
				CG->LeaveScope();
				return false;
			}
			CG->CleanScope();
			CG->LeaveScope();
		} else if( m_pDimExprs != nullptr ) {
			AX_ASSERT( m_pTypeRef->BuiltinType == EBuiltinType::ArrayObject );
			AX_ASSERT_NOT_NULL( m_pTypeRef->pRef );

			const SInternalFunctions &IntFns = CG->InternalFuncs();

			auto &Subexprs = m_pDimExprs->Subexpressions();

			llvm::Value *pDimsPtr = nullptr;
			llvm::Value *pDimsNum = nullptr;

			if( Subexprs.IsEmpty() ) {
				llvm::Function::arg_iterator argiter = IntFns.pArrayDim->arg_begin();

				pDimsPtr = llvm::Constant::getNullValue( argiter++->getType() );
				pDimsNum = llvm::Constant::getNullValue( argiter++->getType() );
			} else {
				const unsigned cPtrBits = g_Env->GetPointerBits();

				llvm::Type *const pUIntPtrTy = llvm::Type::getIntNTy( CG->Context(), cPtrBits );
				llvm::Value *const pArrSize =
					llvm::Constant::getIntegerValue
					(
						pUIntPtrTy,
						llvm::APInt( cPtrBits, ( uint64_t )Subexprs.Num(), false )
					);

				llvm::Function &CurrFunc = CG->CurrentFunction();
				llvm::IRBuilder<> EntryBlockBuilder( &CurrFunc.getEntryBlock(), CurrFunc.getEntryBlock().begin() );

				pDimsPtr = EntryBlockBuilder.CreateAlloca( pUIntPtrTy, pArrSize, "arraydims" );
				pDimsNum = pArrSize;

				llvm::Value *const pOne =
					llvm::Constant::getIntegerValue
					(
						pUIntPtrTy,
						llvm::APInt( cPtrBits, ( uint64_t )1, false )
					);
				for( uintptr i = 0; i < Subexprs.Num(); ++i ) {
					llvm::Value *const pElement =
						CG->Builder().CreateGEP
						(
							pDimsPtr,
							llvm::Constant::getIntegerValue
							(
								pUIntPtrTy,
								llvm::APInt( cPtrBits, ( uint64_t )i, false )
							)
						);

					SValue Val = Subexprs[ i ]->CodeGen();
					if( !Val ) {
						return false;
					}

					llvm::Value *const pVal = CG->EmitCast( Subexprs[ i ].Cast, g_Env->GetUIntPtrType(), Val.Load() );
					if( !pVal ) {
						return false;
					}

					llvm::Value *const pLen = CG->Builder().CreateAdd( pVal, pOne );

					CG->Builder().CreateStore( pLen, pElement, false );
				}
			}

			llvm::Value *Args[ 3 ];
			llvm::Type *const pPtrInt8Ty = llvm::Type::getInt8PtrTy( CG->Context() );

			// pDimensions
			Args[ 0 ] =
				CG->Builder().CreateCast
				(
					llvm::CastInst::getCastOpcode( pDimsPtr, false, pPtrInt8Ty, false ),
					pDimsPtr,
					pPtrInt8Ty
				);
			// cDimensions
			Args[ 1 ] = pDimsNum;
			// pItemType
			Args[ 2 ] = RTyToTypePtr( *m_pTypeRef->pRef );

			if( !Args[ 2 ] ) {
				Token().Error( "[CodeGen] This type of array is not yet supported" );
				return false;
			}

			m_pVarSym->Translated.pValue = CG->Builder().CreateCall( IntFns.pArrayDim, Args, LLVMStr( Token() ) );
		} else {
			CG->EmitConstruct( m_pVarSym->Translated.pValue, m_pVarSym->pVar->Type );
		}

		CG->EmitDestruct( m_pVarSym->Translated.pValue, m_pVarSym->pVar->Type );
		return true;
	}


	/*
	===========================================================================

		VARIABLE ASSIGNMENT STATEMENT

	===========================================================================
	*/

	CVarAssignStmt::CVarAssignStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::AssignVar, Tok, Parser )
	, m_pOpTok( nullptr )
	, m_CompoundOp( EBuiltinOp::None )
	, m_pVarExpr( nullptr )
	, m_pValueExpr( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}

	bool CVarAssignStmt::Parse( CExpression &VarExpr )
	{
		if( !Lexer().Expect( ETokenType::Punctuation ) ) {
			return false;
		}

		m_pOpTok = &Lexer().Token();

		if( m_pOpTok->Cmp( "=" ) ) {
			m_CompoundOp = EBuiltinOp::None;
		} else if( m_pOpTok->Cmp( "+=" ) ) {
			m_CompoundOp = EBuiltinOp::Add;
		} else if( m_pOpTok->Cmp( "-=" ) ) {
			m_CompoundOp = EBuiltinOp::Sub;
		} else if( m_pOpTok->Cmp( "*=" ) ) {
			m_CompoundOp = EBuiltinOp::Mul;
		} else if( m_pOpTok->Cmp( "/=" ) ) {
			m_CompoundOp = EBuiltinOp::Div;
		} else if( m_pOpTok->Cmp( "^=" ) ) {
			m_CompoundOp = EBuiltinOp::Pow;
		} else if( m_pOpTok->Cmp( "%%=" ) ) {
			m_CompoundOp = EBuiltinOp::Mod;
		} else if( m_pOpTok->Cmp( "<<=" ) ) {
			m_CompoundOp = EBuiltinOp::BitSL;
		} else if( m_pOpTok->Cmp( ">>=" ) ) {
			m_CompoundOp = EBuiltinOp::BitSR;
		} else if( m_pOpTok->Cmp( "&=" ) ) {
			m_CompoundOp = EBuiltinOp::BitAnd;
		} else if( m_pOpTok->Cmp( "|=" ) ) {
			m_CompoundOp = EBuiltinOp::BitOr;
		} else if( m_pOpTok->Cmp( "~=" ) ) {
			m_CompoundOp = EBuiltinOp::BitXor;
		} else if( m_pOpTok->Cmp( "~<<=" ) ) {
			m_CompoundOp = EBuiltinOp::BitRL;
		} else if( m_pOpTok->Cmp( "~>>=" ) ) {
			m_CompoundOp = EBuiltinOp::BitRR;
		} else {
			m_pOpTok->Error( "Expected assignment operator, got \"" + m_pOpTok->GetString() + "\"" );
			return false;
		}

		m_pVarExpr = &VarExpr;

		m_pValueExpr = Parser().ParseExpression( nullptr );
		if( !m_pValueExpr ) {
			return false;
		}

		return true;
	}

	Ax::String CVarAssignStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "VarAssign " ) );

		const char *pszCompoundOp = "(unknown-op)";
		switch( m_CompoundOp )
		{
		case EBuiltinOp::None:
			pszCompoundOp = "set";
			break;
		case EBuiltinOp::Add:
			pszCompoundOp = "add-set";
			break;
		case EBuiltinOp::Sub:
			pszCompoundOp = "sub-set";
			break;
		case EBuiltinOp::Mul:
			pszCompoundOp = "mul-set";
			break;
		case EBuiltinOp::Div:
			pszCompoundOp = "div-set";
			break;
		case EBuiltinOp::Pow:
			pszCompoundOp = "pow-set";
			break;
		case EBuiltinOp::Mod:
			pszCompoundOp = "mod-set";
			break;
		case EBuiltinOp::BitSL:
			pszCompoundOp = "bitwiseshiftleft-set";
			break;
		case EBuiltinOp::BitSR:
			pszCompoundOp = "bitwiseshiftright-set";
			break;
		case EBuiltinOp::BitAnd:
			pszCompoundOp = "bitwiseand-set";
			break;
		case EBuiltinOp::BitOr:
			pszCompoundOp = "bitwiseor-set";
			break;
		case EBuiltinOp::BitXor:
			pszCompoundOp = "bitwisexor-set";
			break;
		case EBuiltinOp::BitRL:
			pszCompoundOp = "bitwiserotateleft-set";
			break;
		case EBuiltinOp::BitRR:
			pszCompoundOp = "bitwiserotateright-set";
			break;
		}

		AX_EXPECT_MEMORY( Result.Append( pszCompoundOp ) );

		AX_EXPECT_MEMORY( Result.Append( " var:" ) );
		if( m_pVarExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pVarExpr->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}
		AX_EXPECT_MEMORY( Result.Append( " value:" ) );

		if( m_pValueExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( m_pValueExpr->ToString() ) );
		} else {
			AX_EXPECT_MEMORY( Result.Append( "(null)" ) );
		}

		return Result;
	}

	bool CVarAssignStmt::Semant()
	{
		AX_ASSERT_NOT_NULL( m_pVarExpr );
		AX_ASSERT_NOT_NULL( m_pValueExpr );

		if( !m_pVarExpr->Semant() ) {
			return false;
		}

		if( !m_pVarExpr->IsLValue() ) {
			m_pOpTok->Error( "Cannot perform an assignment on a non l-value expression" );
			return false;
		}

		if( !m_pValueExpr->Semant() ) {
			return false;
		}

		const STypeRef *const pVarType = m_pVarExpr->GetType();
		if( !pVarType ) {
			m_pOpTok->Error( "Left-hand expression has no type in assignment" );
			return false;
		}

		const STypeRef *const pValType = m_pValueExpr->GetType();
		if( !pValType ) {
			m_pOpTok->Error( "Right-hand expression has no type in assignment" );
			return false;
		}

		m_Semanted.CastOp = STypeRef::Cast( *pValType, *pVarType );
		if( m_Semanted.CastOp == ECast::Invalid ) {
			return STypeRef::CastError( *m_pOpTok, *pValType, *pVarType );
		}

		if( m_Semanted.CastOp != ECast::None && m_CompoundOp != EBuiltinOp::None ) {
			if( pVarType->BuiltinType == EBuiltinType::UserDefined || pValType->BuiltinType == EBuiltinType::UserDefined ) {
				m_pOpTok->Error( "Cannot use compound assignment with user defined types (UDTs)" );
				return false;
			}
		}

		m_Semanted.CastType = pVarType->BuiltinType;

		return true;
	}
	bool CVarAssignStmt::CodeGen()
	{
		AX_ASSERT_NOT_NULL( m_pVarExpr );
		AX_ASSERT_NOT_NULL( m_pValueExpr );

		AX_ASSERT( m_Semanted.CastOp != ECast::Invalid );

		CG->EnterScope();

		SValue Var = m_pVarExpr->CodeGen();
		if( !Var ) {
			return false;
		}

		SValue PrecastVal = m_pValueExpr->CodeGen();
		if( !PrecastVal ) {
			return false;
		}

		llvm::Value *const pVal = CG->EmitCast( m_Semanted.CastOp, m_Semanted.CastType, PrecastVal.Load() );
		AX_EXPECT_MEMORY( pVal );

		if( m_CompoundOp != EBuiltinOp::None ) {
			Token().Error( "[CodeGen] Compound operators not yet supported" );
			return false;
		}

		Var.Store( pVal );

		if( m_Semanted.CastOp == ECast::None && m_Semanted.CastType == EBuiltinType::StringObject ) {
			CG->RemoveCleanCall( pVal );
		}

		CG->CleanScope( pVal );
		CG->LeaveScope();

		return true;
	}

	/*
	===========================================================================

		EXITFUNCTION STATEMENT

	===========================================================================
	*/
	
	CReturnStmt::CReturnStmt( const SToken &Tok, CParser &Parser )
	: CStatement( EStmtType::ExitFunctionStmt, Tok, Parser )
	, m_pExpr( nullptr )
	, m_Semanted()
	{
		memset( &m_Semanted, 0, sizeof( m_Semanted ) );
	}

	bool CReturnStmt::Parse()
	{
		AX_ASSERT( Token().IsKeyword( kKeyword_Return ) || Token().IsKeyword( kKeyword_EndFunction ) || Token().IsPunctuation( "=>" ) );

		if( Token().IsPunctuation( "=>" ) ) {
			m_pExpr = Parser().ParseExpression();
			if( !m_pExpr) {
				Token().Error( "Expected expression following `=>`" );
				return false;
			}

			return true;
		}

		if( Lexer().LexLine() ) {
			Lexer().Unlex();

			m_pExpr = Parser().ParseExpression();
			if( !m_pExpr ) {
				return false;
			}
		}

		return true;
	}

	Ax::String CReturnStmt::ToString() const
	{
		Ax::String Result;

		AX_EXPECT_MEMORY( Result.Append( "ReturnFromFunction" ) );
		
		if( m_pExpr != nullptr ) {
			AX_EXPECT_MEMORY( Result.Append( " value:" ) );
			AX_EXPECT_MEMORY( Result.Append( m_pExpr->ToString() ) );
		}

		return Result;
	}

	bool CReturnStmt::Semant()
	{
		const SSymbol *const pOwnerSym = g_Prog->CurrentScope().GetOwner();
		AX_ASSERT_NOT_NULL( pOwnerSym );
		AX_ASSERT_NOT_NULL( pOwnerSym->pFunc );
		AX_ASSERT( pOwnerSym->pFunc->Overloads.Num() == 1 );
		const SFunctionOverload &Fn = *pOwnerSym->pFunc->Overloads.begin().Get();
		const STypeRef &ReturnType = Fn.ReturnInfo.Type;

		m_Semanted.pReturnType = &ReturnType;

		if( m_pExpr != nullptr ) {
			if( !m_pExpr->Semant() ) {
				return false;
			}

			const STypeRef *pExprType = m_pExpr->GetType();
			if( !pExprType ) {
				Token().Error( "Return value expression has no type" );
				return false;
			}

			m_Semanted.CastOp = STypeRef::Cast( *pExprType, ReturnType );
			if( m_Semanted.CastOp == ECast::Invalid ) {
				return STypeRef::CastError( m_pExpr->Token(), *pExprType, ReturnType );
			}
		} else {
			m_Semanted.CastOp = ECast::None;
		}

		return true;
	}
	bool CReturnStmt::CodeGen()
	{
		AX_ASSERT( m_Semanted.CastOp != ECast::Invalid );
		AX_ASSERT_NOT_NULL( m_Semanted.pReturnType );

		llvm::Value *pRetVal = nullptr;

		CG->EnterScope();
		
		if( m_pExpr != nullptr ) {

			SValue RetValPre = m_pExpr->CodeGen();
			if( !RetValPre ) {
				CG->LeaveScope();
				return false;
			}

			pRetVal = CG->EmitCast( m_Semanted.CastOp, m_Semanted.pReturnType->BuiltinType, RetValPre.Load() );
			if( !pRetVal ) {
				CG->LeaveScope();
				return false;
			}
		}

		CG->CleanTopLevel( pRetVal );
		CG->LeaveScope();

		if( m_Semanted.pReturnType->BuiltinType == EBuiltinType::Void ) {
			CG->Builder().CreateRetVoid();
			return true;
		}

		if( !pRetVal ) {
			pRetVal = CG->EmitDefaultInstance( *m_Semanted.pReturnType );
			if( !pRetVal ) {
				return false;
			}
		}

		CG->Builder().CreateRet( pRetVal );
		return true;
	}

}}

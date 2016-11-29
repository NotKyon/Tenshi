#include "_PCH.hpp"
#include "Parser.hpp"
#include "Environment.hpp"
#include "ExprParser.hpp"
#include "StmtParser.hpp"
#include "UDTParser.hpp"
#include "FunctionParser.hpp"
#include "Program.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;
	using namespace Ax::Parser;

	namespace Detail
	{

		const SToken &GetNullToken()
		{
			static SToken NullTok;
			return NullTok;
		}

	}

	CParser::CParser()
	: m_Lexer()
	, m_Operators()
	, m_pDictionary( const_cast< SymbolDictionary * >( &g_Env->Dictionary() ) )
	, m_Types()
	, m_FuncDecls()
	, m_Stmts()
	{
		static const SOperator operators[] = {
			{ "||", 110, EAssoc::Left, EOpType::Binary, EBuiltinOp::RelOr, false },
			{ "~~", 110, EAssoc::Left, EOpType::Binary, EBuiltinOp::RelXor, false },

			{ "&&", 120, EAssoc::Left, EOpType::Binary, EBuiltinOp::RelAnd, false },

			{ "<", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpLt, false },
			{ ">", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpGt, false },
			{ "<=", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpLe, false },
			{ ">=", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpGe, false },
			{ "==", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpEq, false },
			{ "=", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpEq, false },
			{ "!=", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpNe, false },
			{ "<>", 130, EAssoc::Left, EOpType::Binary, EBuiltinOp::CmpNe, false },

			{ "~", 140, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitXor, false },
			{ "|", 140, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitOr, false },
			{ "+", 140, EAssoc::Left, EOpType::Binary, EBuiltinOp::Add, false },
			{ "-", 140, EAssoc::Left, EOpType::Binary, EBuiltinOp::Sub, false },
			
			{ "*", 150, EAssoc::Left, EOpType::Binary, EBuiltinOp::Mul, false },
			{ "/", 150, EAssoc::Left, EOpType::Binary, EBuiltinOp::Div, false },
			{ "%%", 150, EAssoc::Left, EOpType::Binary, EBuiltinOp::Mod, false },
			{ "&", 150, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitAnd, false },

			{ "<<", 160, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitSL, false },
			{ ">>", 160, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitSR, false },
			{ "~<<", 160, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitRL, false },
			{ "~>>", 160, EAssoc::Left, EOpType::Binary, EBuiltinOp::BitRR, false },
			{ "^", 160, EAssoc::Left, EOpType::Binary, EBuiltinOp::Pow, false },

			{ "+", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::None, false },
			{ "-", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::Neg, false },
			{ "~", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::BitNot, false },
			{ "!", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::RelNot, false },
			{ "&", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::Addr, false },
			{ "*", 250, EAssoc::Right, EOpType::Unary, EBuiltinOp::Deref, false },

			{ ".", 255, EAssoc::Left, EOpType::Binary, EBuiltinOp::FieldRef, false }
		};

		AX_EXPECT_MEMORY( m_Operators.Append( operators ) );
		m_Stmts.SetType( EStmtSeqType::Program );
	}
	CParser::~CParser()
	{
	}

	bool CParser::LoadFile( const char *pszFilename, Ax::EEncoding Encoding )
	{
		return m_Lexer.LoadFile( pszFilename, Encoding );
	}
	bool CParser::LoadText( const char *pszFilename, const Ax::String &FileText )
	{
		return m_Lexer.LoadText( pszFilename, FileText );
	}

	void CParser::PushErrorToken( const SToken &Token )
	{
		AX_EXPECT_MEMORY( m_ErrorTokenStack.Append( &Token ) );
	}
	void CParser::PopErrorToken()
	{
		AX_ASSERT( m_ErrorTokenStack.Num() > 0 );
		
		if( m_ErrorTokenStack.IsEmpty() ) {
			return;
		}

		m_ErrorTokenStack.Resize( m_ErrorTokenStack.Num() - 1 );
	}

	void CParser::Error( const Ax::String &Message )
	{
		if( m_ErrorTokenStack.IsEmpty() ) {
			m_Lexer.Error( Message );
		} else {
			m_ErrorTokenStack.Last()->Error( Message );
		}
	}
	void CParser::Warn( const Ax::String &Message )
	{
		if( m_ErrorTokenStack.IsEmpty() ) {
			m_Lexer.Warn( Message );
		} else {
			m_ErrorTokenStack.Last()->Warn( Message );
		}
	}
	void CParser::Hint( const Ax::String &Message )
	{
		if( m_ErrorTokenStack.IsEmpty() ) {
			m_Lexer.Report( Ax::ESeverity::Hint, Message );
		} else {
			m_ErrorTokenStack.Last()->Report( Ax::ESeverity::Hint, Message );
		}
	}
	void CParser::Debug( const Ax::String &Message )
	{
		if( m_ErrorTokenStack.IsEmpty() ) {
			m_Lexer.Report( Ax::ESeverity::Debug, Message );
		} else {
			m_ErrorTokenStack.Last()->Report( Ax::ESeverity::Debug, Message );
		}
	}
	
	bool CParser::ParseProgram()
	{
		g_Prog->SetParser( *this );

		for(;;) {
			const SToken &tok = m_Lexer.Lex();
			if( !tok ) {
				break;
			}

			m_Lexer.Unlex();

			if( !ParseStatement( m_Stmts ) ) {
				return false;
			}
		}

		return true;
	}
	bool CParser::ParseStatement( CStatementSequence &DstSeq )
	{
		const SToken &tok = m_Lexer.Lex();

		if( !tok ) {
			m_Lexer.Error( "Expected a token" );
			return false;
		}
		if( !tok.StartsLine() ) {
			m_Lexer.Error( "Previous statement was expected to close" );
			return false;
		}

		// Statement that begins with a name
		if( tok.IsName() ) {
			CExpression *pTerminal = nullptr;

			// If this is a user word then it could be a function call, label, variable declaration...
			if( !tok.IsKeyword() ) {
				// <name> ':' -> If it's followed by a colon then this is a label
				if( !!m_Lexer.CheckLine( Ax::Parser::ETokenType::Punctuation, ":" ) ) {
					return DstSeq.NewStmt< CLabelDeclStmt >( tok, *this ).Semant();
				}

				// If it's followed by 'as' then it's a variable declaration (without "local" or "global")
				if( !!m_Lexer.Check( Ax::Parser::ETokenType::Name, "as" ) ) {
					const SToken &AsTok = m_Lexer.Token();

					auto &DeclStmt = DstSeq.NewStmt< CVarDeclStmt >( AsTok, *this );
					return DeclStmt.Parse( tok );
				}

				// This has to be a variable or function call
				m_Lexer.Unlex();
				pTerminal = ParseTerminal();
				if( !pTerminal ) {
					return false;
				}

				// Check for a variable assignment
				if( !!m_Lexer.CheckLine( Ax::Parser::ETokenType::Punctuation ) ) {
					m_Lexer.Unlex();

					auto &AssignStmt = DstSeq.NewStmt< CVarAssignStmt >( tok, *this );
					return AssignStmt.Parse( *pTerminal );
				}

				// If this is a function call then leave it alone now
				if( pTerminal->Is( EExprType::FunctionCall ) ) {
					auto &CallStmt = DstSeq.NewStmt< CFunctionCallStmt >( tok, *this );
					CallStmt.Parsed( *pTerminal );
					return true;
				}

				// This must be a function call without the parentheses
				if( !pTerminal->Is( EExprType::Name ) ) {
					pTerminal->Token().Error( "Invalid expression used as statement" );
					return false;
				}
			} else {
				AX_ASSERT_NOT_NULL( tok.pKeyword );
			
				const EKeyword KeyIdent = ( EKeyword )tok.pKeyword->Identifier;

				switch( KeyIdent )
				{
				case kKeyword_User:
					m_Lexer.Unlex();
					pTerminal = ParseTerminal();
					if( !pTerminal ) {
						return false;
					}
					break;

				case kKeyword_Null:
				case kKeyword_AtomicAdd:
				case kKeyword_AtomicAnd:
				case kKeyword_AtomicCompareSet:
				case kKeyword_AtomicDec:
				case kKeyword_AtomicFence:
				case kKeyword_AtomicInc:
				case kKeyword_AtomicOr:
				case kKeyword_AtomicSet:
				case kKeyword_AtomicSub:
				case kKeyword_AtomicXor:
				case kKeyword_Dec:
				case kKeyword_Inc:
				case kKeyword_PreDec:
				case kKeyword_PreInc:
					break;

				case kKeyword_Goto:
				case kKeyword_Gosub:
					return ParseGotoGosub( tok, DstSeq );
				
				case kKeyword_GoBack:
					return ParseReturn( tok, DstSeq );

				case kKeyword_Do:
					return ParseDoLoop( tok, DstSeq );
				case kKeyword_While:
					return ParseWhileLoop( tok, DstSeq );
				case kKeyword_Repeat:
					return ParseRepeatLoop( tok, DstSeq );
				case kKeyword_For:
					return ParseForLoop( tok, DstSeq );

				case kKeyword_Select:
					return ParseSelect( tok, DstSeq );

				case kKeyword_If:
					return ParseIf( tok, DstSeq );

				case kKeyword_Exit:
					if( !DstSeq.InLoop() && !DstSeq.Is( EStmtSeqType::CaseBlock ) ) {
						m_Lexer.Error( "EXIT can only be used in loops or CASE statements" );
						return false;
					}

					return ParseExitRepeat( tok, DstSeq );
				case kKeyword_Break:
					if( !DstSeq.InLoop() ) {
						m_Lexer.Error( "EXIT LOOP can only be used in loops" );
						return false;
					}

					return ParseExitRepeat( tok, DstSeq );
				case kKeyword_Continue:
					if( !DstSeq.InLoop() ) {
						m_Lexer.Error( "REPEAT LOOP can only be used in loops" );
						return false;
					}

					return ParseExitRepeat( tok, DstSeq );
				case kKeyword_Fallthrough:
					if( !DstSeq.Is( EStmtSeqType::CaseBlock ) ) {
						m_Lexer.Error( "FALLTHROUGH can only be used in CASE statements" );
						return false;
					}

					return ParseFallthrough( tok, DstSeq );

				case kKeyword_Local:
				case kKeyword_Global:
					return ParseVariableDeclaration( tok, DstSeq );

				case kKeyword_Type:
					if( !DstSeq.Is( EStmtSeqType::Program ) ) {
						m_Lexer.Error( "TYPEs can only be defined in the global scope" );
						return false;
					}

					return ParseUserDefinedType( tok, DstSeq );
				case kKeyword_Function:
					if( !DstSeq.Is( EStmtSeqType::Program ) ) {
						m_Lexer.Error( "FUNCTIONs can only be defined in the global scope" );
						return false;
					}

					return ParseFunction( tok, DstSeq );

				case kKeyword_Return:
					if( !DstSeq.InFunction() ) {
						m_Lexer.Error( "EXITFUNCTION can only be used inside of a FUNCTION/ENDFUNCTION block" );
						return false;
					}

					return ParseExitFunction( tok, DstSeq );

				case kKeyword_RT_Dim:
					return ParseVariableDeclaration( tok, DstSeq );

				case kKeyword_RT_Redim:
				case kKeyword_RT_Undim:
					m_Lexer.Error( "REDIM/UNDIM not yet implemented" );
					return false;

				default:
					m_Lexer.Expected( "Unexpected keyword" );
					return false;
				}
			}

			CFuncCallExpr *const pFuncInvoke = new CFuncCallExpr( tok, *this );
			AX_EXPECT_MEMORY( pFuncInvoke );

			pFuncInvoke->m_pLHS = pTerminal;

			CExpressionList *pExprList = nullptr;
			const SToken &NextTok = m_Lexer.LexLine();
			if( NextTok ) {
				const SToken *pListTok = nullptr;
				if( NextTok.IsPunctuation( "(" ) || NextTok.IsPunctuation( "[" ) ) {
					pListTok = &NextTok;
				} else {
					m_Lexer.Unlex();
				}
				pExprList = ParseExpressionList( pListTok );
			} else {
				pExprList = new CExpressionList( tok, *this );
				AX_EXPECT_MEMORY( pExprList );
			}

			if( !pExprList ) {
				delete pFuncInvoke;
				return false;
			}

			pExprList->SetParent( pFuncInvoke );
			pFuncInvoke->m_pList = pExprList;

			auto &Stmt = DstSeq.NewStmt< CFunctionCallStmt >( tok, *this );
			Stmt.Parsed( *pFuncInvoke );

			return true;
		} // is name

		// Potential auto-print statement (just a string literal / expression)
		if( tok.IsString() ) {
			m_Lexer.Unlex();

			CExpression *const pExprNode = ParseExpression();
			if( !pExprNode ) {
				return false;
			}

			auto &Stmt = DstSeq.NewStmt< CFunctionCallStmt >( tok, *this );
			Stmt.Parsed( *pExprNode );

			return true;
		}

		//
		//	TODO: Parse array declaration (DIM, UNDIM, etc)
		//
		//	TODO: Parse ENUM/ENDENUM
		//

		// Unknown
		m_Lexer.Error( "Unable to deduce statement type" );
		return false;
	}

	bool CParser::ParseGotoGosub( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CGotoStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseReturn( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CGoBackStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseDoLoop( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CDoLoopStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseWhileLoop( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CWhileLoopStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseRepeatLoop( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CRepeatLoopStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseForLoop( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CForLoopStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseSelect( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CSelectStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseIf( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CIfStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseVariableDeclaration( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CVarDeclStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseExitFunction( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CReturnStmt >( Tok, *this ).Parse();
	}

	bool CParser::ParseExitRepeat( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CLoopFlowStmt >( Tok, *this ).Parse();
	}
	bool CParser::ParseFallthrough( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CCaseFallthroughStmt >( Tok, *this ).Parse();
	}

	bool CParser::ParseUserDefinedType( const SToken &Tok, CStatementSequence &DstSeq )
	{
		if( !DstSeq.Is( EStmtSeqType::Program ) ) {
			Tok.Error( "Cannot start a TYPE outside of program scope" );
			return false;
		}

		CUserDefinedType *const pUDT = new CUserDefinedType( Tok, *this );
		AX_EXPECT_MEMORY( pUDT );

		AX_EXPECT_MEMORY( m_Types.Append( pUDT ) );

		return pUDT->Parse();
	}
	bool CParser::ParseFunction( const SToken &Tok, CStatementSequence &DstSeq )
	{
		if( !DstSeq.Is( EStmtSeqType::Program ) ) {
			Tok.Error( "Cannot start a FUNCTION outside of program scope" );
			return false;
		}

		CFunctionDecl *const pFunc = new CFunctionDecl( Tok, *this );
		AX_EXPECT_MEMORY( pFunc );

		AX_EXPECT_MEMORY( m_FuncDecls.Append( pFunc ) );

		return pFunc->Parse();
	}

	bool CParser::ParseCase( const SToken &Tok, CStatementSequence &DstSeq )
	{
		return DstSeq.NewStmt< CSelectCaseStmt >( Tok, *this ).Parse();
	}

	CExpression *CParser::ParseTerminal()
	{
		const SToken &Tok = m_Lexer.ExpectToken();
		if( !Tok ) {
			return nullptr;
		}

		if( Tok.IsPunctuation( "(" ) ) {
			CExpression *const pSubexpr = ParseExpression( nullptr );
			if( !pSubexpr ) {
				return nullptr;
			}

			if( !m_Lexer.Expect( Ax::Parser::ETokenType::Punctuation, ")" ) ) {
				delete pSubexpr;
				return nullptr;
			}

			return pSubexpr;
		}

		if( Tok.IsString() || Tok.IsNumber() ) {
			CLiteralExpr *const pNode = new CLiteralExpr( Tok, *this );
			AX_EXPECT_MEMORY( pNode );

			return pNode;
		}

		m_Lexer.Unlex();
		return ParseNameTerminal();
	}
	CExpression *CParser::ParseNameTerminal()
	{
		const SToken &Tok = m_Lexer.Expect( ETokenType::Name );
		if( !Tok ) {
			return nullptr;
		}

		CNameExpr *const pNameExpr = new CNameExpr( Tok, *this );
		AX_EXPECT_MEMORY( pNameExpr );

		CExpression *pLeftExpr = pNameExpr;
		bool bRetry;

		do {
			bRetry = false;

			// Check for a subscope
			while( m_Lexer.Check( ETokenType::Punctuation, "." ) ) {
				bRetry = true;

				if( !m_Lexer.Expect( ETokenType::Name ) ) {
					delete pLeftExpr;
					return nullptr;
				}

				CMemberExpr *const pMemberExpr = new CMemberExpr( m_Lexer.Token(), *this );
				AX_EXPECT_MEMORY( pMemberExpr );

				pMemberExpr->m_pLHS = pLeftExpr;
				pLeftExpr = pMemberExpr;
			}

			// Check for an array subscript
			if( m_Lexer.Check( ETokenType::Punctuation, "[" ) ) {
				bRetry = true;

				CArraySubscriptExpr *const pArrExpr = new CArraySubscriptExpr( m_Lexer.Token(), *this );
				AX_EXPECT_MEMORY( pArrExpr );

				pArrExpr->m_pLHS = pLeftExpr;
				pArrExpr->m_pList = ParseExpressionList( &m_Lexer.Token() );
				if( !pArrExpr->m_pList ) {
					delete pArrExpr;
					return nullptr;
				}

				pLeftExpr = pArrExpr;
			}

			// Check for a function call
			if( m_Lexer.Check( ETokenType::Punctuation, "(" ) ) {
				bRetry = true;

				CFuncCallExpr *const pCallExpr = new CFuncCallExpr( m_Lexer.Token(), *this );
				AX_EXPECT_MEMORY( pCallExpr );

				pCallExpr->m_pLHS = pLeftExpr;
				pCallExpr->m_pList = ParseExpressionList( &m_Lexer.Token() );
				if( !pCallExpr->m_pList ) {
					delete pCallExpr;
					return nullptr;
				}

				pLeftExpr = pCallExpr;
			}
		} while( bRetry );

		return pLeftExpr;
	}
	
	CExpression *CParser::ParseUnaryExpression( const Ax::TArray< SOperator > &Operators )
	{
		// Check for prefix operators
		if( !!m_Lexer.Check( Ax::Parser::ETokenType::Punctuation ) ) {
			// Find the operator being referenced (if any)
			const SOperator *pOperator = nullptr;
			for( Ax::uintptr i = 0; i < Operators.Num(); ++i ) {
				if( Operators[ i ].Type != EOpType::Unary || Operators[ i ].Assoc != EAssoc::Right ) {
					continue;
				}

				if( !m_Lexer.Token().Cmp( Operators[ i ].pszOperator ) ) {
					continue;
				}

				pOperator = Operators.Pointer( i );
				break;
			}

			// Parse the subexpression for this operator
			if( pOperator != nullptr ) {
				CUnaryExpr *const pPrefixOpNode = new CUnaryExpr( m_Lexer.Token(), *this );
				AX_EXPECT_MEMORY( pPrefixOpNode );

				pPrefixOpNode->m_bIsPostfix = false;

				CExpression *const pSubexprNode = ParseSubexpression( pOperator->iPrecedence, Operators );
				if( !pSubexprNode ) {
					delete pPrefixOpNode;
					return nullptr;
				}

				pPrefixOpNode->m_pSubexpr = pSubexprNode;
				pSubexprNode->SetParent( pPrefixOpNode );

				pPrefixOpNode->m_Operator = pOperator->BuiltinOp;

				return pPrefixOpNode;
			}

			// The token found was not a prefix operator, so put it back
			m_Lexer.Unlex();
		}

		// Grab the primary expression (typically a terminal (e.g., literal or variable))
		CExpression *pNode = ParseTerminal();
		if( !pNode ) {
			return nullptr;
		}

		CUnaryExpr *pPostfixOp = nullptr;

		// Check for postfix operators
		if( !!m_Lexer.Check( Ax::Parser::ETokenType::Punctuation ) ) {
			// Ignore postfix operators on a separate line
			if( m_Lexer.Token().StartsLine() ) {
				m_Lexer.Unlex();
				return pNode;
			}

			// Check for an appropriate postfix operator
			for( Ax::uintptr i = 0; i < Operators.Num(); ++i ) {
				if( Operators[ i ].Type != EOpType::Unary || Operators[ i ].Assoc != EAssoc::Left ) {
					continue;
				}
				if( !m_Lexer.Token().Cmp( Operators[ i ].pszOperator ) ) {
					continue;
				}

				pPostfixOp = new CUnaryExpr( m_Lexer.Token(), *this );
				AX_EXPECT_MEMORY( pPostfixOp );

				pPostfixOp->m_bIsPostfix = true;

				pPostfixOp->m_pSubexpr = pNode;
				pNode->SetParent( pPostfixOp );

				pPostfixOp->m_Operator = Operators[ i ].BuiltinOp;

				break;
			}

			// The token found was not a postfix operator, so put it back
			m_Lexer.Unlex();
		}

		// Done
		return pNode;
	}
	CExpression *CParser::ParseSubexpression( Ax::int32 iPrecedenceLevel, const Ax::TArray< SOperator > &Operators )
	{
		CExpression *pTree = ParseUnaryExpression( Operators );
		if( !pTree ) {
			return nullptr;
		}

		for(;;) {
			const SToken &Tok = m_Lexer.Check( Ax::Parser::ETokenType::Punctuation );
			if( !Tok ) {
				break;
			}

			// Check for a binary operator of the same or a higher precedence level
			const SOperator *pOperator = nullptr;
			for( Ax::uintptr i = 0; i < Operators.Num(); ++i ) {
				if( Operators[ i ].Type != EOpType::Binary || Operators[ i ].iPrecedence < iPrecedenceLevel ) {
					continue;
				}

				if( Tok.Cmp( Operators[ i ].pszOperator ) ) {
					pOperator = Operators.Pointer( i );
					break;
				}
			}

			// No binary operator of the same or higher precedence level found
			if( !pOperator ) {
				m_Lexer.Unlex();
				break;
			}

			// Set the new expected precedence level
			const int iNewPrecedenceLevel = pOperator->iPrecedence + ( pOperator->Assoc == EAssoc::Left ? 1 : 0 );
			CExpression *const pRightNode = ParseSubexpression( iNewPrecedenceLevel, Operators );
			if( !pRightNode ) {
				delete pTree;
				return nullptr;
			}

			// Create the operator node
			CBinaryExpr *const pOpNode = new CBinaryExpr( Tok, *this );
			AX_EXPECT_MEMORY( pOpNode );

			pOpNode->m_pLHS = pTree;
			pOpNode->m_pRHS = pRightNode;

			pOpNode->m_Operator = pOperator->BuiltinOp;

			pTree->SetParent( pOpNode );
			pRightNode->SetParent( pOpNode );

			pTree = pOpNode;
		}

		return pTree;
	}
	CExpression *CParser::ParseExpression( CExpression *pParentNode )
	{
		CExpression *const pExprNode = ParseSubexpression( 0, m_Operators );
		if( !pExprNode ) {
			return nullptr;
		}

		if( pParentNode != nullptr ) {
			pExprNode->SetParent( pParentNode );
		}

		return pExprNode;
	}

	CExpressionList *CParser::ParseExpressionList( const SToken *pToken )
	{
		const char *pszCheck;

		if( !pToken ) {
			pszCheck = nullptr;
		} else if( pToken->IsPunctuation( "(" ) ) {
			pszCheck = ")";
		} else if( pToken->IsPunctuation( "[" ) ) {
			pszCheck = "]";
		} else {
			AX_ASSERT_MSG( false, "pToken must be '(' or '[' for ParseExpressionList()" );
			return nullptr;
		}

		const bool bDoCheck = pszCheck != nullptr;

		const SToken &ListTok = pToken != nullptr ? *pToken : Detail::GetNullToken();

		CExpressionList *const pListNode = new CExpressionList( ListTok, *this );
		AX_EXPECT_MEMORY( pListNode );

		do {
			if( bDoCheck && m_Lexer.Check( ETokenType::Punctuation, pszCheck ) ) {
				return pListNode;
			}

			CExpression *const pExprNode = ParseExpression( pListNode );
			if( !pExprNode ) {
				delete pListNode;
				return nullptr;
			}

			AX_EXPECT_MEMORY( pListNode->m_Subexpressions.Append( pExprNode ) );
		} while( m_Lexer.Check( Ax::Parser::ETokenType::Punctuation, "," ) );

		if( bDoCheck && !m_Lexer.Expect( ETokenType::Punctuation, pszCheck ) ) {
			delete pListNode;
			return nullptr;
		}

		return pListNode;
	}

	void CParser::PrintAST()
	{
		Ax::String TypeAST;
		const auto &Types = ProgramTypes();
		for( const CUserDefinedType *const pType : Types ) {
			AX_ASSERT_NOT_NULL( pType );
			const CUserDefinedType &Type = *pType;

			AX_EXPECT_MEMORY( TypeAST.Append( Type.ToString() ) );
			AX_EXPECT_MEMORY( TypeAST.Append( "\n" ) );
		}

		Ax::String FuncAST;
		const auto &Funcs = ProgramFunctions();
		for( const CFunctionDecl *const pFunc : Funcs ) {
			AX_ASSERT_NOT_NULL( pFunc );
			const CFunctionDecl &Func = *pFunc;

			AX_EXPECT_MEMORY( FuncAST.Append( Func.ToString() ) );
			AX_EXPECT_MEMORY( FuncAST.Append( "\n" ) );
		}

		Ax::String MainAST = ProgramStatements().ToString();

		static const char *const pszTab = "   ";
		TypeAST.Replace( "\t", pszTab );
		FuncAST.Replace( "\t", pszTab );
		MainAST.Replace( "\t", pszTab );

		AX_EXPECT_MEMORY( MainAST.Append( "\n" ) );

		printf( "%s%s%s", TypeAST.CString(), FuncAST.CString(), MainAST.CString() );
	}

}}

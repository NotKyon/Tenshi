#pragma once

#include <Collections/List.hpp>

#include "Lexer.hpp"
#include "Operator.hpp"
#include "Node.hpp"
#include "Symbol.hpp"

namespace Tenshi { namespace Compiler {

	struct STypeInfo;
	class CUserDefinedType;
	class CFunctionDecl;

	typedef Ax::TArray< CUserDefinedType * >			UDTSequence;
	typedef Ax::TArray< CFunctionDecl * >				FunctionDeclSequence;

	class CNameExpr;
	class CExpressionList;

	class CParser
	{
	friend class CStatement;
	public:
		CParser();
		~CParser();

		bool LoadFile( const char *pszFilename, Ax::EEncoding Encoding = Ax::EEncoding::Unknown );
		bool LoadText( const char *pszFilename, const Ax::String &FileText );

		const UDTSequence &ProgramTypes() const;
		const FunctionDeclSequence &ProgramFunctions() const;
		const CStatementSequence &ProgramStatements() const;

		const CLexer &Lexer() const;
		CLexer &Lexer();

		void PushErrorToken( const SToken &Token );
		void PopErrorToken();

		void Error( const Ax::String &Message );
		void Warn( const Ax::String &Message );
		void Hint( const Ax::String &Message );
		void Debug( const Ax::String &Message );

		bool ParseProgram();
		bool ParseStatement( CStatementSequence &DstSeq );
		CExpression *ParseExpression( CExpression *pParentNode = nullptr );
		CExpressionList *ParseExpressionList( const SToken *pToken = nullptr );

		bool Semant();
		bool CodeGen();

		void PrintAST();

	protected:
		friend class CSelectStmt;

		bool ParseGotoGosub( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseReturn( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseDoLoop( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseWhileLoop( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseRepeatLoop( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseForLoop( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseSelect( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseIf( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseVariableDeclaration( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseExitFunction( const SToken &Tok, CStatementSequence &DstSeq );

		bool ParseExitRepeat( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseFallthrough( const SToken &Tok, CStatementSequence &DstSeq );

		bool ParseUserDefinedType( const SToken &Tok, CStatementSequence &DstSeq );
		bool ParseFunction( const SToken &Tok, CStatementSequence &DstSeq );

		bool ParseCase( const SToken &Tok, CStatementSequence &DstSeq );

		CExpression *ParseTerminal();
		CExpression *ParseNameTerminal();

		CExpression *ParseUnaryExpression( const Ax::TArray< SOperator > &Operators );
		CExpression *ParseSubexpression( Ax::int32 iPrecedenceLevel, const Ax::TArray< SOperator > &Operators );

	private:
		CLexer						m_Lexer;
		Ax::TArray< SOperator >		m_Operators;
		SymbolDictionary *			m_pDictionary;
		UDTSequence					m_Types;
		FunctionDeclSequence		m_FuncDecls;
		CStatementSequence			m_Stmts;
		Ax::TArray< const SToken * > m_ErrorTokenStack;
	};

	//========================================================================//

	inline const UDTSequence &CParser::ProgramTypes() const
	{
		return m_Types;
	}
	inline const FunctionDeclSequence &CParser::ProgramFunctions() const
	{
		return m_FuncDecls;
	}
	inline const CStatementSequence &CParser::ProgramStatements() const
	{
		return m_Stmts;
	}

	inline const CLexer &CParser::Lexer() const
	{
		return m_Lexer;
	}
	inline CLexer &CParser::Lexer()
	{
		return m_Lexer;
	}

}}

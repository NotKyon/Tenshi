#pragma once

#include "Parser.hpp"
#include "Node.hpp"
#include "Symbol.hpp"

namespace Tenshi { namespace Compiler {

	class CFunctionDecl;
	class CParameterDecl;

	struct SFunctionOverload;

	typedef Ax::TArray< CParameterDecl * >				ParameterDeclSeq;

	class CFunctionDecl
	{
	public:
		CFunctionDecl( const SToken &Tok, CParser &Parser );
		~CFunctionDecl();

		bool Parse();

		Ax::String ToString() const;

		bool PreSemant();
		bool Semant();
		bool PreCodeGen();
		bool CodeGen();

	private:
		const SToken &				m_Token;
		CParser &					m_Parser;
		CLexer &					m_Lexer;

		const SToken *				m_pNameTok;
		ParameterDeclSeq			m_Parms;
		CTypeDecl *					m_pReturnType;
		CStatementSequence			m_Stmts;

		struct
		{
			SSymbol *				pSym;
			CScope *				pScope;
			SFunctionOverload *		pOverload;
		}							m_Semanted;

		bool ParseParameters();
		bool ParseParameter();

		CParameterDecl &AllocParm();
	};

	class CParameterDecl
	{
	friend class CFunctionDecl;
	public:
		CParameterDecl();
		~CParameterDecl();

		bool HasDefault() const;

		Ax::String ToString() const;

		bool Semant();
		bool CodeGen();

	private:
		const SToken *				m_pNameTok;
		CTypeDecl *					m_pType;
		CExpression *				m_pDefault;

		struct
		{
			SSymbol *				pSym;
		}							m_Semanted;
	};

}}

#pragma once

#include "Parser.hpp"
#include "Node.hpp"
#include "Symbol.hpp"
#include "TypeInformation.hpp"

namespace Tenshi { namespace Compiler {

	class CUserDefinedType;
	class CUDTField;

	class CUserDefinedType
	{
	public:
		CUserDefinedType( const SToken &Tok, CParser &Parser );
		~CUserDefinedType();

		bool Parse();

		Ax::String ToString() const;

		bool Semant();
		bool CodeGen();

		llvm::Type *GetValueType() const;
		llvm::Type *GetPointerType() const;

	private:
		const SToken &				m_Token;
		CParser &					m_Parser;
		CLexer &					m_Lexer;
		const SToken *				m_pNameTok;
		const SToken *				m_pEndtypeTok;
		STypeInfo *					m_pTypeInfo;
		Ax::TArray< CUDTField * >	m_Fields;
		struct
		{
			llvm::StructType *		pLLVMTy;
		}							m_CodeGen;

		CUDTField &AllocField( const SToken &Tok );
	};

	class CUDTField
	{
	friend class CUserDefinedType;
	public:
		CUDTField( const SToken &Tok, CParser &Parser );
		~CUDTField();

		Ax::String ToString() const;

		bool Semant( STypeInfo &DstInfo );

	private:
		const SToken &				m_Token;
		const SToken *				m_pNameTok;
		CTypeDecl *					m_pType;
		STypeRef *					m_pRef;
		SSymbol *					m_pSym;
	};

}}

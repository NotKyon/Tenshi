#pragma once

#include <Collections/List.hpp>
#include <Collections/Dictionary.hpp>
#include <Core/Logger.hpp>

namespace Ax { namespace Parser {

	struct SToken;

}}

namespace Tenshi { namespace Compiler {

	struct SSymbol;
	class CScope;

	class CExpression;

	class CLabelDeclStmt;

	struct STypeInfo;
	struct SMemberInfo;
	struct SFunctionInfo;

	typedef Ax::TDictionary< SSymbol >					SymbolDictionary;
	typedef Ax::TDictionary< SSymbol >::SEntry			SymbolEntry;

	enum class ESearchArea
	{
		ThisScopeAndSuperScopes, 
		ThisScopeOnly
	};

	// Defines an item within a scope
	struct SSymbol
	{
		// The token this symbol was declared with (e.g., a "function" token)
		const Ax::Parser::SToken *	pDeclToken;
		// The token pointing to this symbol's name
		const Ax::Parser::SToken *	pNameToken;

		// Name of this symbol as it will appear in the user's source
		Ax::String					Name;
		// Scope this symbol was declared in
		CScope *					pDeclScope;
		// Entry to the symbol
		SymbolEntry *				pEntry;

		//
		//	All of these fields except one will be a nullptr. This wastes space
		//	but is easier to work with.
		//

		// Pointer to the user-defined-type this symbol represents (if null then not a UDT)
		STypeInfo *					pUDT;
		// Pointer to the variable this symbol represents (if null then not a variable)
		SMemberInfo *				pVar;
		// Pointer to the function this symbol represents (if null then not a function)
		SFunctionInfo *				pFunc;
		// Pointer to the label's statement
		CLabelDeclStmt *			pLabel;

		// All of these are valid after Codegen
		struct
		{
			// The LLVM Value to be used when referencing this symbol
			llvm::Value *			pValue;
		}							Translated;

		inline SSymbol()
		: pDeclToken( nullptr )
		, pNameToken( nullptr )
		, pDeclScope( nullptr )
		, pEntry( nullptr )
		, Name()
		, pUDT( nullptr )
		, pVar( nullptr )
		, pFunc( nullptr )
		, pLabel( nullptr )
		, Translated()
		{
			Translated.pValue = nullptr;
		}
		inline ~SSymbol()
		{
			if( pEntry != nullptr ) {
				pEntry->pData = nullptr;
				pEntry = nullptr;
			}
		}

		void Report( Ax::ESeverity Sev, const Ax::String &Message ) const;
		inline void Error( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Error, Message );
		}
		inline void Warn( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Warning, Message );
		}
		inline void Hint( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Hint, Message );
		}
		inline void Note( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Normal, Message );
		}
		inline void Debug( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Debug, Message );
		}
	};

	// Defines a list of symbols that are collectively related (e.g., local variables in a function)
	class CScope
	{
	public:
		CScope();
		~CScope();

		void Clear();

		const SSymbol *FindSymbol( const char *pszName, ESearchArea Area = ESearchArea::ThisScopeAndSuperScopes ) const;
		SSymbol *AddSymbol( const char *pszName );

		const SSymbol *FindSymbol( const Ax::Parser::SToken &Token, ESearchArea Area = ESearchArea::ThisScopeAndSuperScopes ) const;
		SSymbol *AddSymbol( const Ax::Parser::SToken &Token );

		const SSymbol *FindSymbol( const CExpression &Node, ESearchArea Area = ESearchArea::ThisScopeAndSuperScopes ) const;
		SSymbol *AddSymbol( const CExpression &Node );

		Ax::uintptr NumSymbols() const;
		const SSymbol *Symbol( Ax::uintptr i ) const;

		Ax::uintptr NumScopes() const;
		const CScope *Scope( Ax::uintptr i ) const;

		void SetDictionary( SymbolDictionary &Dict, const Ax::String &NamePrefix );
		CScope *AddScope();
		CScope *AddScope( const Ax::String &NamePrefix );

		void SetOwner( SSymbol &OwnerSym );
		const SSymbol *GetOwner() const;
		
		void Report( Ax::ESeverity Sev, const Ax::String &Message ) const;
		inline void Error( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Error, Message );
		}
		inline void Warn( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Warning, Message );
		}
		inline void Hint( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Hint, Message );
		}
		inline void Note( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Normal, Message );
		}
		inline void Debug( const Ax::String &Message ) const
		{
			Report( Ax::ESeverity::Debug, Message );
		}

		Ax::uint32 AllocStackSpace( Ax::uint32 cBytes );
		Ax::uint32 TotalStackSpace() const;

	private:
		// The scope that owns this one
		CScope *					m_pParent;
		// List of scopes this one owns
		Ax::TArray< CScope * >		m_Subscopes;
		// List of symbols owned by this scope
		Ax::TArray< SSymbol * >		m_Symbols;
		// Name prefix used to find symbols within this scope (in the array)
		Ax::String					m_NamePrefix;
		// Pointer to the dictionary that contains the symbols
		SymbolDictionary *			m_pDictionary;
		// Dictionary pointer to start searching from
		SymbolEntry *				m_pSearchFrom;
		// Stack space used by variables
		Ax::uint32					m_uStackSpace;
		// Symbol that owns this scope
		SSymbol *					m_pOwnerSym;
	};

	//========================================================================//

	inline void SSymbol::Report( Ax::ESeverity Sev, const Ax::String &Message ) const
	{
		if( pNameToken != nullptr ) {
			pNameToken->Report( Sev, Message );
		} else if( pDeclToken != nullptr ) {
			pDeclToken->Report( Sev, Message );
		} else if( pDeclScope != nullptr ) {
			pDeclScope->Report( Sev, Message );
		} else {
			Ax::Report( Sev, nullptr, 0, Message );
		}
	}

	//========================================================================//

	inline Ax::uintptr CScope::NumSymbols() const
	{
		return m_Symbols.Num();
	}
	inline const SSymbol *CScope::Symbol( Ax::uintptr i ) const
	{
		AX_ASSERT( i < m_Symbols.Num() );
		return m_Symbols[ i ];
	}

	inline Ax::uintptr CScope::NumScopes() const
	{
		return m_Subscopes.Num();
	}
	inline const CScope *CScope::Scope( Ax::uintptr i ) const
	{
		AX_ASSERT( i < m_Subscopes.Num() );
		return m_Subscopes[ i ];
	}

}}

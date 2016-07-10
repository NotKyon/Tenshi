#pragma once

#include <Core/Manager.hpp>

#include "Environment.hpp"
#include "Parser.hpp"
#include "Platform.hpp"
#include "TypeInformation.hpp"

namespace Tenshi { namespace Compiler {

	struct STypeInfo;

	/*
	===========================================================================

		PROGRAM

		Holds the semantic state of the program, including all symbols (type
		declarations, labels, function declarations, variables, etc), scopes
		(function definitions, type definitions, etc), and other program build
		related parameters.

		
		[[ NOTE :: CEnvironment holds the current build settings. ]]

	===========================================================================
	*/

	class CProgram
	{
	public:
		static CProgram &GetInstance();

		void SetParser( CParser &Parser );
		void ClearParser();
		bool HasParser() const;
		CParser &Parser();
		const CParser &Parser() const;

		void PushErrorToken( const SToken &Token );
		void PopErrorToken();

		void Error( const Ax::String &Message );
		void Warn( const Ax::String &Message );
		void Hint( const Ax::String &Message );
		void Debug( const Ax::String &Message );

		const SSymbol *FindSymbol( const char *pszName, ESearchArea Area = ESearchArea::ThisScopeAndSuperScopes ) const;
		SSymbol *AddSymbol( const char *pszName );

		const STypeInfo *FindUDT( const Ax::Parser::SToken &NameTok ) const;

		void PushScope( CScope &Scope );
		void PopScope();
		void PopScopeTo( CScope &Scope );

		const CScope &CurrentScope_Const() const;
		const CScope &CurrentScope() const;
		CScope &CurrentScope();

		const CScope &GlobalScope() const;
		CScope &GlobalScope();

		Ax::uint32 AddGlobalSpace( Ax::uint32 cBytes );
		Ax::uint32 TotalGlobalSpace() const;

		Ax::uint32 AddROMSpace( Ax::uint32 cBytes );
		Ax::uint32 TotalROMSpace() const;

	private:
		CParser *					m_pParser;

		CScope						m_GlobalScope;
		Ax::TArray< CScope * >		m_ScopeStack;

		Ax::uint32					m_GlobalSpace;
		Ax::uint32					m_ROMSpace;

		CProgram();
		~CProgram();

		CProgram( const CProgram & ) AX_DELETE_FUNC;
		CProgram &operator=( const CProgram & ) AX_DELETE_FUNC;
	};

	static Ax::TManager< CProgram > g_Prog;

	//========================================================================//

	inline bool CProgram::HasParser() const
	{
		return m_pParser != nullptr;
	}
	inline CParser &CProgram::Parser()
	{
		AX_ASSERT_NOT_NULL( m_pParser );
		return *m_pParser;
	}
	inline const CParser &CProgram::Parser() const
	{
		AX_ASSERT_NOT_NULL( m_pParser );
		return *m_pParser;
	}

	inline void CProgram::PushErrorToken( const SToken &Token )
	{
		Parser().PushErrorToken( Token );
	}
	inline void CProgram::PopErrorToken()
	{
		Parser().PopErrorToken();
	}

	inline void CProgram::Error( const Ax::String &Message )
	{
		Parser().Error( Message );
	}
	inline void CProgram::Warn( const Ax::String &Message )
	{
		Parser().Warn( Message );
	}
	inline void CProgram::Hint( const Ax::String &Message )
	{
		Parser().Hint( Message );
	}
	inline void CProgram::Debug( const Ax::String &Message )
	{
		Parser().Debug( Message );
	}
	
	inline const CScope &CProgram::CurrentScope_Const() const
	{
		if( m_ScopeStack.Num() > 0 ) {
			return *m_ScopeStack[ m_ScopeStack.Num() - 1 ];
		}

		return m_GlobalScope;
	}
	inline const CScope &CProgram::CurrentScope() const
	{
		return CurrentScope_Const();
	}
	inline CScope &CProgram::CurrentScope()
	{
		return const_cast< CScope & >( CurrentScope_Const() );
	}

	inline const CScope &CProgram::GlobalScope() const
	{
		return m_GlobalScope;
	}
	inline CScope &CProgram::GlobalScope()
	{
		return m_GlobalScope;
	}

}}

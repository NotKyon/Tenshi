#include "_PCH.hpp"
#include "Program.hpp"

#include "Symbol.hpp"
#include "Environment.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	CProgram &CProgram::GetInstance()
	{
		static CProgram instance;
		return instance;
	}

	CProgram::CProgram()
	: m_pParser( nullptr )
	, m_GlobalScope()
	, m_ScopeStack()
	, m_GlobalSpace( 0 )
	, m_ROMSpace( 0 )
	{
		m_GlobalScope.SetDictionary( g_Env->Dictionary(), Ax::String() );
	}
	CProgram::~CProgram()
	{
	}

	void CProgram::SetParser( CParser &Parser )
	{
		ClearParser();
		m_pParser = &Parser;
	}
	void CProgram::ClearParser()
	{
		m_pParser = nullptr;
		m_GlobalScope.Clear();
		m_ScopeStack.Clear();
		m_GlobalSpace = 0;
		m_ROMSpace = 0;
	}

	uint32 CProgram::AddGlobalSpace( uint32 cBytes )
	{
		uint32 g;

		g = m_GlobalSpace;
		if( g + cBytes < g ) {
			if( m_pParser != nullptr ) {
				m_pParser->Error( "Too much global memory storage used" );
			}

			return ~uint32( 0 );
		}

		m_GlobalSpace += cBytes;
		return g;
	}
	uint32 CProgram::TotalGlobalSpace() const
	{
		return m_GlobalSpace;
	}

	uint32 CProgram::AddROMSpace( uint32 cBytes )
	{
		uint32 g;

		g = m_ROMSpace;
		if( g + cBytes < g ) {
			if( m_pParser != nullptr ) {
				m_pParser->Error( "Too much read-only storage used" );
			}

			return ~uint32( 0 );
		}

		m_ROMSpace += cBytes;
		return g;
	}
	uint32 CProgram::TotalROMSpace() const
	{
		return m_ROMSpace;
	}

	void CProgram::PushScope( CScope &Scope )
	{
		AX_EXPECT_MEMORY( m_ScopeStack.Append( &Scope ) );
	}
	void CProgram::PopScope()
	{
		AX_ASSERT_MSG( m_ScopeStack.Num() > 0, "Stack is empty" );
		m_ScopeStack.Resize( m_ScopeStack.Num() - 1 );
	}
	void CProgram::PopScopeTo( CScope &Scope )
	{
		AX_ASSERT_MSG( m_ScopeStack.Num() > 0 && ( &Scope != &m_GlobalScope ), "Stack is empty" );

		if( &Scope == &m_GlobalScope ) {
			m_ScopeStack.Clear();
			return;
		}

		for( Ax::uintptr i = m_ScopeStack.Num(); i > 0; --i ) {
			if( m_ScopeStack[ i - 1 ] == &Scope ) {
				m_ScopeStack.Resize( i );
				return;
			}
		}

		AX_ASSERT_MSG( false, "Scope not found in stack" );
	}

	const SSymbol *CProgram::FindSymbol( const char *pszName, ESearchArea Area ) const
	{
		AX_ASSERT_NOT_NULL( pszName );

		return CurrentScope_Const().FindSymbol( pszName, Area );
	}
	SSymbol *CProgram::AddSymbol( const char *pszName )
	{
		AX_ASSERT_NOT_NULL( pszName );

		const SSymbol *const pExistingSym = FindSymbol( pszName );
		if( pExistingSym != nullptr ) {
			Parser().Error( Ax::String( pszName ).Quote() + " already exists" );
			if( pExistingSym->pDeclToken != nullptr ) {
				pExistingSym->pDeclToken->Report( Ax::ESeverity::Normal, "Declared here" );
			}

			return nullptr;
		}

		SSymbol *const pNewSym = CurrentScope().AddSymbol( pszName );
		if( !pNewSym ) {
			Parser().Error( Ax::String( pszName ).Quote() + " is an ill-formatted name" );
			return nullptr;
		}

		return pNewSym;
	}

	const STypeInfo *CProgram::FindUDT( const Ax::Parser::SToken &NameTok ) const
	{
		const SSymbol *const pSym = m_GlobalScope.FindSymbol( NameTok );
		if( !pSym ) {
			return nullptr;
		}

		if( !pSym->pUDT ) {
			return nullptr;
		}

		return pSym->pUDT;
	}

}}

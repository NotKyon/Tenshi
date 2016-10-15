#include "_PCH.hpp"
#include "Symbol.hpp"
#include "Node.hpp"

namespace Tenshi { namespace Compiler {

	using namespace Ax;

	CScope::CScope()
	: m_pParent( nullptr )
	, m_Subscopes()
	, m_Symbols()
	, m_NamePrefix()
	, m_pDictionary( nullptr )
	, m_pSearchFrom( nullptr )
	, m_pOwnerSym( nullptr )
	{
	}
	CScope::~CScope()
	{
		for( uintptr i = m_Subscopes.Num(); i > 0; --i ) {
			delete m_Subscopes[ i - 1 ];
			m_Subscopes.Resize( i - 1 );
		}

		if( m_pParent != nullptr ) {
			for( uintptr i = m_pParent->m_Subscopes.Num(); i > 0; --i ) {
				if( m_pParent->m_Subscopes[ i - 1 ] == this ) {
					m_pParent->m_Subscopes[ i - 1 ] = nullptr;
					break;
				}
			}
		}
	}

	void CScope::Clear()
	{
		for( SSymbol *pSym : m_Symbols ) {
			delete pSym;
			pSym = nullptr;
		}
		m_Symbols.Clear();

		for( CScope *&pScope : m_Subscopes ) {
			delete pScope;
			pScope = nullptr;
		}
		m_Subscopes.Clear();
	}

	const SSymbol *CScope::FindSymbol( const char *pszName, ESearchArea Area ) const
	{
		AX_ASSERT_NOT_NULL( m_pDictionary );
		AX_ASSERT_NOT_NULL( pszName );

		const CScope *pScope = this;
		do {
			SymbolEntry *pEntry = nullptr;

			if( pScope->m_pDictionary != nullptr ) {
				if( pScope->m_pSearchFrom != nullptr ) {
					pEntry = pScope->m_pDictionary->FindFrom( pszName, *pScope->m_pSearchFrom );
				} else {
					pEntry = pScope->m_pDictionary->Find( pszName );
				}
			}

			if( pEntry != nullptr && pEntry->pData != nullptr ) {
				return pEntry->pData;
			}

			if( Area == ESearchArea::ThisScopeOnly ) {
				break;
			}

			pScope = pScope->m_pParent;
		} while( pScope != nullptr );

		return nullptr;
	}
	SSymbol *CScope::AddSymbol( const char *pszName )
	{
		AX_ASSERT_NOT_NULL( m_pDictionary );
		AX_ASSERT_NOT_NULL( pszName );

		SymbolEntry *const pEntry = m_pSearchFrom != nullptr ? m_pDictionary->LookupFrom( pszName, *m_pSearchFrom ) : m_pDictionary->Lookup( pszName );
		if( !pEntry || pEntry->pData != nullptr ) {
			return nullptr;
		}

		pEntry->pData = new SSymbol();
		AX_EXPECT_MEMORY( pEntry->pData );

		pEntry->pData->pDeclScope = this;
		pEntry->pData->pEntry = pEntry;

		AX_EXPECT_MEMORY( m_Symbols.Append( pEntry->pData ) );

		return pEntry->pData;
	}

	const SSymbol *CScope::FindSymbol( const Ax::Parser::SToken &Token, ESearchArea Area ) const
	{
		AX_ASSERT_NOT_NULL( Token.pSource );
		AX_ASSERT( Token.cLength > 0 );

		const char *const pszBase = Token.pSource->GetText();
		AX_ASSERT_NOT_NULL( pszBase );

		const char *const s = pszBase + Token.uOffset;
		const char *const e = s + Token.cLength;

		const char chBackup = *e;

		*const_cast< char * >( e ) = '\0';
		const SSymbol *const pSym = FindSymbol( s, Area );
		*const_cast< char * >( e ) = chBackup;

		return pSym;
	}
	SSymbol *CScope::AddSymbol( const Ax::Parser::SToken &Token )
	{
		AX_ASSERT_NOT_NULL( Token.pSource );
		AX_ASSERT( Token.cLength > 0 );

		const char *const pszBase = Token.pSource->GetText();
		AX_ASSERT_NOT_NULL( pszBase );

		const char *const s = pszBase + Token.uOffset;
		const char *const e = s + Token.cLength;

		const char chBackup = *e;

		*const_cast< char * >( e ) = '\0';
		SSymbol *const pSym = AddSymbol( s );
		*const_cast< char * >( e ) = chBackup;

		return pSym;
	}

	const SSymbol *CScope::FindSymbol( const CExpression &Node, ESearchArea Area ) const
	{
		return FindSymbol( Node.Token(), Area );
	}
	SSymbol *CScope::AddSymbol( const CExpression &Node )
	{
		return AddSymbol( Node.Token() );
	}

	void CScope::SetDictionary( SymbolDictionary &Dict, const Ax::String &NamePrefix )
	{
		m_pDictionary = &Dict;
		AX_EXPECT_MEMORY( m_NamePrefix.Assign( NamePrefix ) );

		if( !NamePrefix.IsEmpty() ) {
			m_pSearchFrom = Dict.Lookup( NamePrefix );
			// Assert should trigger on development builds of compiler in case NamePrefix is ill-formatted
			AX_ASSERT_NOT_NULL( m_pSearchFrom );
			// Expect will trigger otherwise, as a lack of memory could also explain the nullptr
			AX_EXPECT_MEMORY( m_pSearchFrom );
		} else {
			m_pSearchFrom = nullptr;
		}
	}
	CScope *CScope::AddScope()
	{
		CScope *const pScope = new CScope();
		AX_EXPECT_MEMORY( pScope );

		AX_EXPECT_MEMORY( m_Subscopes.Append( pScope ) );
		pScope->m_pParent = this;

		return pScope;
	}
	CScope *CScope::AddScope( const Ax::String &NamePrefix )
	{
		CScope *const pScope = new CScope();
		AX_EXPECT_MEMORY( pScope );

		AX_EXPECT_MEMORY( m_Subscopes.Append( pScope ) );

		pScope->m_pDictionary = m_pDictionary;
		AX_EXPECT_MEMORY( pScope->m_NamePrefix.Assign( NamePrefix ) );

		pScope->m_pParent = this;

		if( m_pDictionary != nullptr && !NamePrefix.IsEmpty() ) {
			pScope->m_pSearchFrom = m_pDictionary->Lookup( NamePrefix );
			// Assert should trigger on development builds of compiler in case NamePrefix is ill-formatted
			AX_ASSERT_NOT_NULL( pScope->m_pSearchFrom );
			// Expect will trigger otherwise, as a lack of memory could also explain the nullptr
			AX_EXPECT_MEMORY( pScope->m_pSearchFrom );
		}

		return pScope;
	}

	void CScope::SetOwner( SSymbol &OwnerSym )
	{
		AX_ASSERT_IS_NULL( m_pOwnerSym );
		m_pOwnerSym = &OwnerSym;
	}
	const SSymbol *CScope::GetOwner() const
	{
		return m_pOwnerSym;
	}

	Ax::uint32 CScope::AllocStackSpace( Ax::uint32 cBytes )
	{
		const Ax::uint32 s = m_uStackSpace;

		if( s + cBytes < s ) {
			return ~Ax::uint32( 0 );
		}

		m_uStackSpace += cBytes;
		return s;
	}
	Ax::uint32 CScope::TotalStackSpace() const
	{
		return m_uStackSpace;
	}

}}

#pragma once

#include "../Platform/BuildConf.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Types.hpp"
#include "../Allocation/New.hpp"

#include <stdio.h>

#define AX_DICT_ALPHALOWER "abcdefghijklmnopqrstuvwxyz"
#define AX_DICT_ALPHAUPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define AX_DICT_DIGITS "0123456789"
#define AX_DICT_MISC "_"

#define AX_DICT_ALPHA AX_DICT_ALPHALOWER AX_DICT_ALPHAUPPER
#define AX_DICT_ALPHANUM AX_DICT_ALPHA AX_DICT_DIGITS

#define AX_DICT_IDENT AX_DICT_ALPHANUM AX_DICT_MISC

namespace Ax
{

	enum class EFindOption
	{
		ExistingOnly,
		CreateIfNotExist
	};

	template< typename tElement >
	class TDictionary
	{
	public:
		struct SEntry
		{
			tElement *				pData;
			SEntry *				pEntries;

			inline SEntry *Alloc( uint8 cNumEntries, int memtag )
			{
				if( pEntries != nullptr ) {
					return pEntries;
				}

				const size_t n = sizeof( SEntry )*size_t( cNumEntries );
				pEntries = ( SEntry * )Ax::Alloc( n, memtag );
				if( !AX_VERIFY_NOT_NULL( pEntries ) ) {
					return nullptr;
				}

				memset( ( void * )pEntries, 0, n );

				return pEntries;
			}
			inline SEntry *Dealloc( uint8 cNumEntries )
			{
				if( !pEntries ) {
					return nullptr;
				}

				for( uint8 i = 0; i < cNumEntries; ++i ) {
					pEntries[ i ].Dealloc( cNumEntries );
				}

				pEntries = ( SEntry * )Ax::Dealloc( ( void * )pEntries );
				return nullptr;
			}
		};

		TDictionary( const SMemtag &memtag = SMemtag() );
		~TDictionary();

		inline bool IsInitialized() const { return m_cEntries > 0; }
		bool Init( const char *pAllowed, ECase casing = ECase::Sensitive );

		SEntry *Find( const char *pKey ) const;
		SEntry *Lookup( const char *pKey );

		SEntry *FindFrom( const char *pszKey, SEntry &Entry ) const;
		SEntry *LookupFrom( const char *pszKey, SEntry &Entry );

		void Purge();
		bool IsValidChar( char ch ) const;

	protected:
		SEntry *m_pEntries;

		uint8 m_Convmap[ 256 ];
		uint8 m_cEntries;
		
		const int m_iTag;

		uint8 GenerateConvmap( const char *pAllowed, ECase casing );

		SEntry *FindFromEntry( SEntry *&pEntries, const char *pStr, EFindOption opt );

	private:
		TDictionary( const TDictionary & ) = delete;
		TDictionary &operator=( const TDictionary & ) = delete;
	};

	template< typename tElement >
	inline uint8 TDictionary< tElement >::GenerateConvmap( const char *pAllowed, ECase casing )
	{
		uint8 i, j, k;
		uint8 bias;

		for( i = 0; i< 255; i++ ) {
			m_Convmap[ i ] = 0xFF;
		}

		bias = 0;
		for( i = 0; pAllowed[ i ] && i< 255; i++ ) {
			k = ( ( uint8 * )pAllowed )[ i ];
			if( k >= 255 ) {
				return 0;
			}

			if( casing == ECase::Insensitive ) {
				if( k >= 'a' && k <= 'z' ) {
					if( m_Convmap[ k - 'a' + 'A' ]  != 0xFF ) {
						bias++;
						continue;
					}
				} else if( k >= 'A' && k <= 'Z' ) {
					if( m_Convmap[ k - 'A' + 'a' ]  != 0xFF ) {
						bias++;
						continue;
					}
				}
			}

			m_Convmap[ k ] = i - bias;
		}

		i -= bias;

		for( j = 0; j < i; ++j ) {
			for( k = j + 1; k < i; ++k ) {
				if( pAllowed[ j ] == pAllowed[ k ] ) {
					return 0; //duplicate pEntries
				}
			}
		}

		// allow for case insensitivity by adjusting the conversion-map to point
		// to the same pEntries for different characters
		if( casing == ECase::Insensitive ) {
			if( m_Convmap[ 'a' ] != 0xFF ) {
				for( j = 'A'; j <= 'Z'; ++j ) {
					if( m_Convmap[ j ] == 0xFF ) {
						continue;
					}

					--i;
				}

				for( j = 'A'; j <= 'Z'; ++j ) {
					m_Convmap[ j ] = m_Convmap[ j - 'A' + 'a' ];
				}
			} else if( m_Convmap[ 'A' ] != 0xFF ) {
				for( j = 'a'; j <= 'z'; ++j ) {
					if( m_Convmap[ j ] == 0xFF ) {
						continue;
					}

					--i;
				}

				for( j = 'a'; j <= 'z'; ++j ) {
					m_Convmap[ j ] = m_Convmap[ j - 'a' + 'A' ];
				}
			} else {
				return 0;
			}
		}

		return i;
	}

	template< typename tElement >
	inline typename TDictionary< tElement >::SEntry *TDictionary< tElement >::FindFromEntry( SEntry *&pEntries, const char *pStr, EFindOption opt )
	{
		AX_ASSERT_NOT_NULL( m_pEntries );
		AX_ASSERT_NOT_NULL( pEntries );
		AX_ASSERT_NOT_NULL( pStr );
		AX_ASSERT( m_cEntries > 0 );

		AX_STATIC_SUPPRESS( 6011 ) //pStr is already checked from assert
		const uint8 i = m_Convmap[ *( uint8 * )pStr++ ];
		if( i == 0xFF ) {
			return nullptr;
		}

		if( *pStr == '\0' ) {
			return &pEntries[ i ];
		}

		AX_STATIC_SUPPRESS( 6011 ) //pEntries is not NULL -- assert above
		if( !pEntries[ i ].pEntries ) {
			if( opt != EFindOption::CreateIfNotExist ) {
				return nullptr;
			}

			if( !pEntries[ i ].Alloc( m_cEntries, m_iTag ) ) {
				return nullptr;
			}
		}

		return FindFromEntry( pEntries[ i ].pEntries, pStr, opt );
	}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4351)
#endif
	template< typename tElement >
	inline TDictionary< tElement >::TDictionary( const SMemtag &memtag )
	: m_pEntries( nullptr )
	, m_Convmap()
	, m_cEntries( 0 )
	, m_iTag( memtag )
	{
	}
#ifdef _MSC_VER
# pragma warning(pop)
#endif
	template< typename tElement >
	inline TDictionary< tElement >::~TDictionary()
	{
		Purge();

		Ax::Dealloc( ( void * )m_pEntries );
		m_pEntries = nullptr;
	
		m_cEntries = 0;
	}

	template< typename tElement >
	inline bool TDictionary< tElement >::Init( const char *pAllowed, ECase casing )
	{
		AX_ASSERT_MSG( m_cEntries == 0, "Already initialized" );
		AX_ASSERT_NOT_NULL( pAllowed );

		m_cEntries = GenerateConvmap( pAllowed, casing );
		AX_ASSERT_MSG( m_cEntries != 0, "Invalid allowable characters string" );

		const size_t n = sizeof( SEntry )*size_t( m_cEntries );
		m_pEntries = ( SEntry * )Ax::Alloc( n, m_iTag );
		if( !AX_VERIFY_NOT_NULL( m_pEntries ) ) {
			m_cEntries = 0;
			return false;
		}

		memset( ( void * )m_pEntries, 0, n );

		return true;
	}

	template< typename tElement >
	inline typename TDictionary< tElement >::SEntry *TDictionary< tElement >::Find( const char *pKey ) const
	{
		return const_cast< TDictionary< tElement > * >( this )->FindFromEntry( const_cast< TDictionary< tElement >::SEntry *& >( m_pEntries ), pKey, EFindOption::ExistingOnly );
	}
	template< typename tElement >
	inline typename TDictionary< tElement >::SEntry *TDictionary< tElement >::Lookup( const char *pKey )
	{
		return FindFromEntry( m_pEntries, pKey, EFindOption::CreateIfNotExist );
	}

	template< typename tElement >
	inline typename TDictionary< tElement >::SEntry *TDictionary< tElement >::FindFrom( const char *pszKey, SEntry &Entry ) const
	{
		if( !Entry.pEntries ) {
			return nullptr;
		}

		return const_cast< TDictionary< tElement > * >( this )->FindFromEntry( const_cast< TDictionary< tElement >::SEntry *& >( Entry.pEntries ), pszKey, EFindOption::ExistingOnly );
	}
	template< typename tElement >
	inline typename TDictionary< tElement >::SEntry *TDictionary< tElement >::LookupFrom( const char *pszKey, SEntry &Entry )
	{
		if( !Entry.pEntries && !Entry.Alloc( m_cEntries, m_iTag ) ) {
			return nullptr;
		}

		return FindFromEntry( Entry.pEntries, pszKey, EFindOption::CreateIfNotExist );
	}

	template< typename tElement >
	inline void TDictionary< tElement >::Purge()
	{
		for( uint8 i = 0; i < m_cEntries; ++i ) {
			m_pEntries[ i ].Dealloc( m_cEntries );
			m_pEntries[ i ].pData = nullptr;
		}
	}
	
	template< typename tElement >
	inline bool TDictionary< tElement >::IsValidChar( char ch ) const
	{
		return m_Convmap[ ( uint8 )ch ] != 0xFF;
	}

}

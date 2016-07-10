#pragma once

#include "../Allocation/New.hpp"
#include "../Allocation/Allocator.hpp"

namespace Ax
{

	template< typename TElement >
	class TArray
	{
	public:
		static const size_t kDefaultGranularity = sizeof( TElement ) > 512 ? 1 : 512/sizeof( TElement );

		TArray( const SMemtag &memtag = SMemtag() );
		TArray( const TArray &arr, const SMemtag &memtag = SMemtag() );
		template< size_t N >
		TArray( const TElement( &arr )[ N ], const SMemtag &memtag = SMemtag() )
		: m_cArr( 0 )
		, m_cMax( 0 )
		, m_pArr( NULL )
		, m_cGranularity( kDefaultGranularity )
		, m_iTag( memtag )
		{
			Append( arr );
		}
		TArray( size_t cItems, const TElement *pItems, const SMemtag &memtag = SMemtag() );
#if AX_CXX_N2118
		inline TArray( TArray &&x )
		: m_cArr( x.m_cArr )
		, m_cMax( x.m_cMax )
		, m_pArr( x.m_pArr )
		, m_cGranularity( x.m_cGranularity )
		, m_iTag( x.m_iTag )
		{
			x.m_cArr = 0;
			x.m_cMax = 0;
			x.m_pArr = nullptr;
		}
#endif
		~TArray();

		inline int GetMemtag() const
		{
			return m_iTag;
		}

		inline bool IsEmpty() const { return m_cArr == 0; }
		inline size_t Num() const { return m_cArr; }
		inline size_t NumAllocated() const { return m_cMax; }

		void Clear();
		void Purge();

		bool Resize( size_t size, const TElement &x = TElement() );
		bool Reserve( size_t size );
		bool SetAllocated( size_t size );

		inline void SetGranularity( size_t granularity ) { m_cGranularity = granularity; }
		inline size_t GetGranularity() const { return m_cGranularity; }

		bool Assign( size_t cItems, const TElement *pItems );
		inline bool Assign( const TElement &x ) { return Assign( 1, &x ); }
		template< size_t N >
		inline bool Assign( const TElement( &arr )[ N ] ) { return Assign( N, arr ); }
		inline bool Assign( const TArray &arr, size_t first = 0, intptr cItems = -1 ) { return Assign( cItems < 0 ? arr.Num() + 1 + cItems : cItems, arr.Pointer( first ) ); }

		bool Append( size_t cItems, const TElement *pItems );
		inline bool Append( const TElement &x ) { return Append( 1, &x ); }
		template< size_t N >
		inline bool Append( const TElement( &arr )[ N ] ) { return Append( N, arr ); }
		inline bool Append( const TArray &arr, size_t first = 0, intptr cItems = -1 ) { return Append( cItems < 0 ? arr.Num() + 1 + cItems : cItems, arr.Pointer( first ) ); }
		inline bool Append() { return Resize( m_cArr + 1 ); }

		bool Prepend( size_t cItems, const TElement *pItems );
		inline bool Prepend( const TElement &x ) { return Prepend( 1, &x ); }
		template< size_t N >
		inline bool Prepend( const TElement( &arr )[ N ] ) { return Prepend( N, arr ); }
		inline bool Prepend( const TArray &arr, size_t first = 0, intptr cItems = -1 ) { return Prepend( cItems < 0 ? arr.Num() + 1 + cItems : cItems, arr.Pointer( first ) ); }

		bool Insert( size_t cItems, const TElement *pItems, TElement *before );
		inline bool Insert( const TElement &x, TElement *before ) { return Insert( 1, &x, before ); }
		template< size_t N >
		inline bool Insert( const TElement( &arr )[ N ], TElement *before ) { return Insert( N, arr, before ); }
		inline bool Insert( const TArray &arr, TElement *before, size_t first = 0, intptr cItems = -1 ) { return Append( cItems < 0 ? arr.Num() + 1 + cItems : cItems, arr.Pointer( first ), before ); }

		void Remove( size_t first, size_t count = 1 );
		inline void RemoveLast() { if( m_cArr > 0 ) { Resize( m_cArr - 1 ); } }
		TElement PopLast();

		inline TElement *begin() { return m_pArr; }
		inline TElement *end() { return m_pArr + m_cArr; }

		inline const TElement *begin() const { return m_pArr; }
		inline const TElement *end() const { return m_pArr + m_cArr; }
		
		inline TElement *Pointer( size_t index = 0 ) { return index < 0 ? NULL : m_pArr + index; }
		inline const TElement *Pointer( size_t index = 0 ) const { return index < 0 ? NULL : m_pArr + index; }

		inline size_t Index( const TElement *ptr ) const { return ptr >= m_pArr ? ( size_t )( ptr - m_pArr )/sizeof( TElement ) : -1; }

		inline bool operator!() const { return IsEmpty(); }
		inline operator bool() const { return !IsEmpty(); }

		inline TArray &operator=( const TArray &arr ) { Assign( arr ); return *this; }
		template< size_t N >
		inline TArray &operator=( const TElement( &arr )[ N ] ) { Assign( arr ); return *this; }

		inline TArray &operator+=( const TArray &arr ) { Append( arr ); return *this; }
		template< size_t N >
		inline TArray &operator+=( const TElement( &arr )[ N ] ) { Append( arr ); return *this; }

		inline TElement &At( size_t index ) { return *Pointer( index ); }
		inline const TElement &At( size_t index ) const { return *Pointer( index ); }

		inline TElement &First() { return *Pointer( 0 ); }
		inline const TElement &First() const { return *Pointer( 0 ); }
		inline TElement &Last() { return *Pointer( m_cArr - 1 ); }
		inline const TElement &Last() const { return *Pointer( m_cArr - 1 ); }
		
		inline TElement &operator[]( size_t index ) { return *Pointer( index ); }
		inline const TElement &operator[]( size_t index ) const { return *Pointer( index ); }

	private:
		size_t m_cArr;
		size_t m_cMax;
		TElement *m_pArr;
		size_t m_cGranularity;
		const int m_iTag;
	};

	template< typename TElement >
	inline TArray< TElement >::TArray( const SMemtag &memtag )
	: m_cArr( 0 )
	, m_cMax( 0 )
	, m_pArr( NULL )
	, m_cGranularity( kDefaultGranularity )
	, m_iTag( memtag )
	{
	}
	template< typename TElement >
	inline TArray< TElement >::TArray( const TArray &arr, const SMemtag &memtag )
	: m_cArr( 0 )
	, m_cMax( 0 )
	, m_pArr( NULL )
	, m_cGranularity( arr.m_cGranularity )
	, m_iTag( memtag )
	{
		Append( arr );
	}
	template< typename TElement >
	inline TArray< TElement >::TArray( size_t cItems, const TElement *pItems, const SMemtag &memtag )
	: m_cArr( 0 )
	, m_cMax( 0 )
	, m_pArr( NULL )
	, m_cGranularity( kDefaultGranularity )
	, m_iTag( memtag )
	{
		Append( cItems, pItems );
	}
	template< typename TElement >
	inline TArray< TElement >::~TArray()
	{
		Purge();
	}

	template< typename TElement >
	inline void TArray< TElement >::Clear()
	{
		for( size_t i = m_cArr; i > 0; --i ) {
			Destroy( m_pArr[ i - 1 ] );
		}
		m_cArr = 0;
	}
	template< typename TElement >
	inline void TArray< TElement >::Purge()
	{
		SetAllocated( 0 );
	}

	template< typename TElement >
	inline bool TArray< TElement >::Resize( size_t size, const TElement &x )
	{
		if( !Reserve( size ) ) {
			return false;
		}

		for( size_t i = m_cArr; i > size; --i ) {
			Destroy( m_pArr[ i - 1 ] );
		}
		for( size_t i = m_cArr; i < size; ++i ) {
			Construct( m_pArr[ i ], x );
		}

		m_cArr = size;

		return true;
	}
	template< typename TElement >
	inline bool TArray< TElement >::Reserve( size_t size )
	{
		if( m_cMax >= size ) {
			return true;
		}

		const size_t granulatedSize = size%m_cGranularity != 0 ? size + ( m_cGranularity - size%m_cGranularity ) : size;
		return SetAllocated( granulatedSize );
	}
	template< typename TElement >
	inline bool TArray< TElement >::SetAllocated( size_t size )
	{
		while( m_cArr > size ) {
			Destroy( m_pArr[ --m_cArr ] );
		}

		TElement *pItems = NULL;
		if( size > 0 ) {
			pItems = reinterpret_cast< TElement * >( Alloc( sizeof( TElement )*size, m_iTag ) );
			if( !pItems ) {
				return false;
			}

			for( size_t i = 0; i < m_cArr; ++i ) {
				Construct( pItems[ i ], m_pArr[ i ] );
			}

		}

		Dealloc( reinterpret_cast< void * >( m_pArr ) );
		m_pArr = pItems;
		m_cMax = size;

		return true;
	}

	template< typename TElement >
	inline bool TArray< TElement >::Assign( size_t cItems, const TElement *pItems )
	{
		if( ( cItems > 0 && !pItems ) || !Reserve( cItems ) ) {
			return false;
		}

		while( m_cArr > 0 ) {
			Destroy( m_pArr[ --m_cArr ] );
		}

		while( m_cArr < cItems ) {
			Construct( m_pArr[ m_cArr ], pItems[ m_cArr ] );
			++m_cArr;
		}

		return true;
	}
	template< typename TElement >
	inline bool TArray< TElement >::Append( size_t cItems, const TElement *pItems )
	{
		if( ( cItems > 0 && !pItems ) || !Reserve( m_cArr + cItems ) ) {
			return false;
		}

		for( size_t i = 0; i < cItems; ++i ) {
			Construct( m_pArr[ m_cArr++ ], pItems[ i ] );
		}

		return true;
	}
	template< typename TElement >
	inline bool TArray< TElement >::Prepend( size_t cItems, const TElement *pItems )
	{
		if( !m_cArr ) {
			return Append( cItems, pItems );
		}

		return Insert( cItems, pItems, m_pArr );
	}
	template< typename TElement >
	inline bool TArray< TElement >::Insert( size_t cItems, const TElement *pItems, TElement *pBefore )
	{
		const size_t cBefore = Index( pBefore );
		if( cBefore > m_cArr || !pItems || !Reserve( m_cArr + cItems ) ) {
			return false;
		}

		const size_t top = m_cArr + cItems;
		for( size_t i = 0; i < m_cArr - cBefore; ++i ) {
			Construct( m_pArr[ top - i ], m_pArr[ m_cArr - i ] );
			Destroy( m_pArr[ m_cArr - i ] );
		}

		for( size_t i = 0; i < cItems; ++i ) {
			Construct( m_pArr[ cBefore + i ], pItems[ i ] );
		}

		m_cArr += cItems;

		return true;
	}
	
	/*
	
		[ 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 ]
		.                 R----------->
		
		RM: 4 .. 7 (inclusive)
		... start = 4
		... cItems = 4


		[ 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 ]
		.                             D
		.                         D
		.                     D
		.                 D
		... destruct in reverse order from 7 to 4


		[ 0 | 1 | 2 | 3 | 8 | 9 | 6 | 7 | 8 | 9 ]
		.                         X   X   |   |
		.                 M               |   |
		.                     M           |   |
		.                                 |   D
		.                                 D
		... NOTE: pItems 6 and 7 are dead
		... move pItems 8 and 9 to spots 4 and 5 respectively (in that order)
		... destruct where pItems 8 and 9 were


		[ 0 | 1 | 2 | 3 | 8 | 9 ]

		... final
	
	*/

	template< typename TElement >
	inline void TArray< TElement >::Remove( size_t first, size_t count )
	{
		if( first >= m_cArr || !count ) {
			return;
		}

		const size_t top = first + count > m_cArr ? m_cArr : first + count;
		for( size_t i = top; i > first; --i ) {
			Destroy( m_pArr[ i - 1 ] );
		}

		const size_t remainder = m_cArr - top;
		for( size_t i = 0; i < remainder; ++i ) {
			Construct( m_pArr[ first + i ], m_pArr[ top + i ] );
			Destroy( m_pArr[ top + i ] );
		}

		const size_t removed = count > m_cArr ? m_cArr : count;
		m_cArr -= removed;
	}

	template< typename TElement >
	inline TElement TArray< TElement >::PopLast()
	{
		if( !m_cArr ) {
			return TElement();
		}

		const TElement x = m_pArr[ m_cArr - 1 ];
		Resize( m_cArr - 1 );
		return x;
	}

}

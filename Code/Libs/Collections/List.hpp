#pragma once

#include "../Allocation/New.hpp"
#include "../Platform/Platform.hpp" //for AX_DELETE_FUNC

namespace Ax
{

	// Forward declarations
	template< typename TElement >
	class TIntrusiveList;

	// A link within a list -- meant to be used as a member of whatever object owns it
	template< typename TElement >
	class TIntrusiveLink
	{
	friend class TIntrusiveList< TElement >;
	public:
		typedef TElement ElementType;
		typedef TIntrusiveList< TElement > ListType;

		// Default constructor
		TIntrusiveLink();
		// Construct with a given owner
		TIntrusiveLink( TElement *pNode );
		// Construct with a given owner, added to the end of a list
		TIntrusiveLink( TElement *pNode, TIntrusiveList< TElement > &list );
		// Destructor -- unlinks from list
		~TIntrusiveLink();

		// Remove self from whatever list we're in
		void Unlink();
		// Unlink from current list and insert before the given link
		void InsertBefore( TIntrusiveLink &link );
		// Unlink from current list and insert after the given link
		void InsertAfter( TIntrusiveLink &link );

		// Move this link to the front of the list it's within
		void ToFront();
		// Move this link to the back of the list it's within
		void ToBack();
		// Move this link to before the link immediately before this one
		void ToPrior();
		// Move this link to after the link immediately after this one
		void ToNext();

		// Retrieve the previous sibling link
		inline TIntrusiveLink *PrevLink() { return m_pPrev; }
		// Retrieve the previous sibling link [const]
		inline const TIntrusiveLink *PrevLink() const { return m_pPrev; }

		// Retrieve the next sibling link
		inline TIntrusiveLink *NextLink() { return m_pNext; }
		// Retrieve the next sibling link [const]
		inline const TIntrusiveLink *NextLink() const { return m_pNext; }

		// Retrieve the owner of the sibling link prior to this
		inline TElement *Prev() { return m_pPrev != NULL ? m_pPrev->m_pNode : NULL; }
		// Retrieve the owner of the sibling link prior to this [const]
		inline const TElement *Prev() const { return m_pPrev != NULL ? m_pPrev->m_pNode : NULL; }

		// Retrieve the owner of the sibling link after this
		inline TElement *Next() { return m_pNext != NULL ? m_pNext->m_pNode : NULL; }
		// Retrieve the owner of the sibling link after this [const]
		inline const TElement *Next() const { return m_pNext != NULL ? m_pNext->m_pNode : NULL; }

		// Set the owner of this link
		inline void SetNode( TElement *pNode ) { m_pNode = pNode; }
		// Retrieve the owner of this link
		inline TElement *Node() { return m_pNode; }
		// Retrieve the owner of this link [const]
		inline const TElement *Node() const { return m_pNode; }

		// Retrieve the owner of this link
		inline TElement &operator *() { return *m_pNode; }
		// Retrieve the owner of this link [const]
		inline const TElement &operator *() const { return *m_pNode; }

		// Dereference the owner of this link
		inline TElement *operator->() { return m_pNode; }
		// Dereference the owner of this link [const]
		inline const TElement *operator->() const { return m_pNode; }

		// Retrieve the list this link is a part of
		inline TIntrusiveList< TElement > *List() { return m_pList; }
		// Retrieve the list this link is a part of [const]
		inline const TIntrusiveList< TElement > *List() const { return m_pList; }

		// Determine whether an owner is set
		inline operator bool() const { return m_pNode != NULL; }
		// Retrieve the owner of this link (through casting)
		inline operator TElement *() { return m_pNode; }
		// Retrieve the owner of this link (through casting)
		inline operator const TElement *() const { return m_pNode; }

	protected:
		// Prior sibling link
		TIntrusiveLink< TElement > *m_pPrev;
		// Next sibling link
		TIntrusiveLink< TElement > *m_pNext;
		// List this link resides within
		TIntrusiveList< TElement > *m_pList;
		// Current owner node
		TElement *m_pNode;

	private:
		TIntrusiveLink( const TIntrusiveLink & ) AX_DELETE_FUNC;
		TIntrusiveLink &operator=( const TIntrusiveLink & ) AX_DELETE_FUNC;
	};
	// A list of intrusive links -- does not do any allocations
	template< typename TElement >
	class TIntrusiveList
	{
	public:
		typedef TElement ElementType;
		typedef TIntrusiveLink< TElement > LinkType;
		typedef int( *Comparator_t )( const TElement &, const TElement & );

		static inline int OperatorLessComparator_f( const TElement &a, const TElement &b )
		{
			return +( a < b );
		}

		// Default constructor
		TIntrusiveList();
		// Destructor -- unlinks all items in the list
		~TIntrusiveList();

		// Unlinks the given link from this list (debug mode checks that the link is in this list)
		void Unlink( TIntrusiveLink< TElement > &link );
		// Unlink all items in this list
		void Clear();
		// Unlink all items in this list, deleting the owner nodes
		void DeleteAll();
		// Determine whether this list has any links in it
		bool IsEmpty() const;
		// Count how many links are in this list (this value is not cached)
		int Num() const;

		// Add a link to the front of this list after unlinking it from whatever list it was in before
		void AddHead( TIntrusiveLink< TElement > &link );
		// Add a link to the back of this list after unlinking it from whatever list it was in before
		void AddTail( TIntrusiveLink< TElement > &link );
		// Insert 'link' before 'before' (debug mode checks that 'before' is part of this list) after unlinking 'link' from whatever list it was in before
		void InsertBefore( TIntrusiveLink< TElement > &link, TIntrusiveLink< TElement > &before );
		// Insert 'link' after 'after' (debug mode checks that 'after' is part of this list) after unlinking 'link' from whatever list it was in before
		void InsertAfter( TIntrusiveLink< TElement > &link, TIntrusiveLink< TElement > &after );

		// Sort the items in this list with the given comparison function
		void Sort( Comparator_t pfnCompare );
		// Sort the items in this list with ElementType::operator<() comparison
		inline void Sort() { Sort( &OperatorLessComparator_f ); }

		// Retrieve the link at the front of this list
		inline TIntrusiveLink< TElement > *HeadLink() { return m_pHead; }
		// Retrieve the link at the front of this list [const]
		inline const TIntrusiveLink< TElement > *HeadLink() const { return m_pHead; }

		// Retrieve the link at the back of this list
		inline TIntrusiveLink< TElement > *TailLink() { return m_pTail; }
		// Retrieve the link at the back of this list [const]
		inline const TIntrusiveLink< TElement > *TailLink() const { return m_pTail; }

		// Retrieve the owner of the link at the front of this list
		inline TElement *Head() { return m_pHead != NULL ? m_pHead->m_pNode : NULL; }
		// Retrieve the owner of the link at the front of this list [const]
		inline const TElement *Head() const { return m_pHead != NULL ? m_pHead->m_pNode : NULL; }

		// Retrieve the owner of the link at the back of this list
		inline TElement *Tail() { return m_pTail != NULL ? m_pTail->m_pNode : NULL; }
		// Retrieve the owner of the link at the back of this list [const]
		inline const TElement *Tail() const { return m_pTail != NULL ? m_pTail->m_pNode : NULL; }

		// Determine whether this list has any items in it
		inline operator bool() const { return !IsEmpty(); }
		// Determine whether this list has no items in it
		inline bool operator!() const { return IsEmpty(); }

	private:
		// Link to the front of the list
		TIntrusiveLink< TElement > *m_pHead;
		// Link to the back of the list
		TIntrusiveLink< TElement > *m_pTail;

		TIntrusiveList( const TIntrusiveList & ) AX_DELETE_FUNC;
		TIntrusiveList &operator=( const TIntrusiveList & ) AX_DELETE_FUNC;
	};

	namespace Detail
	{
	
		template< typename TLink >
		struct TListIterator
		{
			typedef TLink LinkType;
			typedef typename TLink::ElementType ElementType;

			TLink *pLink;

			inline TListIterator(): pLink( NULL ) {}
			inline TListIterator( TLink &x ): pLink( &x ) {}
			inline TListIterator( TLink *x ): pLink( x ) {}
			inline TListIterator( const TListIterator &iter ): pLink( iter.pLink ) {}
			inline ~TListIterator() {}

			inline ElementType *Get() { return pLink != NULL ? pLink->Node() : NULL; }
			inline const ElementType *Get() const { return pLink != NULL ? pLink->Node() : NULL; }

			inline TListIterator &operator=( const TListIterator &iter ) { pLink = iter.pLink; return *this; }

			inline bool operator!() const { return !pLink || !pLink->Node(); }
			inline bool operator==( const TListIterator &iter ) const { return pLink == iter.pLink; }
			inline bool operator!=( const TListIterator &iter ) const { return pLink != iter.pLink; }
			inline operator ElementType *() { return pLink->Node(); }
			inline operator const ElementType *() const { return pLink->Node(); }
			inline operator bool() const { return Get() != nullptr; }
			
			inline ElementType &operator*() { return *pLink->Node(); }
			inline ElementType *operator->() { return pLink->Node(); }
			inline const ElementType &operator*() const { return *pLink->Node(); }
			inline const ElementType *operator->() const { return pLink->Node(); }

			inline TListIterator &Retreat() { pLink = pLink != NULL ? pLink->PrevLink() : NULL; return *this; }
			inline TListIterator &Advance() { pLink = pLink != NULL ? pLink->NextLink() : NULL; return *this; }

			inline TListIterator &operator--() { return Retreat(); }
			inline TListIterator &operator++() { return Advance(); }

			inline TListIterator operator--( int ) const { return TListIterator( const_cast< TLink * >( pLink ) ).Retreat(); }
			inline TListIterator operator++( int ) const { return TListIterator( const_cast< TLink * >( pLink ) ).Advance(); }
		};
		
	}

	template< typename TElement >
	class TList
	{
	public:
		typedef typename TIntrusiveList< TElement >::Comparator_t Comparator_t;
		typedef Detail::TListIterator< TIntrusiveLink< TElement > > Iterator;
		//typedef TListIterator< const TIntrusiveLink< TElement > > ConstIterator;

		TList( const SMemtag &memtag = SMemtag() );
		TList( const TList &ls, const SMemtag &memtag = SMemtag() );
		template< int TSize >
		inline TList( const TElement( &arr )[ TSize ], const SMemtag &memtag = SMemtag() )
		: m_List()
		, m_iTag( memtag )
		{
			for( int i = 0; i < TSize; ++i ) {
				AddTail( arr[ i ] );
			}
		}
		TList( int count, TElement *items, const SMemtag &memtag = SMemtag() );
		~TList();

		inline TList &operator=( const TList &ls )
		{
			if( this == &ls ) {
				return *this;
			}

			Clear();
			for( const auto &x : ls ) {
				AddTail( x );
			}
			return *this;
		}
		template< int TSize >
		inline TList &operator=( const TElement( &arr )[ TSize ] )
		{
			Clear();
			for( int i = 0; i < TSize; ++i ) {
				AddTail( arr[ i ] );
			}
			return *this;
		}

		void Clear();
		bool IsEmpty() const;
		int Num() const;

		// For C++ range-based for-loops
		inline Iterator begin() { return Iterator( m_List.HeadLink() ); }
		inline Iterator end() { return Iterator(); }
		inline Iterator begin() const { return Iterator( const_cast< TIntrusiveLink< TElement > * >( m_List.HeadLink() ) ); }
		inline Iterator end() const { return Iterator(); }
		//inline ConstIterator begin() const { return Iterator( m_List.HeadLink() ); }
		//inline ConstIterator end() const { return Iterator(); }

		inline Iterator First() { return Iterator( m_List.HeadLink() ); }
		inline Iterator Last() { return Iterator( m_List.TailLink() ); }

		inline Iterator First() const { return Iterator( const_cast< TIntrusiveLink< TElement > * >( m_List.HeadLink() ) ); }
		inline Iterator Last() const { return Iterator( const_cast< TIntrusiveLink< TElement > * >( m_List.TailLink() ) ); }

		Iterator AddHead();
		Iterator AddTail();
		Iterator InsertBefore( Iterator x );
		Iterator InsertAfter( Iterator x );

		Iterator AddHead( const TElement &element );
		Iterator AddTail( const TElement &element );
		Iterator InsertBefore( Iterator x, const TElement &element );
		Iterator InsertAfter( Iterator x, const TElement &element );
		inline Iterator Find( const TElement &item ) { return ( ( const TList * )this )->Find( item ); }
		const Iterator Find( const TElement &item ) const;

		Iterator Remove( Iterator iter );

		void Sort( Comparator_t pfnCompare );
		void Sort();

	private:
		TIntrusiveList< TElement > m_List;
		const int m_iTag;

		TIntrusiveLink< TElement > *Alloc();
		TIntrusiveLink< TElement > *Alloc( const TElement &x );
		void Dealloc( TIntrusiveLink< TElement > *ptr );
	};


	/*
	===========================================================================

		INTRUSIVE LINK

	===========================================================================
	*/

	template< typename TElement >
	TIntrusiveLink< TElement >::TIntrusiveLink()
	: m_pPrev( NULL )
	, m_pNext( NULL )
	, m_pList( NULL )
	, m_pNode( NULL )
	{
	}
	template< typename TElement >
	TIntrusiveLink< TElement >::TIntrusiveLink( TElement *pNode )
	: m_pPrev( NULL )
	, m_pNext( NULL )
	, m_pList( NULL )
	, m_pNode( pNode )
	{
	}
	template< typename TElement >
	TIntrusiveLink< TElement >::TIntrusiveLink( TElement *pNode, TIntrusiveList< TElement > &list )
	: m_pPrev( NULL )
	, m_pNext( NULL )
	, m_pList( NULL )
	, m_pNode( pNode )
	{
		list.AddTail( *this );
	}
	template< typename TElement >
	TIntrusiveLink< TElement >::~TIntrusiveLink()
	{
		Unlink();
	}
	
	template< typename TElement >
	void TIntrusiveLink< TElement >::Unlink()
	{
		if( !m_pList ) {
			return;
		}

		m_pList->Unlink( *this );
	}
	template< typename TElement >
	void TIntrusiveLink< TElement >::InsertBefore( TIntrusiveLink &link )
	{
		if( !m_pList ) {
			return;
		}

		m_pList->InsertBefore( link, *this );
	}
	template< typename TElement >
	void TIntrusiveLink< TElement >::InsertAfter( TIntrusiveLink &link )
	{
		if( !m_pList ) {
			return;
		}

		m_pList->InsertAfter( link, *this );
	}

	template< typename TElement >
	void TIntrusiveLink< TElement >::ToFront()
	{
		if( !m_pList ) {
			return;
		}

		m_pList->AddHead( *this );
	}
	template< typename TElement >
	void TIntrusiveLink< TElement >::ToBack()
	{
		if( !m_pList ) {
			return;
		}

		m_pList->AddTail( *this );
	}
	template< typename TElement >
	void TIntrusiveLink< TElement >::ToPrior()
	{
		if( !m_pList || !m_pPrev ) {
			return;
		}

		m_pList->InsertBefore( *this, *m_pPrev );
	}
	template< typename TElement >
	void TIntrusiveLink< TElement >::ToNext()
	{
		if( !m_pList || !m_pNext ) {
			return;
		}

		m_pList->InsertAfter( *this, *m_pNext );
	}


	/*
	===========================================================================

		INTRUSIVE LIST

	===========================================================================
	*/

	template< typename TElement >
	TIntrusiveList< TElement >::TIntrusiveList()
	: m_pHead( NULL )
	, m_pTail( NULL )
	{
	}
	template< typename TElement >
	TIntrusiveList< TElement >::~TIntrusiveList()
	{
		Clear();
	}

	template< typename TElement >
	void TIntrusiveList< TElement >::Unlink( TIntrusiveLink< TElement > &link )
	{
		if( link.m_pList != this ) {
			return;
		}

		if( link.m_pPrev != NULL ) {
			link.m_pPrev->m_pNext = link.m_pNext;
		} else {
			m_pHead = link.m_pNext;
		}

		if( link.m_pNext != NULL ) {
			link.m_pNext->m_pPrev = link.m_pPrev;
		} else {
			m_pTail = link.m_pPrev;
		}

		link.m_pList = NULL;
		link.m_pPrev = NULL;
		link.m_pNext = NULL;
	}
	template< typename TElement >
	void TIntrusiveList< TElement >::Clear()
	{
		while( m_pHead != NULL ) {
			Unlink( *m_pHead );
		}
	}
	template< typename TElement >
	void TIntrusiveList< TElement >::DeleteAll()
	{
		while( m_pHead != NULL ) {
			TIntrusiveLink< TElement > *link = m_pHead;

			Unlink( *link );
			delete link->m_pNode;
		}
	}
	template< typename TElement >
	bool TIntrusiveList< TElement >::IsEmpty() const
	{
		return m_pHead == NULL;
	}
	template< typename TElement >
	int TIntrusiveList< TElement >::Num() const
	{
		int n = 0;

		for( TIntrusiveLink< TElement > *p = m_pHead; p != NULL; p = p->m_pNext ) {
			++n;
		}

		return n;
	}

	template< typename TElement >
	void TIntrusiveList< TElement >::AddHead( TIntrusiveLink< TElement > &link )
	{
		if( &link == m_pHead ) {
			return;
		}

		if( m_pHead != NULL ) {
			InsertBefore( link, *m_pHead );
			return;
		}

		link.Unlink();
		m_pHead = &link;
		m_pTail = &link;
		link.m_pList = this;
	}
	template< typename TElement >
	void TIntrusiveList< TElement >::AddTail( TIntrusiveLink< TElement > &link )
	{
		if( &link == m_pTail ) {
			return;
		}

		if( m_pTail != NULL ) {
			InsertAfter( link, *m_pTail );
			return;
		}

		link.Unlink();
		m_pHead = &link;
		m_pTail = &link;
		link.m_pList = this;
	}
	template< typename TElement >
	void TIntrusiveList< TElement >::InsertBefore( TIntrusiveLink< TElement > &link, TIntrusiveLink< TElement > &before )
	{
		link.Unlink();

		link.m_pPrev = before.m_pPrev;
		if( before.m_pPrev != NULL ) {
			before.m_pPrev->m_pNext = &link;
		} else {
			m_pHead = &link;
		}
		before.m_pPrev = &link;
		link.m_pNext = &before;

		link.m_pList = this;
	}
	template< typename TElement >
	void TIntrusiveList< TElement >::InsertAfter( TIntrusiveLink< TElement > &link, TIntrusiveLink< TElement > &after )
	{
		link.Unlink();

		link.m_pNext = after.m_pNext;
		if( after.m_pNext != NULL ) {
			after.m_pNext->m_pPrev = &link;
		} else {
			m_pTail = &link;
		}
		after.m_pNext = &link;
		link.m_pPrev = &after;

		link.m_pList = this;
	}

	template< typename TElement >
	void TIntrusiveList< TElement >::Sort( Comparator_t pfnCompare )
	{
		TIntrusiveLink< TElement > *pNode;
		TIntrusiveLink< TElement > *pTemp;
		size_t cSwaps;

		if( !pfnCompare ) {
			return;
		}

		//
		//	TERRIBLE IMPLEMENTATION
		//	TODO: Use a better sorting algorithm
		//
		do {
			cSwaps = 0;

			for( pNode = m_pHead; pNode != NULL; pNode = pNode->m_pNext ) {
				if( !pNode->m_pNode || !pNode->m_pNext || !pNode->m_pNext->m_pNode ) {
					continue;
				}

				if( pfnCompare( *pNode->m_pNode, *pNode->m_pNext->m_pNode ) <= 0 ) {
					continue;
				}

				pTemp = pNode->m_pNext;
				pTemp->Unlink();
				pTemp->m_pList = this;

				pTemp->m_pPrev = pNode != NULL ? pNode->m_pPrev : m_pTail;
				pTemp->m_pNext = pNode;

				if( pTemp->m_pPrev != NULL ) {
					pTemp->m_pPrev->m_pNext = pTemp;
				} else {
					m_pHead = pTemp;
				}

				if( pNode != NULL ) {
					pNode->m_pPrev = pTemp;
				} else {
					m_pTail = pTemp;
				}
			
				++cSwaps;
			}
		} while( cSwaps > 0 );
	}


	/*
	===========================================================================

		LIST

	===========================================================================
	*/

	template< typename TElement >
	TList< TElement >::TList( const SMemtag &memtag )
	: m_List()
	, m_iTag( memtag )
	{
	}
	template< typename TElement >
	TList< TElement >::TList( const TList &ls, const SMemtag &memtag )
	: m_List()
	, m_iTag( memtag )
	{
		for( Iterator x = ls.begin(); x != ls.end(); ++x ) {
			AddTail( *x );
		}
	}
	template< typename TElement >
	TList< TElement >::TList( int count, TElement *pItems, const SMemtag &memtag )
	: m_List()
	, m_iTag( memtag )
	{
		if( !pItems ) {
			return;
		}

		for( int i = 0; i < count; ++i ) {
			AddTail( pItems[ i ] );
		}
	}
	template< typename TElement >
	TList< TElement >::~TList()
	{
		Clear();
	}

	template< typename TElement >
	void TList< TElement >::Clear()
	{
		while( !IsEmpty() ) {
			Remove( begin() );
		}
	}
	template< typename TElement >
	bool TList< TElement >::IsEmpty() const
	{
		return m_List.IsEmpty();
	}
	template< typename TElement >
	int TList< TElement >::Num() const
	{
		return m_List.Num();
	}

	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::AddHead()
	{
		TIntrusiveLink< TElement > *pItem = Alloc();
		if( !pItem ) {
			return end();
		}

		m_List.AddHead( *pItem );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::AddTail()
	{
		TIntrusiveLink< TElement > *pItem = Alloc();
		if( !pItem ) {
			return end();
		}

		m_List.AddTail( *pItem );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::InsertBefore( Iterator x )
	{
		if( !x ) {
			return AddTail();
		}

		TIntrusiveLink< TElement > *pItem = Alloc();
		if( !pItem ) {
			return end();
		}

		m_List.InsertBefore( *pItem, *x.link );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::InsertAfter( Iterator x )
	{
		if( !x ) {
			return AddTail();
		}

		TIntrusiveLink< TElement > *pItem = Alloc();
		if( !pItem ) {
			return end();
		}

		m_List.InsertAfter( *pItem, *x.link );
		return Iterator( pItem );
	}

	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::AddHead( const TElement &element )
	{
		TIntrusiveLink< TElement > *pItem = Alloc( element );
		if( !pItem ) {
			return end();
		}

		m_List.AddHead( *pItem );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::AddTail( const TElement &element )
	{
		TIntrusiveLink< TElement > *pItem = Alloc( element );
		if( !pItem ) {
			return end();
		}

		m_List.AddTail( *pItem );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::InsertBefore( Iterator x, const TElement &element )
	{
		if( !x ) {
			return AddTail( element );
		}

		TIntrusiveLink< TElement > *pItem = Alloc( element );
		if( !pItem ) {
			return end();
		}

		m_List.InsertBefore( *pItem, *x.link );
		return Iterator( pItem );
	}
	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::InsertAfter( Iterator x, const TElement &element )
	{
		if( !x ) {
			return AddTail( element );
		}

		TIntrusiveLink< TElement > *pItem = Alloc( element );
		if( !pItem ) {
			return end();
		}

		m_List.InsertAfter( *pItem, *x.link );
		return Iterator( pItem );
	}
	template< typename TElement >
	const typename TList< TElement >::Iterator TList< TElement >::Find( const TElement &item ) const
	{
		const TIntrusiveLink< TElement > *p;

		for( p = m_List.HeadLink(); p != NULL; p = p->NextLink() ) {
			if( *p == item ) {
				return Iterator( p );
			}
		}

		return end();
	}

	template< typename TElement >
	typename TList< TElement >::Iterator TList< TElement >::Remove( Iterator iter )
	{
		if( !iter ) {
			return end();
		}

		Iterator next = Iterator( iter.pLink->NextLink() );
		m_List.Unlink( *iter.pLink );
		Dealloc( iter.pLink );
		return next;
	}

	template< typename TElement >
	void TList< TElement >::Sort( Comparator_t pfnCompare )
	{
		m_List.Sort( pfnCompare );
	}
	template< typename TElement >
	void TList< TElement >::Sort()
	{
		m_List.Sort();
	}

	template< typename TElement >
	TIntrusiveLink< TElement > *TList< TElement >::Alloc()
	{
		static const size_t n = sizeof( TIntrusiveLink< TElement > ) + sizeof( TElement );
		char *const p = reinterpret_cast< char * >( Ax::Alloc( n, m_iTag ) );
		if( !p ) {
			return NULL;
		}

		TIntrusiveLink< TElement > *const a = ( TIntrusiveLink< TElement > * )( p );
		TElement *const b = ( TElement * )( p + sizeof( TIntrusiveLink< TElement > ) );

		new( ( void * )a, Ax::Detail::SPlcmntNw() ) TIntrusiveLink< TElement >( b );
		new( ( void * )b, Ax::Detail::SPlcmntNw() ) TElement();

		return a;
	}
	template< typename TElement >
	TIntrusiveLink< TElement > *TList< TElement >::Alloc( const TElement &element )
	{
		static const size_t n = sizeof( TIntrusiveLink< TElement > ) + sizeof( TElement );
		char *const p = reinterpret_cast< char * >( Ax::Alloc( n, m_iTag ) );
		if( !p ) {
			return NULL;
		}

		TIntrusiveLink< TElement > *const a = ( TIntrusiveLink< TElement > * )( p );
		TElement *const b = ( TElement * )( p + sizeof( TIntrusiveLink< TElement > ) );

		new( ( void * )a, Ax::Detail::SPlcmntNw() ) TIntrusiveLink< TElement >( b );
		new( ( void * )b, Ax::Detail::SPlcmntNw() ) TElement( element );

		return a;
	}
	template< typename TElement >
	void TList< TElement >::Dealloc( TIntrusiveLink< TElement > *ptr )
	{
		if( !ptr ) {
			return;
		}

		if( ptr->Node() != NULL ) {
			ptr->Node()->~TElement();
		}

		Ax::Dealloc( reinterpret_cast< void * >( ptr ) );
	}

}

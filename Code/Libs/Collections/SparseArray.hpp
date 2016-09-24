#pragma once

#include "../Allocation/New.hpp"
#include "../Allocation/Allocator.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Types.hpp"

#include "Array.hpp"
#include "List.hpp"
#include "Pair.hpp"

//
//	***** WORK IN PROGRESS *****
//

namespace Ax
{

	template< typename TElement >
	class TSparseArray
	{
	public:
		typedef TElement								ElementType;
		typedef TSparseArray<TElement>					ThisType;
		typedef TPair< uintptr, TElement * >			PairType;
		typedef uintptr									MaskType;

		static const uintptr		kDesiredChunkSizeInBytes = 4096;
		static const uintptr		kElementSizeInBytes = sizeof( TElement );
		static const uintptr		kBaseElementsPerChunk = kDesiredChunkSizeInBytes/kElementSizeInBytes;
		static const uintptr		kMinElementsPerChunk = 4;
		static const uintptr		kElementsPerChunk = kBaseElementsPerChunk > kMinElementsPerChunk ? kBaseElementsPerChunk : kMinElementsPerChunk;
		static const uintptr		kMaskBits = sizeof( MaskType )*8;
		static const uintptr		kNumMasks = kElementsPerChunk/kMaskBits + ( +( kElementsPerChunk%kMaskBits != 0 ) );

		inline TSparseArray( const SMemtag &memtag = SMemtag() )
		: m_FreeChunks()
		, m_Chunks( memtag )
		, m_cElements( 0 )
		{
		}
		inline ~TSparseArray()
		{
			Purge();
		}

		inline int GetMemtag() const
		{
			return m_Chunks.GetMemtag();
		}

		inline PairType Create()
		{
			PairType R = CreateUnconstructed();
			if( R.Second != nullptr ) {
				Construct( *R.Second );
			}

			return R;
		}
		inline PairType Create( const TElement &X )
		{
			PairType R = CreateUnconstructed();
			if( R.Second != nullptr ) {
				Construct( *R.Second, X );
			}

			return R;
		}

		inline void Clear()
		{
			for( uintptr i = m_Chunks.Num(); i > 0; --i ) {
				DeallocChunk( m_Chunks[ i - 1 ] );
			}
			m_Chunks.Clear();
		}
		inline void Purge()
		{
			Clear();
			m_Chunks.Purge();
		}

		inline bool IsEmpty() const
		{
			return m_cElements == 0;
		}

		inline uintptr Len() const
		{
			return m_cElements;
		}
		inline uintptr Num() const
		{
			return m_cElements;
		}
		inline uintptr Mem() const
		{
			return m_Chunks.Num()*sizeof( SChunk ) + sizeof( *this );
		}

	private:
		struct SElement
		{
			uint8					Bytes[ sizeof( TElement ) ];

			inline TElement *Ptr()
			{
				return reinterpret_cast< TElement * >( &Bytes[ 0 ] );
			}
			inline const TElement *Ptr() const
			{
				return reinterpret_cast< const TElement * >( &Bytes[ 0 ] );
			}

			inline TElement *operator->()
			{
				return reinterpret_cast< TElement * >( &Bytes[ 0 ] );
			}
			inline const TElement *operator->() const
			{
				return reinterpret_cast< const TElement * >( &Bytes[ 0 ] );
			}

			inline TElement &operator*()
			{
				return reinterpret_cast< TElement & >( Bytes[ 0 ] );
			}
			inline const TElement &operator*() const
			{
				return reinterpret_cast< const TElement & >( Bytes[ 0 ] );
			}

			inline operator TElement &()
			{
				return reinterpret_cast< TElement & >( Bytes[ 0 ] );
			}
			inline operator const TElement &() const
			{
				return reinterpret_cast< const TElement & >( Bytes[ 0 ] );
			}
		};
		struct SChunk
		{
			typedef Ax::TIntrusiveList< SChunk >		IList;
			typedef Ax::TIntrusiveLink< SChunk >		ILink;

			ILink					FreeSibling;

			uintptr					uBaseElement;

			MaskType				UsedMask			[ kNumMasks ];
			MaskType				ReservedMask		[ kNumMasks ];
			uintptr					cUsedElements;
			uintptr					cReservedElements;

			SElement				Elements			[ kElementsPerChunk ];

			static inline void SetBit( MaskType( &Mask )[ kNumMasks ], uintptr uIndex )
			{
				Mask[ uIndex/kMaskBits ] |= uintptr( 1 )<<( uIndex%kMaskBits );
			}
			static inline void ClearBit( MaskType( &Mask )[ kNumMasks ], uintptr uIndex )
			{
				Mask[ uIndex/kMaskBits ] &= ~( uintptr( 1 )<<( uIndex%kMaskBits ) );
			}
			static inline bool TestBit( const MaskType( &Mask )[ kNumMasks ], uintptr uIndex )
			{
				return !!( Mask[ uIndex/kMaskBits ] & ( uintptr( 1 )<<( uIndex%kMaskBits ) ) );
			}

			inline void InitMasks()
			{
				for( MaskType &x : UsedMask ) {
					x = 0;
				}
				for( MaskType &x : ReservedMask ) {
					x = 0;
				}

				cUsedElements = 0;
				cReservedElements = 0;
			}
			inline void DestructUsed()
			{
				for( uintptr uMaskIndex = 0; uMaskIndex < kNumMasks; ++uMaskIndex ) {
					if( !cUsedElements ) {
						break;
					}

					const uintptr uMask = UsedMask[ uMaskIndex ];
					if( !uMask ) {
						continue;
					}

					for( uintptr uBitIndex = 0; uBitIndex < kMaskBits; ++uBitIndex ) {
						if( ~uMask & ( uintptr( 1 )<<uBitIndex ) ) {
							continue;
						}

						const uintptr uElementIndex = uMaskIndex*kMaskBits + uBitIndex;
						Destroy( *Elements[ uElementIndex ] );

						UsedMask[ uMaskIndex ] &= ~( uintptr( 1 )<<uBitIndex );
						if( --cUsedElements == 0 ) {
							break;
						}
					}
				}
			}

			inline bool HasFree() const
			{
				return cReservedElements != kElementsPerChunk;
			}

			inline void MarkRangeReserved( uintptr uBegin, uintptr uEnd )
			{
				AX_ASSERT( uBegin < kElementsPerChunk );
				AX_ASSERT( uEnd <= kElementsPerChunk );
				AX_ASSERT( uEnd > uBegin );

				for( uintptr i = uBegin; i < uEnd; ++i ) {
					cReservedElements += +!TestBit( ReservedMask, i );
					SetBit( ReservedMask, i );
				}
			}
			inline void UnmarkRangeReserved( uintptr uBegin, uintptr uEnd )
			{
				AX_ASSERT( uBegin < kElementsPerChunk );
				AX_ASSERT( uEnd <= kElementsPerChunk );
				AX_ASSERT( uEnd > uBegin );

				for( uintptr i = uBegin; i < uEnd; ++i ) {
					cReservedElements -= +TestBit( ReservedMask, i );
					ClearBit( ReservedMask, i );
				}
			}
			inline void MarkRangeUsed( uintptr uBegin, uintptr uEnd )
			{
				AX_ASSERT( uBegin < kElementsPerChunk );
				AX_ASSERT( uEnd <= kElementsPerChunk );
				AX_ASSERT( uEnd > uBegin );

				for( uintptr i = uBegin; i < uEnd; ++i ) {
					cUsedElements += +!TestBit( UsedMask, i );
					SetBit( UsedMask, i );
				}
			}
			inline void UnmarkRangeUsed( uintptr uBegin, uintptr uEnd )
			{
				AX_ASSERT( uBegin < kElementsPerChunk );
				AX_ASSERT( uEnd <= kElementsPerChunk );
				AX_ASSERT( uEnd > uBegin );

				for( uintptr i = uBegin; i < uEnd; ++i ) {
					cUsedElements -= +TestBit( UsedMask, i );
					ClearBit( UsedMask, i );
				}
			}

			inline intptr FindFreeIndex() const
			{
				if( cReservedElements == kElementsPerChunk ) {
					return -1;
				}

				const uintptr uStartMask = cReservedElements/kMaskBits;
				for( uintptr uLoopIndex = 0; uLoopIndex < kNumMasks; ++uLoopIndex ) {
					const uintptr uMaskIndex = ( uStartMask + uLoopIndex )%kNumMasks;
					const uintptr uMask = ReservedMask[ uMaskIndex ];
					if( uMask == ~uintptr( 0 ) ) {
						continue;
					}

					const uintptr uStartBit = cReservedElements%kMaskBits;
					for( uintptr uSubloopIndex = 0; uSubloopIndex < kMaskBits; ++uSubloopIndex ) {
						const uintptr uBitIndex = ( uStartBit + uSubloopIndex )%kMaskBits;
						if( ~uMask & ( uintptr( 1 )<<uBitIndex ) ) {
							continue;
						}

						return uMaskIndex*kMaskBits + uBitIndex;
					}
				}

				AX_ASSERT_MSG( false, "Unreachable" );
				return -1;
			}

			inline SElement *FindFreeElement() const
			{
				const intptr iIndex = FindFreeIndex();
				if( iIndex < 0 ) {
					return nullptr;
				}

				return &Elements[ iIndex ];
			}
		};

		typename SChunk::IList		m_FreeChunks;
		Ax::TArray< SChunk * >		m_Chunks;
		uintptr						m_cElements;

		// Find the chunk containing the given element index
		inline SChunk *FindChunk( uintptr uIndex )
		{
			const uintptr uChunkIndex = uIndex/kElementsPerChunk;
			if( uChunkIndex >= m_Chunks.Num() ) {
				return nullptr;
			}

			return m_Chunks[ uChunkIndex ];
		}

		// Allocate a chunk
		inline SChunk *AllocChunk( uintptr uBaseElement )
		{
			AX_ASSERT_MSG( uBaseElement%kElementsPerChunk == 0, "Base element of chunk must be aligned" );
			AX_ASSERT_IS_NULL( FindChunk( uBaseElement ) );

			const uintptr uChunkIndex = uBaseElement/kElementsPerChunk;
			if( m_Chunks.Num() <= uChunkIndex ) {
				if( !m_Chunks.Resize( uChunkIndex + 1 ) ) {
					return nullptr;
				}
			}

			SChunk *const pChunk = Alloc< SChunk >( 1, m_Chunks.GetMemtag() );
			if( !pChunk ) {
				return nullptr;
			}

			pChunk->InitMasks();
			pChunk->uBaseElement = uBaseElement;

			m_Chunks[ uChunkIndex ] = pChunk;

			pChunk->FreeSibling.Link( pChunk );
			m_FreeChunks.AddTail( pChunk->FreeSibling );

			return pChunk;
		}
		// Allocate a chunk, without a specific base element
		inline SChunk *AllocChunk()
		{
			uintptr i;
			for( i = 0; i < m_Chunks.Num(); ++i ) {
				if( m_Chunks[ i ] != nullptr ) {
					continue;
				}

				break;
			}
			
			return AllocChunk( i*kElementsPerChunk );
		}
		// Deallocate a chunk (and all items in it)
		inline void DeallocChunk( SChunk *pChunk )
		{
			if( !pChunk ) {
				return;
			}

			const uintptr uChunkIndex = pChunk->uBaseElement/kElementsPerChunk;
			AX_ASSERT( uChunkIndex < m_Chunks.Num() );
			AX_ASSERT_NOT_NULL( m_Chunks[ uChunkIndex ] );
			AX_ASSERT( m_Chunks[ uChunkIndex ] == pChunk );

			pChunk->FreeSibling.Unlink();

			pChunk->DestructUsed();

			m_Chunks[ uChunkIndex ] = nullptr;
			Dealloc( pChunk );
		}

		// Find a free index from the available chunks (and retrieve the corresponding chunk and chunk index)
		inline intptr FindFreeIndex( SChunk *&pOutChunk, uintptr &uOutChunkIndex ) const
		{
			SChunk *const pChunk = m_FreeChunks.Head();

			if( !pChunk ) {
				return -1;
			}

			AX_ASSERT( pChunk->HasFree() );

			const intptr iChunkIndex = pChunk->FindFreeIndex();
			AX_ASSERT( iChunkIndex >= 0 );

			pOutChunk = pChunk;
			uOutChunkIndex = uintptr( iChunkIndex );

			return intptr( pChunk->uBaseElement ) + iChunkIndex;
		}

		// Find or allocate a chunk with the given index
		inline SChunk *LookupChunk( uintptr uIndex )
		{
			SChunk *pChunk = FindChunk( uIndex );
			if( pChunk != nullptr ) {
				return pChunk;
			}

			pChunk = AllocChunk( uIndex/kElementsPerChunk );
			return pChunk;
		}

		// Mark an index as used
		//
		// This will also mark the index as reserved to prevent a "find free
		// index" routine from returning the same index.
		//
		// If marking the index as used causes there to be no more free elements
		// in the chunk then the chunk is also removed from the free list.
		inline bool MarkIndexUsed( uintptr uIndex )
		{
			const uintptr uElementIndex = uIndex%kElementsPerChunk;
			SChunk *const pChunk = LookupChunk( uIndex );
			if( !pChunk ) {
				return false;
			}

			AX_ASSERT_MSG( SChunk::TestBit( pChunk->UsedMask, uElementIndex ) == false, "Element already used" );

			pChunk->MarkRangeReserved( uElementIndex, uElementIndex + 1 );
			pChunk->MarkRangeUsed( uElementIndex, uElementIndex + 1 );

			if( !pChunk->HasFree() ) {
				pChunk->FreeSibling.Unlink();
			}

			return true;
		}
		// Mark an index as unused
		//
		// This will also mark an index as unreserved.
		//
		// If the chunk previously had no free elements then marking it as
		// unused will also add it to the beginning of the free list. The chunk
		// is added to the beginning instead of the end in an effort to improve
		// cache usage. (The chunk will have been the most recently accessed and
		// therefore the most likely to still be in cache on the next allocation
		// routine.)
		inline void MarkIndexUnused( uintptr uIndex )
		{
			const uintptr uElementIndex = uIndex%kElementsPerChunk;
			SChunk *const pChunk = FindChunk( uIndex );
			AX_ASSERT_NOT_NULL( pChunk );

			AX_ASSERT_MSG( SChunk::TestBit( pChunk->UsedMask, uElementIndex ) == true, "Element not used" );

			const bool bWasFree = pChunk->HasFree();

			pChunk->UnmarkRangeReserved( uElementIndex, uElementIndex + 1 );
			pChunk->UnmarkRangeUsed( uElementIndex, uElementIndex + 1 );

			if( !bWasFree ) {
				m_FreeChunks.AddHead( pChunk->FreeSibling );
			}
		}

		// Find a free spot in the array and grab the item
		inline PairType CreateUnconstructed()
		{
			SChunk *pFreeChunk = nullptr;
			uintptr uChunkIndex = ~uintptr( 0 );

			SElement *pElement = nullptr;

			intptr iFreeIndex = FindFreeIndex( pFreeChunk, uChunkIndex );
			if( iFreeIndex < 0 ) {
				pFreeChunk = AllocChunk();
				if( !pFreeChunk ) {
					return PairType( 0, nullptr );
				}

				iFreeIndex = pFreeChunk->FindFreeIndex();
				AX_ASSERT_MSG( iFreeIndex >= 0, "New chunk must have free indexes" );

				uChunkIndex = iFreeIndex;
				iFreeIndex += pFreeChunk->uBaseIndex;
			}

			AX_ASSERT( iFreeIndex > 0 );
			MarkIndexUsed( uintptr( iFreeIndex ) );

			AX_ASSERT( uChunkIndex < kElementsPerChunk );
			pElement = &pFreeChunk->Elements[ uChunkIndex ];

			return MakePair( uintptr( iFreeIndex ), pElement->Ptr() );
		}
	};

}

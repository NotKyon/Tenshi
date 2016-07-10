#include "Allocator.hpp"
#include "../Core/Assert.hpp"
#if AX_MEMTAGS_ENABLED
# include "../Async/Atomic.hpp"
# include "../Core/Logger.hpp"
# include "../System/Timer.hpp"
# include "../Core/String.hpp"
#endif

#define AX_CRT_DEBUG_ENABLED 0

#ifndef AX_CRT_DEBUG_ENABLED
# if AX_DEBUG_ENABLED && defined( _MSC_VER )
#  define AX_CRT_DEBUG_ENABLED 1
# else
#  define AX_CRT_DEBUG_ENABLED 0
# endif
#endif

#if AX_CRT_DEBUG_ENABLED
# include <crtdbg.h>
#endif

#ifndef AX_ALLOCATOR_MAX_NAME
# define AX_ALLOCATOR_MAX_NAME 64
#endif

// Memory tag profiling can be detrimental to multicore performance due to
// reliance on QueryPerformanceCounter(). Only enable if you really need to know
// the timing information per alloc/dealloc.

#ifndef AX_MEMTAG_PROFILING_ENABLED
# define AX_MEMTAG_PROFILING_ENABLED 0
#endif
#if AX_MEMTAG_PROFILING_ENABLED && !AX_MEMTAGS_ENABLED
# undef AX_MEMTAG_PROFILING_ENABLED
# define AX_MEMTAG_PROFILING_ENABLED 0
#endif


/*
===============================================================================

	UTILITY

===============================================================================
*/
namespace
{

struct SMemtagDetails
{
	char							szName				[ AX_ALLOCATOR_MAX_NAME ];
	Ax::IAllocator *				pAllocator;
	Ax::STagStats					Stats;

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4351)
#endif
									// Constructor
	inline							SMemtagDetails		()
									: szName()
									, pAllocator( nullptr )
									, Stats()
									{
										szName[ 0 ] = '\0';
									}
#ifdef _MSC_VER
# pragma warning(pop)
#endif
									// Destructor
	inline							~SMemtagDetails		()
									{
#if AX_DEBUG_ENABLED
										// Check for outstanding allocations
										if( Stats.cAllocs != Stats.cDeallocs ) {
											Ax::Warnf
											(
												szName,
												"There are %i outstanding allocations.",
												( int )( Stats.cAllocs - Stats.cDeallocs )
											);
										}
#endif
									}

	inline void						SetName				( const char *newName )
									{
										if( !newName || *newName == '\0' ) {
											szName[ 0 ] = '\0';
											return;
										}

										size_t i;
										for( i = 0; newName[ i ] != '\0'; ++i ) {
											if( i + 2 == AX_ALLOCATOR_MAX_NAME ) {
												break;
											}
											szName[ i ] = newName[ i ];
										}
										szName[ i ] = '\0';
									}
};

struct SMemtagTable
{
	SMemtagDetails					Details				[ AX_MAX_MEMTAGS ];

									// Retrieve the instance of this data
	static SMemtagTable &			GetInstance			()
									{
										static SMemtagTable instance;
										return instance;
									}

private:
									// Constructor
	inline							SMemtagTable		()
									: Details()
									{
#if AX_CRT_DEBUG_ENABLED
										_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | _CRTDBG_DELAY_FREE_MEM_DF );
#endif
									}
									// Destructor
	inline							~SMemtagTable		()
									{
#if AX_CRT_DEBUG_ENABLED
										_CrtCheckMemory();
										_CrtDumpMemoryLeaks();
#endif
									}
};

struct STagStub
{
	size_t							size;
	int								tag;
# if AX_MEMTAG_DEBUG_ENABLED
	static const Ax::uint32			kMagicNumber		= 0xC0DEFACE;
	static const Ax::uint32			kBadNumber			= 0x1BADCAFE;
	const char *					pszFile;
	const char *					pszFunction;
	Ax::uint32						Line;
	Ax::uint32						Counter;
	Ax::uint32						MagicNumber;
# endif
};

}

inline SMemtagDetails &GetDetails( int tag )
{
	AX_ASSERT( tag >= 0 && tag < AX_MAX_MEMTAGS );

	const int i = tag >= 0 && tag < AX_MAX_MEMTAGS ? tag : AX_MEMTAG_MISC;
	return SMemtagTable::GetInstance().Details[ i ];
}


/*
===============================================================================

	IMPLEMENTATION

===============================================================================
*/

void Ax::SetCustomAllocator( IAllocator *allocator, int tag )
{
	SMemtagDetails &tagDetails = GetDetails( tag );
	STagStats &tagStats = tagDetails.Stats;
	const unsigned int numOutstandingAllocs = tagStats.cAllocs - tagStats.cDeallocs;

	AX_ASSERT_MSG( numOutstandingAllocs == 0, "Cannot adjust allocator while there are outstanding allocations." );
	( void )numOutstandingAllocs;

	tagDetails.pAllocator = allocator;
}
Ax::IAllocator *Ax::GetCustomAllocator( int tag )
{
	return GetDetails( tag ).pAllocator;
}

void Ax::SetAllocatorName( const char *name, int tag )
{
	SMemtagDetails &tagDetails = GetDetails( tag );

	tagDetails.SetName( name );
}
AX_RETURN_NOT_NULL
const char *Ax::GetAllocatorName( int tag )
{
	return GetDetails( tag ).szName;
}

namespace Ax {

	static const char *UnsignedIntegerToString( char *pszOutBuffer, uintptr cOutBufferMax, uint64 uValue, uint32 uRadix = 10 )
	{
		static const char *const pszDigits = "0123456789ABCDEF";
		static const uintptr kPadding = sizeof( void * )*2;

		AX_ASSERT_NOT_NULL( pszOutBuffer );
		AX_ASSERT( cOutBufferMax > 1 );
		AX_ASSERT( uRadix >= 2 && uRadix <= 16 );
		AX_ASSERT( uRadix != 16 || cOutBufferMax > kPadding + 2 );

		uintptr cBuffer = cOutBufferMax;
		uint64 n = uValue;
		char *p = &pszOutBuffer[ cOutBufferMax - 1 ];
		char *const s = p - kPadding;

		*p = '\0';
		while( uValue > 0 && cBuffer > 0 ) {
			*--p = pszDigits[ n%uRadix ];
			n /= uRadix;
			--cBuffer;
		}
		if( *p == '\0' ) {
			*--p = '0';
		}

		if( uRadix == 16 ) {
			while( p > s ) {
				*--p = '0';
			}

			*--p = 'x';
			*--p = '0';
		}

		return p;
	}
	template< uintptr tOutBufferMax >
	inline const char *UnsignedIntegerToString( char( &pszOutBuffer )[ tOutBufferMax ], uint64 uValue, uint32 uRadix = 10 )
	{
		return UnsignedIntegerToString( pszOutBuffer, tOutBufferMax, uValue, uRadix );
	}
	static void VerifyAlloc( void *Pointer, const SMemtagDetails &Details, const STagStub &Stub, const STagStats &Stats )
	{
		char szBuffer[ 256 ];
		char szNameBuffer[ 32 ];
		char szTempBuffer[ 32 ];
		const char *pszName;
		const uintptr Size = Stub.size;
		const uintptr RealSize = Size + sizeof( STagStub );

		if( Details.szName[ 0 ] != '\0' ) {
			pszName = Details.szName;
		} else {
			pszName = UnsignedIntegerToString( szNameBuffer, Stub.tag );
		}
		
		szBuffer[ 0 ] = '\0';

		const char *pszExpression = "Dealloc(Pointer)";

		if( Stats.cAllocs <= Stats.cDeallocs ) {
			StrCat( szBuffer, "There are less allocations than deallocations.\nAllocs=" );
			StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, Stats.cAllocs ) );
			StrCat( szBuffer, "\nDeallocs=" );
			StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, Stats.cDeallocs ) );

			pszExpression = "cAllocs > cDeallocs";
		} else if( Stats.CurMemUsage < RealSize ) {
			StrCat( szBuffer, "Current memory usage is registered to be less than the amount to be deallocated.\nCurUsage=" );
			StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, Stats.CurMemUsage ) );
			StrCat( szBuffer, "\nDeallocSize=" );
			StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, RealSize ) );

			pszExpression = "CurMemUsage >= RealSize";
		}

		const char *pszFile = __FILE__;
		uint32 Line = __LINE__;
		const char *pszFunction = AX_PRETTY_FUNCTION;

#if AX_MEMTAG_DEBUG_ENABLED
		if( Stub.MagicNumber == STagStub::kMagicNumber || Stub.MagicNumber == STagStub::kBadNumber ) {
			if( Stub.MagicNumber == STagStub::kBadNumber && szBuffer[ 0 ] != '\0' ) {
				StrCat( szBuffer, "Bad pointer for deallocation\nPointer=" );
				StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, uint64( Pointer ), 16 ) );
			}

			if( szBuffer[ 0 ] != '\0' && Stub.pszFile != NULL ) {
				pszFile = Stub.pszFile;
				Line = Stub.Line;
				pszFunction = Stub.pszFunction;

				StrCat( szBuffer, "\n\nAlloc. File : " );
				StrCat( szBuffer, Stub.pszFile );
				StrCat( szBuffer, "\nAlloc. Line : " );
				StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, Stub.Line ) );
				StrCat( szBuffer, "\nAlloc. Counter : " );
				StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, Stub.Counter ) );
				if( Stub.pszFunction != NULL ) {
					StrCat( szBuffer, "\nAlloc. Function : " );
					StrCat( szBuffer, Stub.pszFunction );
				}
			}
		} else if( szBuffer[ 0 ] != '\0' ) {
			StrCat( szBuffer, "Bad pointer for deallocation\nPointer=" );
			StrCat( szBuffer, UnsignedIntegerToString( szTempBuffer, uint64( Pointer ), 16 ) );
		}
#endif

		if( szBuffer[ 0 ] == '\0' ) {
			return;
		}

		Detail::HandleAssert( pszFile, Line, pszFunction, pszExpression, szBuffer );
	}

}

AX_RETURN_NOT_NULL
void *Ax::Alloc( size_t n, int tag )
{
	if( !n ) {
		AX_ASSERT_MSG( false, "Expected 'n' > 0" );

		static unsigned char notNull = 0;
		return &notNull;
	}

	SMemtagDetails &tagDetails = GetDetails( tag );
	STagStats &tagStats = tagDetails.Stats;
	IAllocator *const allocator = tagDetails.pAllocator;
	const size_t realSize = n + sizeof( STagStub );

#if AX_MEMTAG_PROFILING_ENABLED
	System::CTimer perfTimer;
#endif

	void *const allocedMemory = allocator != nullptr ? allocator->Alloc( realSize ) : malloc( realSize );
	AX_VERIFY_NOT_NULL( allocedMemory );

#if AX_MEMTAGS_ENABLED
# if AX_MEMTAG_PROFILING_ENABLED
	const uint64 totalTime = perfTimer.GetElapsed();
# endif

	Async::AtomicInc( &tagStats.cAllocs );
	Async::AtomicAdd( &tagStats.CurMemUsage, realSize );
	if( tagStats.CurMemUsage > tagStats.MaxMemUsage ) {
		Async::AtomicSet( &tagStats.MaxMemUsage, tagStats.CurMemUsage );
	}

# if AX_MEMTAG_PROFILING_ENABLED
	AtomicAdd( &tagStats.TotalAllocTime, totalTime );
	if( tagStats.MinAllocTime > totalTime ) {
		AtomicSet( &tagStats.MinAllocTime, totalTime );
	}
	if( tagStats.MaxAllocTime < totalTime ) {
		AtomicSet( &tagStats.MaxAllocTime, totalTime );
	}
# endif
#endif

	STagStub *const stub = ( STagStub * )allocedMemory;

	stub->size = n;
	stub->tag = tag;
#if AX_MEMTAG_DEBUG_ENABLED
	stub->pszFile = NULL;
	stub->pszFunction = NULL;
	stub->Line = 0;
	stub->Counter = 0;
	stub->MagicNumber = STagStub::kMagicNumber;
#endif

	return ( void * )( stub + 1 );
}
AX_RETURN_NULL
void *Ax::Dealloc( void *p )
{
	if( !p ) {
		return nullptr;
	}

	STagStub *const stub = ( ( STagStub * )p ) - 1;
	const size_t size = stub->size;
	const size_t realSize = size + sizeof( STagStub );

	SMemtagDetails &tagDetails = GetDetails( stub->tag );
	STagStats &tagStats = tagDetails.Stats;
	IAllocator *const allocator = tagDetails.pAllocator;

	VerifyAlloc( p, tagDetails, *stub, tagStats );

#if AX_MEMTAG_PROFILING_ENABLED
	System::CTimer perfTimer;
#endif

	if( allocator != nullptr ) {
		allocator->Dealloc( ( void * )stub );
	} else {
		free( ( void * )stub );
	}

#if AX_MEMTAGS_ENABLED
# if AX_MEMTAG_PROFILING_ENABLED
	const uint64 totalTime = perfTimer.GetElapsed();
# endif

	Async::AtomicInc( &tagStats.cDeallocs );
	Async::AtomicSub( &tagStats.CurMemUsage, realSize );

# if AX_MEMTAG_PROFILING_ENABLED
	AtomicAdd( &tagStats.TotalDeallocTime, totalTime );
	if( tagStats.MinDeallocTime > totalTime ) {
		AtomicSet( &tagStats.MinDeallocTime, totalTime );
	}
	if( tagStats.MaxDeallocTime < totalTime ) {
		AtomicSet( &tagStats.MaxDeallocTime, totalTime );
	}
# endif
#endif

	return nullptr;
}

Ax::STagStats &Ax::GetAllocStats( int tag )
{
	AX_ASSERT( tag >= 0 && tag < AX_MAX_MEMTAGS );

	return GetDetails( tag ).Stats;
}
#if AX_MEMTAG_DEBUG_ENABLED
void Ax::IntegrateAllocationDebugData( void *p, const char *pszFile, uint32 Line, uint32 Counter, const char *pszFunction )
{
	if( !p ) {
		return;
	}

	STagStub &Stub = *( ( ( STagStub * )p ) - 1 );

	AX_ASSERT_IS_NULL( Stub.pszFile );
	AX_ASSERT( Stub.MagicNumber == STagStub::kMagicNumber );

	Stub.pszFile = pszFile;
	Stub.pszFunction = pszFunction;
	Stub.Line = Line;
	Stub.Counter = Counter;
}
#endif

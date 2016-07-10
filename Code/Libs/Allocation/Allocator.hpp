#pragma once

#include "../Platform/BuildConf.hpp"
#include "../Core/Types.hpp"

#ifndef AX_MEMTAGS_ENABLED
# define AX_MEMTAGS_ENABLED 1
#endif
#ifndef AX_MEMTAG_DEBUG_ENABLED
# if AX_MEMTAGS_ENABLED && AX_DEBUG_ENABLED
#  define AX_MEMTAG_DEBUG_ENABLED 1
# else
#  define AX_MEMTAG_DEBUG_ENABLED 0
# endif
#endif
#if !AX_MEMTAGS_ENABLED
# undef AX_MEMTAG_DEBUG_ENABLED
# define AX_MEMTAG_DEBUG_ENABLED 0
#endif

#ifndef AX_DEFAULT_MEMTAG
# define AX_DEFAULT_MEMTAG 0
#endif
#ifndef AX_MEMTAG_MISC
# define AX_MEMTAG_MISC AX_DEFAULT_MEMTAG
#endif

#ifndef AX_MAX_MEMTAGS
# define AX_MAX_MEMTAGS 256
#endif

namespace Ax
{

	/*!
	 * Stores information about the various memory allocations and deallocations
	 * that have occurred throughout the life of a given subsystem (tag).
	 * 
	 * This keeps track of the number of allocations and deallocations that have
	 * succeeded, as well as the current and peak memory usage, allocation times
	 * and deallocation times.
	 * 
	 * To get a handle to this structure call Ax::GetAllocStats().
	 * 
	 * This is useful for keeping track of Ax's various internal systems. You
	 * can also track down memory leaks and do simple profiling.
	 */
	struct STagStats
	{
		/// Total number of allocations that have been made with the allocator
		uintcpu						cAllocs;
		/// Total number of deallocations that have been made with the allocator
		uintcpu						cDeallocs;

		/// Current memory usage (in bytes) of the allocator
		uintptr						CurMemUsage;
		/// Most memory ever used by this allocator
		uintptr						MaxMemUsage;

		/// Total amount of time spent in allocations (microseconds)
		uint64						TotalAllocTime;
		/// Most time spent on any allocation (microseconds)
		uint64						MaxAllocTime;
		/// Least time spent on any allocation (microseconds)
		uint64						MinAllocTime;

		/// Total amount of time spent in deallocations (microseconds)
		uint64						TotalDeallocTime;
		/// Most time spent on any deallocation (microseconds)
		uint64						MaxDeallocTime;
		/// Least time spent on any deallocation (microseconds)
		uint64						MinDeallocTime;

		/// Constructor
		inline STagStats()
		{
			Reset();
		}
		/// Destructor
		inline ~STagStats()
		{
		}

		/// Reset the statistics for this tag back to their defaults
		inline void Reset()
		{
			cAllocs = 0;
			cDeallocs = 0;

			CurMemUsage = 0;
			MaxMemUsage = 0;

			TotalAllocTime = 0;
			MaxAllocTime = 0;
			MinAllocTime = ( uint64 )-1;

			TotalDeallocTime = 0;
			MaxDeallocTime = 0;
			MinDeallocTime = ( uint64 )-1;
		}
	};
	
	/*!
	 * Helper type to signify a memory tag
	 */
	struct SMemtag
	{
		const int					value;

		inline SMemtag( int x = AX_DEFAULT_MEMTAG )
		: value( x )
		{
		}
		inline SMemtag( const SMemtag &x )
		: value( x.value )
		{
		}

		inline operator int() const
		{
			return value;
		}
		inline int operator()() const
		{
			return value;
		}
	};

	/*!
	 * Interface for a custom allocation system. Allows you to specify how
	 * memory is allocated and deallocated by Ax. For example, you could
	 * reserve all the memory a particular system needs up-front to guarantee a
	 * request within the system won't fail.
	 * 
	 * Use Ax::SetCustomAllocator() to set an allocator.
	 * Use Ax::GetCustomAllocator() to retrieve an allocator.
	 * 
	 * NOTE: If you set an allocator you need to make sure there are no
	 *       outstanding memory allocations for anything using the previous
	 *       allocator.
	 */
	class IAllocator
	{
	public:
		/// Constructor. (Nothing special.)
		IAllocator()
		{
		}
		/// Destructor. (Nothing special.)
		virtual ~IAllocator()
		{
		}

		/// Perform an allocation. (n > 0)
		virtual void *Alloc( size_t n ) = 0;
		/// Perform a deallocation. (p != nullptr)
		virtual void Dealloc( void *p ) = 0;
	};


	/*
	===========================================================================
	
		FUNCTIONS
	
	===========================================================================
	*/

	/// Set a custom allocator (Ax::IAllocator) for the given subsystem.
	void SetCustomAllocator( IAllocator *pAllocator, int tag = AX_DEFAULT_MEMTAG );
	/// Retrieve a custom allocator (Ax::IAllocator) for the given subsystem.
	IAllocator *GetCustomAllocator( int tag = AX_DEFAULT_MEMTAG );
	
	/// Set the name of the current allocator
	void SetAllocatorName( const char *pszName, int tag = AX_DEFAULT_MEMTAG );
	/// Retrieve the name of the current allocator
	AX_RETURN_NOT_NULL
	const char *GetAllocatorName( int tag = AX_DEFAULT_MEMTAG );

	/*!
	 * Allocate memory using Ax's internal systems. Call Ax::MainDealloc()
	 * when you're done with the returned handle.
	 * 
	 * This will not gracefully exit if there is not enough memory to satisfy
	 * the request.
	 * 
	 * Returns nullptr if n == 0.
	 */
	AX_RETURN_NOT_NULL
	void *Alloc( size_t n, int tag = AX_DEFAULT_MEMTAG );
	/// Deallocate memory that was previously allocated with Ax::MainAlloc().
	AX_RETURN_NULL
	void *Dealloc( void *p );
	/// Adjust an allocation with additional debug data (should only be done once, immediately after allocation)
#if AX_MEMTAG_DEBUG_ENABLED
	void IntegrateAllocationDebugData( void *p, const char *pszFile, uint32 Line, uint32 Counter, const char *pszFunction );
#else
	AX_FORCEINLINE void IntegrateAllocationDebugData( void *, const char *, uint32, uint32, const char * )
	{
	}
#endif

	template< typename tElement >
	AX_RETURN_NOT_NULL
	AX_FORCEINLINE tElement *Alloc( size_t n = 1, int tag = AX_DEFAULT_MEMTAG )
	{
		return ( tElement * )Alloc( n*sizeof( tElement ), tag );
	}
	template< typename tElement >
	AX_RETURN_NULL
	AX_FORCEINLINE tElement *Dealloc( tElement *p )
	{
		return ( tElement * )Dealloc( ( void * )p );
	}

	/*!
	 * Retrieve the allocation statistics used for the given tag.
	 *
	 * The returned handle can be modified, but it CANNOT be deallocated.
	 */
	STagStats &GetAllocStats( int tag = AX_DEFAULT_MEMTAG );

}

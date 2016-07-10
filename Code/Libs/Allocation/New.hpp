#pragma once

#include "Allocator.hpp"
#include <stddef.h>

#ifndef AX_OVERRIDE_NEW_DELETE_ENABLED
# define AX_OVERRIDE_NEW_DELETE_ENABLED 0
#endif

namespace Ax { namespace Detail { struct SPlcmntNw {}; } }

inline void *operator new( size_t, void *p, const Ax::Detail::SPlcmntNw & )
{
	return p;
}
inline void operator delete( void *, void *, const Ax::Detail::SPlcmntNw & )
{
}

namespace Ax
{

	template< typename T >
	inline void Construct( T &x )
	{
		new( ( void * )&x, Detail::SPlcmntNw() ) T();
	}
	template< typename T >
	inline void Construct( T &x, const T &y )
	{
		new( ( void * )&x, Detail::SPlcmntNw() ) T( y );
	}

	template< typename T >
	inline void Destroy( T &x )
	{
		x.~T();
	}
	
	template< typename T >
	inline T *New( int tag = AX_DEFAULT_MEMTAG )
	{
		return ( T * )Alloc( sizeof( T ), tag );
	}
	template< typename T >
	inline void Delete( T *&p )
	{
		Dealloc( ( void * )p );
		p = nullptr;
	}

}

#if AX_OVERRIDE_NEW_DELETE_ENABLED
inline void *operator new( size_t n )
{
	return Ax::Alloc( n );
}
inline void operator delete( void *p )
{
	Ax::Dealloc( p );
}

inline void *operator new[]( size_t n )
{
	return Ax::Alloc( n );
}
inline void operator delete[]( void *p )
{
	Ax::Dealloc( p );
}
#endif

inline void *operator new( size_t n, const Ax::SMemtag &memtag )
{
	return Ax::Alloc( n, memtag );
}
inline void operator delete( void *p, const Ax::SMemtag & )
{
	Ax::Dealloc( p );
}

inline void *operator new[]( size_t n, const Ax::SMemtag &memtag )
{
	return Ax::Alloc( n, memtag );
}
inline void operator delete[]( void *p, const Ax::SMemtag & )
{
	Ax::Dealloc( p );
}

inline void *operator new( size_t n, const Ax::SMemtag &memtag, const char *pszFile, Ax::uint32 Line, Ax::uint32 Counter, const char *pszFunction )
{
	void *const p = Ax::Alloc( n, memtag );
	if( p != NULL ) {
		Ax::IntegrateAllocationDebugData( p, pszFile, Line, Counter, pszFunction );
	}

	return p;
}
inline void *operator new[]( size_t n, const Ax::SMemtag &memtag, const char *pszFile, Ax::uint32 Line, Ax::uint32 Counter, const char *pszFunction )
{
	void *const p = Ax::Alloc( n, memtag );
	if( p != NULL ) {
		Ax::IntegrateAllocationDebugData( p, pszFile, Line, Counter, pszFunction );
	}

	return p;
}

namespace Ax { namespace Detail { static Ax::uint32 g_uNewCounter = 0; } }

#define AX_NEW						new( Ax::SMemtag(), __FILE__, __LINE__, Ax::Detail::g_uNewCounter++, AX_PRETTY_FUNCTION )
#define AX_TAGGED_NEW(Tag)			new( Ax::SMemtag(Tag), __FILE__, __LINE__, Ax::Detail::g_uNewCounter++, AX_PRETTY_FUNCTION )

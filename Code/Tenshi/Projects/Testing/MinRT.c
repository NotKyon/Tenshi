#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
# define EXTERN_C					extern "C"
#else
# define EXTERN_C
#endif

#define IMPORT						EXTERN_C
#define EXPORT						EXTERN_C

#ifdef _MSC_VER
# define CURRENT_FUNCTION			__FUNCTION__
#else
# define CURRENT_FUNCTION			__func__
#endif

#ifdef _DEBUG
# define TRACE(...)\
	DbgTrace_(__FILE__,__LINE__,CURRENT_FUNCTION,__VA_ARGS__)
#else
# define TRACE(...)					((void)0)
#endif

#if defined( _MSC_VER ) && defined( _WIN32 )
# define LOCKFP(FilePtr_)			_lock_file((FilePtr_))
# define UNLOCKFP(FilePtr_)			_unlock_file((FilePtr_))
#else
# define LOCKFP(FilePtr_)			((void)0)
# define UNLOCKFP(FilePtr_)			((void)0)
#endif

IMPORT void TenshiMain( void );

EXPORT void teAutoprint( const char *pszText )
{
	printf( "%s\n", pszText != NULL ? pszText : "" );
}
EXPORT int teSafeSync( void )
{
	return 1;
}

EXPORT void DbgTrace_( const char *file, unsigned int line, const char *func,
const char *format, ... )
{
	va_list args;

	LOCKFP( stderr );

	fprintf( stderr, "%s(%u): in %s: ", file, line, func );

	va_start( args, format );
	vfprintf( stderr, format, args );
	va_end( args );

	fprintf( stderr, "\n" );
	fflush( stderr );

	UNLOCKFP( stderr );
}

static char *StrAlloc( char *p, size_t n )
{
	char *q;

	TRACE( "p=%p; n=%u", ( void * )p, ( unsigned int )n );

	q = n > 0 ? malloc( n + 1 ) : NULL;

	if( p != NULL ) {
		if( q != NULL ) {
			memcpy( q, p, strlen( p ) + 1 );
		}

		free( p );
		p = NULL;
	} else if( q != NULL ) {
		*q = '\0';
		q[ n ] = '\0';
	}

	if( n > 0 && !q ) {
		fprintf( stderr, "ERROR: Out of memory\n" );
		exit( EXIT_FAILURE );
	}

	return q;
}
static char *StrDup( const char *s )
{
	size_t n;
	char *p;

	TRACE( "s=%p", ( const void * )s );

	if( !s ) {
		return NULL;
	}

	n = strlen( s );
	p = StrAlloc( NULL, n );

	memcpy( p, s, n + 1 );

	return p;
}

EXPORT char *teStrConcat( const char *a, const char *b )
{
	size_t alen, blen;
	size_t len;
	char *p;

	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );

	if( !a || !b ) {
		if( a != NULL ) {
			return StrDup( a );
		}
		
		if( b != NULL ) {
			return StrDup( b );
		}

		return NULL;
	}

	alen = strlen( a );
	blen = strlen( b );
	len = alen + blen;

	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );

	p = StrAlloc( NULL, len );

	memcpy( p, a, alen );
	memcpy( p + alen, b, blen + 1 );

	return p;
}
EXPORT char *teStrFindRm( const char *a, const char *b )
{
#define MAX_OCCURRENCES 512
	const char *occurrences[ MAX_OCCURRENCES ];
	size_t c_occurrences;
	size_t i;

	const char *base;
	const char *s;
	const char *t;

	size_t alen, blen;
	size_t len;
	
	char *p;
	char *q;

	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );

	if( !a || !b ) {
		return ( char * )a;
	}

	blen = strlen( b );
	if( !blen ) {
		return ( char * )a;
	}

	alen = strlen( a );
	p = NULL;
	q = NULL;

	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );

	len = alen;
	s = a;
	base = a;
	do {
		c_occurrences = 0;
		for(;;) {
			t = strstr( s, b );
			if( !t ) {
				occurrences[ c_occurrences++ ] = strchr( s, '\0' );
				s = NULL;
				break;
			}

			TRACE
			(
				"#%.2u@%p (len{%u}-=(t{%p}-s{%p}){%u}){%u}",
				( unsigned int )c_occurrences, ( const void * )t,
				( unsigned int )len,
				( const void * )t, ( const void * )s,
				( unsigned int )( size_t )( t - s ),
				( unsigned int )( len - ( size_t )( t - s ) )
			);

			occurrences[ c_occurrences++ ] = t;
			len -= blen;
			s = t + blen;

			if( c_occurrences == MAX_OCCURRENCES ) {
				break;
			}
		}

		q -= ( size_t )p;
		p = StrAlloc( p, len );
		q += ( size_t )p;
		TRACE( "cp(p=%p,len=%u)x%u",
			( const void * )p, ( unsigned int )len,
			( unsigned int )c_occurrences );

		for( i = 0; i < c_occurrences; ++i ) {
			t = occurrences[ i ];

			TRACE
			(
				"~%.2u dst=q{%p} src=base{%p}, cnt=(t{%p} - base{%p}){%u}",
				i,
				( const void * )q, ( const void * )base,
				( const void * )t, ( const void * )base,
				( unsigned int )( size_t )( t - base )
			);

			memcpy( q, base, t - base );
			q += ( size_t )( t - base );
			base += ( size_t )( t - base ) + blen;
		}
	} while( s != NULL );

	if( q != NULL ) {
		*q = '\0';
	}
	TRACE( "ret=%p, ret.num=%u", ( const void * )p, ( unsigned int )( size_t )( q - p ) );

	return p;
}
EXPORT char *teStrRepeat( const char *s, unsigned int n )
{
	unsigned int i;
	size_t slen;
	char *p;

	TRACE( "s=%p, n=%u", ( const void * )s, n );

	slen = s != NULL && n > 0 ? strlen( s ) : 0;
	if( !slen ) {
		return NULL;
	}

	p = StrAlloc( NULL, slen*( size_t )n );

	for( i = 0; i < n; ++i ) {
		memcpy( p + slen*i, s, slen );
	}

	return p;
}
EXPORT char *teStrCatDir( const char *a, const char *b )
{
	size_t alen, blen;
	size_t len;
	char *p;
	char chinsert;

	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );

	if( !a || !b ) {
		if( a != NULL ) {
			return StrDup( a );
		}

		if( b != NULL ) {
			return StrDup( b );
		}

		return NULL;
	}

	alen = strlen( a );
	blen = strlen( b );

	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );

	len = alen + blen;
	chinsert = '\0';

	if( alen > 0 && a[ alen - 1 ] != '/' ) {
#ifdef _WIN32
		if( a[ alen - 1 ] != '\\' ) {
			chinsert = '/';
			++len;
		}
#else
		chinsert = '/';
		++len;
#endif
	}

	p = StrAlloc( NULL, len );

	if( alen > 0 ) {
		memcpy( p, a, alen );
	}

	if( chinsert != '\0' ) {
		p[ alen++ ] = chinsert;
	}

	memcpy( p + alen, b, blen );

	return p;
}
EXPORT char *teStrReclaim( char *s )
{
	return StrAlloc( s, 0 );
}

EXPORT char *teCastInt32ToStr( int i )
{
	unsigned int n;
	size_t len;
	char sign;
	char *p, *r;
	char buf[ 64 ];

	p = &buf[ sizeof( buf ) - 1 ];

	*p = '\0';

	sign = i < 0 ? '-' : i == 0 ? '0' : '\0';
	n = i < 0 ? -i : i;

	while( n > 0 ) {
		*--p = '0' + n%10;
		n /= 10;
	}

	if( sign != '\0' ) {
		*--p = sign;
	}

	len = ( size_t )( &buf[ sizeof( buf ) ] - p );
	r = StrAlloc( NULL, len );
	memcpy( r, p, len );

	return r;
}

int main( int argc, char **argv )
{
	( void )argc;
	( void )argv;

	TenshiMain();
	return EXIT_SUCCESS;
}


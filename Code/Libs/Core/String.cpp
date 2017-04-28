#include "String.hpp"
#include "Assert.hpp"

#define AXSTR_IMPLEMENT 1
#define AXSTR_STDSTR_ENABLED 1

/*
 *	Perform one step of a UTF-8 to UTF-32 decoding.
 *
 *	ppUTF8Src: Points to the pointer of what will be read from. That pointer is
 *	-           incremented by this function upon completion.
 *	pUTF8SrcEnd: One element past the end of the buffer referenced by
 *	-             *ppUTF8Src. No data will be read from this point or beyond.
 *
 *	return: UTF-32 encoding of the character.
 */
AXSTR_FUNC axstr_utf32_t AXSTR_CALL axstr_step_utf8_decode( const axstr_utf8_t **ppUTF8Src, const axstr_utf8_t *pUTF8SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf32_t uCodepoint;
	const axstr_utf8_t *p;

	p = *ppUTF8Src;

	if( p >= pUTF8SrcEnd ) {
		return 0;
	}

	if( ( p[0] & 0xF0 ) == 0xF0 ) {
		if( p + 4 > pUTF8SrcEnd ) {
			*ppUTF8Src = pUTF8SrcEnd;
			return 0xFFFD;
		}

		uCodepoint  = ( ( axstr_utf32_t )( p[0] & 0x07 ) ) << 18;
		uCodepoint |= ( ( axstr_utf32_t )( p[1] & 0x3F ) ) << 12;
		uCodepoint |= ( ( axstr_utf32_t )( p[2] & 0x3F ) ) <<  6;
		uCodepoint |= ( ( axstr_utf32_t )( p[3] & 0x3F ) ) <<  0;

		*ppUTF8Src += 4;
		return uCodepoint;
	}

	if( ( p[0] & 0xE0 ) == 0xE0 ) {
		if( p + 3 > pUTF8SrcEnd ) {
			*ppUTF8Src = pUTF8SrcEnd;
			return 0xFFFD;
		}

		uCodepoint  = ( ( axstr_utf32_t )( p[0] & 0x0F ) ) << 12;
		uCodepoint |= ( ( axstr_utf32_t )( p[1] & 0x3F ) ) <<  6;
		uCodepoint |= ( ( axstr_utf32_t )( p[2] & 0x3F ) ) <<  0;

		*ppUTF8Src += 3;
		return uCodepoint;
	}

	if( ( p[0] & 0xC0 ) == 0xC0 ) {
		if( p + 2 > pUTF8SrcEnd ) {
			*ppUTF8Src = pUTF8SrcEnd;
			return 0xFFFD;
		}

		uCodepoint  = ( ( axstr_utf32_t )( p[0] & 0x1F ) ) <<  6;
		uCodepoint |= ( ( axstr_utf32_t )( p[1] & 0x3F ) ) <<  0;

		*ppUTF8Src += 2;
		return uCodepoint;
	}

	uCodepoint = ( axstr_utf32_t )( p[0] & 0x7F );
	*ppUTF8Src += 1;

	return uCodepoint;
}
#else
;
#endif
/*
 *	Perform one step of a UTF-16 to UTF-32 decoding.
 *
 *	ppUTF16Src: Points to the pointer of what will be read from. That pointer is
 *	-           incremented by this function upon completion.
 *	pUTF16SrcEnd: One element past the end of the buffer referenced by
 *	-             *ppUTF16Src. No data will be read from this point or beyond.
 *
 *	return: UTF-32 encoding of the character.
 */
AXSTR_FUNC axstr_utf32_t AXSTR_CALL axstr_step_utf16_decode( const axstr_utf16_t **ppUTF16Src, const axstr_utf16_t *pUTF16SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf32_t uCodepoint;

	if( *ppUTF16Src >= pUTF16SrcEnd ) {
		return 0;
	}

	/* if encoded as one codepoint then return that codepoint */
	if( **ppUTF16Src < 0xD800 || **ppUTF16Src > 0xDFFF ) {
		return ( axstr_utf32_t )*( *ppUTF16Src )++;
	}

	/* two codepoints */
	if( *ppUTF16Src + 1 >= pUTF16SrcEnd ) {
		*ppUTF16Src = pUTF16SrcEnd;
		return 0xFFFD;
	}

	/* check for invalid first codepoint */
	if( **ppUTF16Src > 0xDBFF ) {
		( *ppUTF16Src ) += 2;
		return 0xFFFD;
	}

	/* check for invalid second codepoint */
	if( ( *ppUTF16Src )[1] < 0xDC00 || ( *ppUTF16Src )[1] > 0xDFFF ) {
		( *ppUTF16Src ) += 2;
		return 0xFFFD;
	}

	/* encode */
	uCodepoint = 0x10000 + ( ( ( ( axstr_utf32_t )( ( *ppUTF16Src )[0] & 0x3FF ) )<<10 ) | ( ( ( *ppUTF16Src )[1] ) & 0x3FF ) );
	( *ppUTF16Src ) += 2;

	return uCodepoint;
}
#else
;
#endif

/*
 *	Perform one step of a UTF-32 to UTF-8 encoding.
 *
 *	ppUTF8Dst: Points to the pointer of what will be written to. That pointer
 *	-           is incremented by this function upon completion.
 *	pUTF8DstEnd: One element past the end of the buffer referenced by
 *	-             *ppUTF8Dst. No data will be written to or beyond this point.
 *	uCodepoint: Unicode character to encode.
 *
 *	return: 1 on success; 0 on failure.
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_step_utf8_encode( axstr_utf8_t **ppUTF8Dst, axstr_utf8_t *pUTF8DstEnd, axstr_utf32_t uCodepoint )
#if AXSTR_IMPLEMENT
{
	if( uCodepoint > 0x10000 ) {
		if( *ppUTF8Dst + 4 >= pUTF8DstEnd ) {
			return 0;
		}

		( *ppUTF8Dst )[ 0 ] = ( axstr_utf8_t )( 0xF0 | ( ( uCodepoint>>18 ) & 0x07 ) );
		( *ppUTF8Dst )[ 1 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>>12 ) & 0x3F ) );
		( *ppUTF8Dst )[ 2 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>> 6 ) & 0x3F ) );
		( *ppUTF8Dst )[ 3 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>> 0 ) & 0x3F ) );
		
		*ppUTF8Dst += 4;
	} else if( uCodepoint > 0x7FF ) {
		if( *ppUTF8Dst + 3 >= pUTF8DstEnd ) {
			return 0;
		}
		
		( *ppUTF8Dst )[ 0 ] = ( axstr_utf8_t )( 0xE0 | ( ( uCodepoint>>12 ) & 0x0F ) );
		( *ppUTF8Dst )[ 1 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>> 6 ) & 0x3F ) );
		( *ppUTF8Dst )[ 2 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>> 0 ) & 0x3F ) );

		*ppUTF8Dst += 3;
	} else if( uCodepoint > 0x7F ) {
		if( *ppUTF8Dst + 2 >= pUTF8DstEnd ) {
			return 0;
		}

		( *ppUTF8Dst )[ 0 ] = ( axstr_utf8_t )( 0xC0 | ( ( uCodepoint>> 6 ) & 0x1F ) );
		( *ppUTF8Dst )[ 1 ] = ( axstr_utf8_t )( 0x80 | ( ( uCodepoint>> 0 ) & 0x3F ) );
		
		*ppUTF8Dst += 2;
	} else {
		if( *ppUTF8Dst + 1 >= pUTF8DstEnd ) {
			return 0;
		}

		( *ppUTF8Dst )[ 0 ] = ( axstr_utf8_t )uCodepoint;
		
		*ppUTF8Dst += 1;
	}

	return 1;
}
#else
;
#endif
/*
 *	Perform one step of a UTF-32 to UTF-16 encoding.
 *
 *	ppUTF16Dst: Points to the pointer of what will be written to. That pointer
 *	-           is incremented by this function upon completion.
 *	pUTF16DstEnd: One element past the end of the buffer referenced by
 *	-             *ppUTF16Dst. No data will be written to or beyond this point.
 *	uCodepoint: Unicode character to encode.
 *
 *	return: 1 on success; 0 on failure.
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_step_utf16_encode( axstr_utf16_t **ppUTF16Dst, axstr_utf16_t *pUTF16DstEnd, axstr_utf32_t uCodepoint )
#if AXSTR_IMPLEMENT
{
	if( uCodepoint >= 0x10000 ) {
		if( *ppUTF16Dst + 2 >= pUTF16DstEnd ) {
			return 0;
		}

		( *ppUTF16Dst )[0] = 0xD800 | ( axstr_utf16_t )( ( uCodepoint >> 10 ) & 0x3FF );
		( *ppUTF16Dst )[1] = 0xDC00 | ( axstr_utf16_t )( ( uCodepoint >>  0 ) & 0x3FF );

		*ppUTF16Dst += 2;
	} else {
		if( *ppUTF16Dst + 1 >= pUTF16DstEnd ) {
			return 0;
		}

		( *ppUTF16Dst )[0] = ( axstr_utf16_t )uCodepoint;

		*ppUTF16Dst += 1;
	}

	return 1;
}
#else
;
#endif

/*
 *	Convert a chunk of UTF-8 data to UTF-16 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf16( axstr_utf16_t *pUTF16Dst, axstr_size_t cMaxDstUTF16Chars, const axstr_utf8_t *pUTF8Src )
#if AXSTR_IMPLEMENT
{
	axstr_utf16_t *dstp;
	axstr_utf16_t *dste;
	const axstr_utf8_t *srcp;
	const axstr_utf8_t *srce;

	dstp = pUTF16Dst;
	dste = dstp + cMaxDstUTF16Chars;
	srcp = pUTF8Src;
	srce = ( const axstr_utf8_t * )( ~( axstr_size_t )0 );

	while( *srcp != '\0' ) {
		axstr_utf32_t uCodepoint;

		uCodepoint = axstr_step_utf8_decode( &srcp, srce );
		if( !axstr_step_utf16_encode( &dstp, dste, uCodepoint ) ) {
			return 0;
		}
	}

	*dstp = ( axstr_utf16_t )'\0';
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-8 data to UTF-16 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf16_n( axstr_utf16_t *pUTF16Dst, axstr_size_t cMaxDstUTF16Chars, const axstr_utf8_t *pUTF8Src, const axstr_utf8_t *pUTF8SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf16_t *dstp;
	axstr_utf16_t *dste;
	const axstr_utf8_t *srcp;
	const axstr_utf8_t *srce;

	dstp = pUTF16Dst;
	dste = dstp + cMaxDstUTF16Chars;
	srcp = pUTF8Src;
	srce = pUTF8SrcEnd;

	while( srcp < srce ) {
		axstr_utf32_t uCodepoint;

		uCodepoint = axstr_step_utf8_decode( &srcp, srce );
		if( !axstr_step_utf16_encode( &dstp, dste, uCodepoint ) ) {
			return 0;
		}
	}

	*dstp = ( axstr_utf16_t )'\0';
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-8 data to UTF-32 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf32( axstr_utf32_t *pUTF32Dst, axstr_size_t cMaxDstUTF32Chars, const axstr_utf8_t *pUTF8Src )
#if AXSTR_IMPLEMENT
{
	axstr_utf32_t *dstp;
	axstr_utf32_t *dste;
	const axstr_utf8_t *srcp;
	const axstr_utf8_t *srce;

	dstp = pUTF32Dst;
	dste = pUTF32Dst + cMaxDstUTF32Chars;
	srcp = pUTF8Src;
	srce = ( const axstr_utf8_t * )( ~( axstr_size_t )0 );

	while( *srcp != '\0' ) {
		if( dstp + 1 >= dste ) {
			break;
		}

		*dstp++ = axstr_step_utf8_decode( &srcp, srce );
	}

	*dstp = 0;
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-8 data to UTF-32 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf32_n( axstr_utf32_t *pUTF32Dst, axstr_size_t cMaxDstUTF32Chars, const axstr_utf8_t *pUTF8Src, const axstr_utf8_t *pUTF8SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf32_t *dstp;
	axstr_utf32_t *dste;
	const axstr_utf8_t *srcp;
	const axstr_utf8_t *srce;

	dstp = pUTF32Dst;
	dste = pUTF32Dst + cMaxDstUTF32Chars;
	srcp = pUTF8Src;
	srce = pUTF8SrcEnd;

	while( srcp < srce ) {
		if( dstp + 1 >= dste ) {
			break;
		}

		*dstp++ = axstr_step_utf8_decode( &srcp, srce );
	}

	*dstp = 0;
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-16 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf16_to_utf8( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf16_t *pUTF16Src )
#if AXSTR_IMPLEMENT
{
	axstr_utf8_t *dstp;
	axstr_utf8_t *dste;
	const axstr_utf16_t *srcp;
	const axstr_utf16_t *srce;

	dstp = pUTF8Dst;
	dste = dstp + cMaxDstUTF8Bytes;
	srcp = pUTF16Src;
	srce = ( const axstr_utf16_t * )( ~( axstr_size_t )0 );

	while( *srcp != '\0' ) {
		axstr_utf32_t uCodepoint;

		uCodepoint = axstr_step_utf16_decode( &srcp, srce );
		if( !axstr_step_utf8_encode( &dstp, dste, uCodepoint ) ) {
			return 0;
		}
	}

	*dstp = '\0';
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-16 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf16_to_utf8_n( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf16_t *pUTF16Src, const axstr_utf16_t *pUTF16SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf8_t *dstp;
	axstr_utf8_t *dste;
	const axstr_utf16_t *srcp;
	const axstr_utf16_t *srce;

	dstp = pUTF8Dst;
	dste = dstp + cMaxDstUTF8Bytes;
	srcp = pUTF16Src;
	srce = pUTF16SrcEnd;

	while( srcp < srce ) {
		axstr_utf32_t uCodepoint;

		uCodepoint = axstr_step_utf16_decode( &srcp, srce );
		if( !axstr_step_utf8_encode( &dstp, dste, uCodepoint ) ) {
			return 0;
		}
	}

	*dstp = '\0';
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-32 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf32_to_utf8( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf32_t *pUTF32Src )
#if AXSTR_IMPLEMENT
{
	axstr_utf8_t *dstp;
	axstr_utf8_t *dste;
	const axstr_utf32_t *srcp;

	dstp = pUTF8Dst;
	dste = pUTF8Dst + cMaxDstUTF8Bytes;
	srcp = pUTF32Src;

	while( *srcp != 0 ) {
		if( !axstr_step_utf8_encode( &dstp, dste, *srcp++ ) ) {
			return 0;
		}
	}

	*dstp = '\0';
	return 1;
}
#else
;
#endif
/*
 *	Convert a chunk of UTF-32 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf32_to_utf8_n( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf32_t *pUTF32Src, const axstr_utf32_t *pUTF32SrcEnd )
#if AXSTR_IMPLEMENT
{
	axstr_utf8_t *dstp;
	axstr_utf8_t *dste;
	const axstr_utf32_t *srcp;
	const axstr_utf32_t *srce;

	dstp = pUTF8Dst;
	dste = pUTF8Dst + cMaxDstUTF8Bytes;
	srcp = pUTF32Src;
	srce = pUTF32SrcEnd;

	while( srcp < srce ) {
		if( !axstr_step_utf8_encode( &dstp, dste, *srcp++ ) ) {
			return 0;
		}
	}

	*dstp = '\0';
	return 1;
}
#else
;
#endif
/*
 *	Write a byte-order-mark (BOM) to a buffer
 *
 *	If bom is set to axstr_bom_enabled then the appropriate byte-order-mark for
 *	the encoding `enc` is used.
 *
 *	return: Number of bytes written to pDstBuf. (Will not exceed cDstBytes.)
 */
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_write_bom( void *pDstBuf, axstr_size_t cDstBytes, axstr_encoding_t enc, axstr_byteordermark_t bom )
#if AXSTR_IMPLEMENT
{
	static const unsigned char UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };
	static const unsigned char UTF16_BE_BOM[] = { 0xFE, 0xFF };
	static const unsigned char UTF16_LE_BOM[] = { 0xFF, 0xFE };
	static const unsigned char UTF32_BE_BOM[] = { 0x00, 0x00, 0xFE, 0xFF };
	static const unsigned char UTF32_LE_BOM[] = { 0xFF, 0xFE, 0x00, 0x00 };

	if( cDstBytes > 4 ) {
		return 0;
	}

	if( bom == axstr_bom_enabled ) {
		const unsigned char *p;
		axstr_size_t n;

		p = ( const unsigned char * )0;
		n = 0;
		
		switch( enc )
		{
		case axstr_enc_unknown:
			break;

		case axstr_enc_utf8:
			p = &UTF8_BOM[0];
			n = sizeof( UTF8_BOM );
			break;
		case axstr_enc_utf16_be:
			p = &UTF16_BE_BOM[0];
			n = sizeof( UTF16_BE_BOM );
			break;
		case axstr_enc_utf16_le:
			p = &UTF16_LE_BOM[0];
			n = sizeof( UTF16_LE_BOM );
			break;
		case axstr_enc_utf32_be:
			p = &UTF32_BE_BOM[0];
			n = sizeof( UTF32_BE_BOM );
			break;
		case axstr_enc_utf32_le:
			p = &UTF32_LE_BOM[0];
			n = sizeof( UTF32_LE_BOM );
			break;
		}

#if AXSTR_STDSTR_ENABLED
		memcpy( pDstBuf, ( const void * )p, n );
#else
		{
			axstr_size_t i;
			for( i=0; i<n; ++i ) {
				( ( unsigned char * )pDstBuf )[ i ] = p[ i ];
			}
		}
#endif

		return n;
	}

	return 0;
}
#else
;
#endif

#if AXSTR_IMPLEMENT
static axstr_bool_t AXSTR_CALL axstr_stream_bytes_( void **ppDstBuf, axstr_size_t *pcDstBytes, const void *pSrc, axstr_size_t cSrcBytes )
{
	axstr_size_t i;
	unsigned char *d;
	const unsigned char *s;

	if( cSrcBytes > *pcDstBytes ) {
		return ( axstr_bool_t )0;
	}

	d = ( unsigned char * )*ppDstBuf;
	s = ( const unsigned char * )pSrc;
	for( i = 0; i < cSrcBytes; ++i ) {
		*d++ = *s++;
	}

	*pcDstBytes -= cSrcBytes;
	*ppDstBuf = ( void * )d;

	return ( axstr_bool_t )1;
}
#endif

/*
 *	Convert a UTF-8 stream to an arbitrary binary encoding with an optional
 *	byte-order-mark. Does not explicitly write a null terminator. (Useful when
 *	writing to a (potentially memory mapped) file.)
 *
 *	This returns the number of bytes written.
 */
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_to_encoding_n( void *pDstBuf, axstr_size_t cDstBytes, const char *pszSrc, axstr_size_t cSrcBytes, axstr_encoding_t enc, axstr_byteordermark_t bom )
#if AXSTR_IMPLEMENT
{
	const axstr_utf8_t *s;
	const axstr_utf8_t *e;
	axstr_size_t n;
	void *d;

	n = axstr_write_bom( pDstBuf, cDstBytes, enc, bom );

	d = ( void * )( ( unsigned char * )pDstBuf + n );
	n = cDstBytes - n;

	s = ( const axstr_utf8_t * )pszSrc;
	e = s + ( cSrcBytes == AXSTR_UNKNOWN_LENGTH ? axstr_len( pszSrc ) : cSrcBytes );

	if( enc == axstr_enc_utf8 ) {
		if( !axstr_stream_bytes_( &d, &n, ( const void * )s, ( axstr_size_t )( e - s ) ) ) {
			return 0;
		}

		return cDstBytes - n;
	}

	do {
		axstr_utf32_t cp;
		unsigned char bytes[ 4 ];
		axstr_size_t nbytes;

		cp = axstr_step_utf8_decode( &s, e );

		nbytes = 0;

		if( enc == axstr_enc_utf16_be || enc == axstr_enc_utf16_le ) {
			axstr_utf16_t utf16[ 2 ];

			if( cp < 0x10000 ) {
				utf16[ 0 ] = ( axstr_utf16_t )cp;
				utf16[ 1 ] = 0;
				nbytes = 2;
			} else {
				utf16[ 0 ] = 0xD800 | ( axstr_utf16_t )( ( cp >> 10 ) & 0x3FF );
				utf16[ 1 ] = 0xDC00 | ( axstr_utf16_t )( ( cp >>  0 ) & 0x3FF );
				nbytes = 4;
			}

			if( enc == axstr_enc_utf16_be ) {
				bytes[ 0 ] = ( unsigned char )( ( utf16[0] & 0xFF00 )>>8 );
				bytes[ 1 ] = ( unsigned char )( ( utf16[0] & 0x00FF )>>0 );
				bytes[ 2 ] = ( unsigned char )( ( utf16[1] & 0xFF00 )>>8 );
				bytes[ 3 ] = ( unsigned char )( ( utf16[1] & 0x00FF )>>0 );
			} else {
				bytes[ 0 ] = ( unsigned char )( ( utf16[0] & 0x00FF )>>0 );
				bytes[ 1 ] = ( unsigned char )( ( utf16[0] & 0xFF00 )>>8 );
				bytes[ 2 ] = ( unsigned char )( ( utf16[1] & 0x00FF )>>0 );
				bytes[ 3 ] = ( unsigned char )( ( utf16[1] & 0xFF00 )>>8 );
			}
		} else if( enc == axstr_enc_utf32_be ) {
			bytes[ 0 ] = ( unsigned char )( ( cp & 0xFF000000 )>>24 );
			bytes[ 1 ] = ( unsigned char )( ( cp & 0x00FF0000 )>>16 );
			bytes[ 2 ] = ( unsigned char )( ( cp & 0x0000FF00 )>> 8 );
			bytes[ 3 ] = ( unsigned char )( ( cp & 0x000000FF )>> 0 );
			nbytes = 4;
		} else {
			bytes[ 0 ] = ( unsigned char )( ( cp & 0x000000FF )>> 0 );
			bytes[ 1 ] = ( unsigned char )( ( cp & 0x0000FF00 )>> 8 );
			bytes[ 2 ] = ( unsigned char )( ( cp & 0x00FF0000 )>>16 );
			bytes[ 3 ] = ( unsigned char )( ( cp & 0xFF000000 )>>24 );
			nbytes = 4;
		}

		if( !axstr_stream_bytes_( &d, &n, &bytes[0], nbytes ) ) {
			return 0;
		}
	} while( s < e );

	return cDstBytes - n;
}
#else
;
#endif
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_to_encoding( void *pDstBuf, axstr_size_t cDstBytes, const char *pszSrc, axstr_encoding_t enc, axstr_byteordermark_t bom )
#if AXSTR_IMPLEMENT
{
	return axstr_to_encoding_n( pDstBuf, cDstBytes, pszSrc, AXSTR_UNKNOWN_LENGTH, enc, bom );
}
#else
;
#endif

Ax::EEncoding Ax::DetectEncoding( const uint8( &buf )[ 4 ], uintptr *pOutLength )
{
	const uint8 *p = ( const uint8 * )&buf[ 0 ];
	
	if( p[0] == 0x00 && p[1] == 0x00 && p[2] == 0xFE && p[3] == 0xFF ) {
		if( pOutLength != nullptr ) {
			*pOutLength = 4;
		}
		return EEncoding::UTF32_BE;
	}

	if( p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF ) {
		if( pOutLength != nullptr ) {
			*pOutLength = 3;
		}
		return EEncoding::UTF8;
	}

	if( p[0] == 0xFE && p[1] == 0xFF ) {
		if( pOutLength != nullptr ) {
			*pOutLength = 2;
		}
		return EEncoding::UTF16_BE;
	}
	if( p[0] == 0xFF && p[1] == 0xFE ) {
		if( p[2] == 0x00 && p[3] == 0x00 ) {
			if( pOutLength != nullptr ) {
				*pOutLength = 4;
			}
			return EEncoding::UTF32_LE;
		}

		if( pOutLength != nullptr ) {
			*pOutLength = 2;
		}
		return EEncoding::UTF16_LE;
	}

	if( pOutLength != nullptr ) {
		*pOutLength = 0;
	}
	return EEncoding::UTF8;
}

bool Ax::CaseCompare( const char *pszFirst, const char *pszSecond, intptr Length )
{
	if( Length < 0 ) {
#ifdef _WIN32
		return _stricmp( pszFirst, pszSecond ) == 0;
#else
		return strcasecmp( pszFirst, pszSecond ) == 0;
#endif
	}

#ifdef _WIN32
	return _strnicmp( pszFirst, pszSecond, uintptr( Length ) ) == 0;
#else
	return strncasecmp( pszFirst, pszSecond, uintptr( Length ) ) == 0;
#endif
}
bool Ax::Compare( const char *pszFirst, const char *pszSecond, intptr Length )
{
	if( Length < 0 ) {
		return strcmp( pszFirst, pszSecond ) == 0;
	}

	return strncmp( pszFirst, pszSecond, uintptr( Length ) ) == 0;
}

char *Ax::StrDup( char *&pszOutDst, const char *pszSrc, int memtag )
{
	char *pszTempDst = NULL;
	if( pszSrc != NULL ) {
		const uintptr cSrc = strlen( pszSrc );
		pszTempDst = ( char * )Alloc( cSrc + 1, memtag );
		if( !pszTempDst ) {
			return NULL;
		}

		memcpy( ( void * )pszTempDst, ( const void * )pszSrc, cSrc + 1 );
	}

	Dealloc( ( void * )pszOutDst );
	pszOutDst = pszTempDst;

	return pszTempDst;
}
char *Ax::StrDupN( char *&pszOutDst, const char *pszSrc, uintptr cSrc, int memtag )
{
	char *pszTempDst = NULL;
	if( pszSrc != NULL ) {
		uintptr cDup;
		for( cDup = 0; pszSrc[ cDup ] != '\0'; ++cDup ) {
			if( cDup == cSrc )
			{
				break;
			}
		}

		pszTempDst = ( char * )Alloc( cDup + 1, memtag );
		if( !pszTempDst ) {
			return NULL;
		}

		memcpy( ( void * )pszTempDst, ( const void * )pszSrc, cDup );
		pszTempDst[ cDup ] = '\0';
	}

	Dealloc( ( void * )pszOutDst );
	pszOutDst = pszTempDst;

	return pszTempDst;
}

const char *Ax::StrCpy( char *dst, uintptr dstn, const char *src )
{
	if( !dst || dstn <= 1 || !src ) {
		return NULL;
	}

	const char *const base = dst;

	while( dstn > 1 && *src != '\0' ) {
		*dst++ = *src++;
		--dstn;
	}

	*dst++ = '\0';

	return base;
}
const char *Ax::StrCpyN( char *dst, uintptr dstn, const char *src, uintptr srcn )
{
	if( !dst || dstn <= 1 || !src ) {
		return NULL;
	}

	const char *const base = dst;
	uintptr n = dstn < srcn ? dstn : srcn;

	while( n > 1 && *src != '\0' ) {
		*dst++ = *src++;
		--n;
	}

	*dst++ = '\0';

	return base;
}
void Ax::AppendString( char *dst, uintptr dstn, uintptr *index, const char *src, uintptr srcn )
{
	uintptr n;
	uintptr i;

	bool search = true;
	if( index != NULL ) {
		i = *index;
		if( i >= dstn ) {
			return;
		}

		if( dst[ i ] == '\0' ) {
			search = false;
		}
	}

	if( search ) {
		const char *const p = strchr( dst, '\0' );
		i = p - dst;
	}

	n = srcn;
	while( i + 1 < dstn && n-- > 0 ) {
		dst[ i++ ] = *src++;
	}

	dst[ i ] = '\0';

	if( index != NULL ) {
		*index = i;
	}
}
void Ax::AppendString( char *dst, uintptr dstn, uintptr *index, const char *src )
{
	return AppendString( dst, dstn, index, src, strlen( src ) );
}

bool Ax::CmpPathChar( char a, char b )
{
#ifdef _WIN32
	a = ( a >= 'A' && a <= 'Z' ) ? a - 'A' + 'a' : ( a == '/' ? '\\' : a );
	b = ( b >= 'A' && b <= 'Z' ) ? b - 'A' + 'a' : ( b == '/' ? '\\' : b );
#endif

	return a == b;
}
bool Ax::IsPathSep( char a )
{
#ifdef _WIN32
	if( a == '\\' ) {
		return true;
	}
#endif

	return a == '/';
}

int Ax::FormatV( char *dst, uintptr dstn, const char *fmt, va_list args )
{
	if( !dst || dstn <= 1 || !fmt || !args ) {
		return 0;
	}

#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	const int r = vsprintf_s( dst, dstn, fmt, args );
#else
	const int r = vsnprintf( dst, dstn, fmt, args );
	dst[ dstn - 1 ] = '\0';
#endif

	return r < 0 ? 0 : r;
}

// retrieve the relative path from one file to another
//
// dst: destination buffer
// dstn: size of destination buffer in bytes
// srcfrom: the file with the base path
// srcto: the file you are getting to from "srcfrom"
//
// in the event of truncation a terminating NULL character will be placed as the
// last byte within the buffer (within the bounds dictated by dstn).
//
// return: dst if inputs are good
const char *Ax::GetRelativePath( char *dst, uintptr dstn, const char *srcfrom, const char *srcto )
{
	// ignore bad inputs
	if( !dst || dstn <= 1 || !srcfrom || !srcto ) {
		return NULL;
	}

	// skip all matching characters
	const char *srcfrom_p = srcfrom, *srcto_p = srcto;
	const char *srcfrom_q = srcfrom, *srcto_q = srcto;
	while( CmpPathChar( *srcfrom_p, *srcto_p ) ) {
		if( IsPathSep( *srcfrom_p ) ) {
			srcfrom_q = srcfrom_p + 1;
			srcto_q = srcto_p + 1;
		}
	
		++srcfrom_p;
		++srcto_p;
	}

	// sometimes directories are passed in without a terminating '/' so respect that
	if( *srcfrom_p == '\0' && ( *srcto_p == '/' || *srcto_p == '\\' ) ) {
		++srcto_p;
	} else {
		srcfrom_p = srcfrom_q;
		srcto_p = srcto_q;
	}

	// if there were no matching characters
	if( srcfrom_p == srcfrom ) {
		// then the destination string is absolute
		return StrCpy( dst, dstn, srcto );
	}

	// append index
	uintptr index = 0;
	*dst = '\0';

	// find how many directories up "from" is from "to"
	while( *srcfrom_p != '\0' ) {
		if( IsPathSep( *srcfrom_p ) ) {
			while( IsPathSep( *( srcfrom_p + 1 ) ) ) {
				++srcfrom_p;
			}

			AppendString( dst, dstn, &index, "../" );
		}

		++srcfrom_p;
	}

	// append the rest of the "to" string
	AppendString( dst, dstn, &index, srcto_p );

	// done
	return dst;
}

const char *Ax::GetAbsolutePath( char *dst, uintptr dstn, const char *relpath )
{
	if( !dst || dstn <= 1 || !relpath ) {
		return NULL;
	}

	*dst = '\0';
#ifdef _WIN32
	GetFullPathNameA( relpath, ( DWORD )dstn, dst, NULL );
#else
	if( dstn < PATH_MAX ) {
		char buf[ PATH_MAX ];
		realpath( relpath, buf );
		StrCpy( dst, dstn, buf );
	} else {
		realpath( relpath, dst );
	}
#endif

	return dst;
}

const char *Ax::StrNStr( const char *a, const char *b, uintptr n )
{
	const char *p;
	const char *q = b;

	do
	{
		p = strchr( q, *a );
		if( !p ) {
			break;
		}

		q = p + 1;
	} while( strncmp( a, p, n ) != 0 );

	return p;
}
const char *Ax::FindLongestMatch( const char *a, const char *b, uintptr alen, uintptr *matchLen )
{
	uintptr testLen = alen;
	while( testLen > 0 ) {
		const char *p = StrNStr( a, b, testLen );
		if( p != NULL ) {
			*matchLen = testLen;
			return p;
		}

		--testLen;
	}

	return NULL;
}

bool Ax::FindCommonText( uintptr nbuff, char *buff, uintptr ntexts, const char *const *texts )
{

	uintptr bestMatchCount = 0;
	uintptr bestMatchIndex = ( uintptr )-1;
	bool matchFound = false;

	uintptr len = strlen( texts[ 0 ] );
	for( uintptr i = 0; texts[ 0 ][ i ] != '\0'; ++i ) {
		uintptr testMatchCount = ( uintptr )-1;
		uintptr tindex;

		for( tindex = 1; tindex < ntexts; ++tindex ) {
			uintptr matchLen;
			const char *p = FindLongestMatch( &texts[ 0 ][ i ], texts[ tindex ], len - i, &matchLen );
			if( !p ) {
				break;
			}

			if( matchLen < testMatchCount ) {
				testMatchCount = matchLen;
			}
		}
		
		if( tindex == ntexts ) {
			if( testMatchCount > bestMatchCount ) {
				bestMatchCount = testMatchCount;
				bestMatchIndex = i;
			}

			matchFound = true;
		}
	}

	if( !matchFound ) {
		return false;
	}

#if defined( __STDC_WANT_SECURE_LIB__ ) && defined( _MSC_VER )
	strncpy_s( buff, nbuff, &texts[ 0 ][ bestMatchIndex ], bestMatchCount );
#else
	uintptr n = nbuff - 1 < bestMatchCount ? nbuff - 1 : bestMatchCount;

	strncpy( buff, &texts[ 0 ][ bestMatchIndex ], n );
	buff[ n ] = '\0';
#endif

	return true;

}

bool Ax::CopyTextToClipboard( const char *pszText, uintptr cText )
{
	AX_ASSERT_NOT_NULL( pszText );

	const uintptr cLen = ( cText == ~uintptr( 0 ) ? strlen( pszText ) : cText );
	( void )cLen;

#ifdef _WIN32
	if( !OpenClipboard( NULL ) ) {
		return false;
	}

	//
	//	Setting the window to NULL in OpenClipboard() will cause
	//	EmptyClipboard() to associate the clipboard with NULL which will then
	//	cause SetClipboardData() to fail
	//
# if 0
	EmptyClipboard();
# endif

	const HGLOBAL hClipMem = GlobalAlloc( GMEM_DDESHARE, cLen + 1 );
	if( !hClipMem ) {
		CloseClipboard();
		return false;
	}

	char *const pszClipBuf = ( char * )GlobalLock( hClipMem );
	if( !pszClipBuf ) {
		GlobalFree( hClipMem );
		CloseClipboard();
		return false;
	}
	memcpy( pszClipBuf, pszText, cLen );
	pszClipBuf[ cLen ] = '\0';

	GlobalUnlock( hClipMem );

	SetClipboardData( CF_TEXT, hClipMem );
	CloseClipboard();
#endif

	return true;
}
bool Ax::PasteTextFromClipboard( char *pszOutText, uintptr cMaxOutText )
{
	AX_ASSERT_NOT_NULL( pszOutText );
	AX_ASSERT( cMaxOutText > 0 );

	*pszOutText = '\0';

#ifdef _WIN32
	if( !OpenClipboard( NULL ) ) {
		return false;
	}

	const char *pszSrc = ( const char * )GetClipboardData( CF_TEXT );
	if( !pszSrc ) {
		CloseClipboard();
		return true;
	}

	StrCpy( pszOutText, cMaxOutText, pszSrc );
	CloseClipboard();
#endif

	return true;
}

/*
===============================================================================

	STRING (CLASS)

===============================================================================
*/

Ax::String::String( const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
}
Ax::String::String( const String &x, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( x );
}
Ax::String::String( const String &x, intptr first, intptr count, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( x, first, count );
}
Ax::String::String( const char *p, intptr count, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( p, count );
}
Ax::String::String( const char *p, intptr first, intptr count, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( p, first, count );
}
Ax::String::String( const char *s, const char *e, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( s, e );
}
Ax::String::String( const wchar_t *p, intptr count, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )AppendUTF16( p, count );
}
Ax::String::String( const wchar_t *p, intptr first, intptr count, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )AppendUTF16( &p[ first ], count );
}
Ax::String::String( const wchar_t *s, const wchar_t *e, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )AppendUTF16( s, ( Ax::intptr )( e - s ) );
}
Ax::String::String( char ch, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( ch );
}
Ax::String::String( wchar_t ch, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )AssignUTF16( ch );
}
Ax::String::String( bool b, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Assign( b ? "true" : "false" );
}
Ax::String::String( int i, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	( void )Format( "%i", i );
}
Ax::String::String( float f, int decimalPlaces, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
	if( decimalPlaces != 0 ) {
		( void )Format( "%.*f", decimalPlaces, f );
	} else {
		( void )Format( "%g", f );
	}
}
Ax::String::String( void *p, const SMemtag &memtag )
: m_cStr( 0 )
, m_cMax( 0 )
, m_pStr( NULL )
, m_iTag( memtag )
{
#ifdef _WIN32
	( void )Format( "0x%p", p );
#else
	( void )Format( "%p", p );
#endif
}

bool Ax::String::Reserve( uintptr numChars )
{
	if( m_cMax > numChars ) {
		return true;
	}

	const uintptr allChars = numChars + 1;
	const uintptr numBytes = allChars%16 == 0 ? allChars : allChars + ( 16 - allChars%16 );

	return SetCapacity( numBytes );
}
bool Ax::String::SetCapacity( uintptr numBytes )
{
	if( numBytes < 0 ) {
		return false;
	}

	char *p = NULL;
	if( numBytes > 0 ) {
		p = reinterpret_cast< char * >( Alloc( numBytes, m_iTag ) );
		if( !p ) {
			return false;
		}

		if( m_cStr > 0 ) {
			const uintptr len = m_cStr < numBytes ? m_cStr : numBytes - 1;
			memcpy( ( void * )p, ( const void * )m_pStr, len + 1 );
			p[ len ] = '\0';
			m_cStr = len;
		} else {
			*p = '\0';
		}
	}

	Dealloc( reinterpret_cast< void * >( m_pStr ) );
	m_pStr = p;

	m_cMax = numBytes;

	return true;
}

bool Ax::String::Assign( const char *p, intptr first, intptr count )
{
	uintptr base, len;
	GetOffsetAndSize( p, base, len, first, count );

	if( !Reserve( len ) ) {
		return false;
	}

	memcpy( ( void * )m_pStr, ( const void * )&p[ base ], len );
	m_pStr[ len ] = '\0';
	m_cStr = len;

	return true;
}
bool Ax::String::Append( const char *p, intptr first, intptr count )
{
	uintptr base, len;
	GetOffsetAndSize( p, base, len, first, count );

	if( !Reserve( m_cStr + len ) ) {
		return false;
	}

	memcpy( ( void * )&m_pStr[ m_cStr ], ( const void * )&p[ base ], len );
	m_pStr[ m_cStr + len ] = '\0';
	m_cStr += len;

	return true;
}
bool Ax::String::Prepend( const char *p, intptr first, intptr count )
{
	return Insert( p, 0, first, count );
}
bool Ax::String::Insert( const char *p, intptr before, intptr first, intptr count )
{
	const uintptr beforeIndex = before < 0 ? m_cStr + 1 + before : before;

	uintptr base, len;
	GetOffsetAndSize( p, base, len, first, count );

	if( !Reserve( m_cStr + len ) ) {
		return false;
	}

	memmove( &m_pStr[ beforeIndex + len ], &m_pStr[ beforeIndex ], m_cStr - beforeIndex );

	memcpy( ( void * )&m_pStr[ beforeIndex ], ( const void * )&p[ base ], len );
	m_cStr += len;
	m_pStr[ m_cStr ] = '\0';

	return true;
}

Ax::intptr Ax::String::Find( char c, uintptr startIndex ) const
{
	if( startIndex >= m_cStr ) {
		return -1;
	}

	const char *const p = strchr( &m_pStr[ startIndex ], c );
	if( !p ) {
		return -1;
	}

	return p - m_pStr;
}
Ax::intptr Ax::String::Find( const char *s, uintptr startIndex ) const
{
	if( startIndex >= m_cStr || !s ) {
		return -1;
	}

	const char *const p = strstr( &m_pStr[ startIndex ], s );
	if( !p ) {
		return -1;
	}

	return p - m_pStr;
}
Ax::intptr Ax::String::FindLast( char c ) const
{
	if( !m_cStr ) {
		return -1;
	}

	const char *const p = strrchr( m_pStr, c );
	if( !p ) {
		return -1;
	}

	return p - m_pStr;
}
Ax::intptr Ax::String::FindLast( const char *s ) const
{
	if( !s || !m_cStr || *s == '\0' ) {
		return -1;
	}

	const uintptr n = strlen( s );
	if( n > m_cStr ) {
		return -1;
	}

	for( intptr i = m_cStr - n; i >= 0; --i ) {
		if( m_pStr[ i ] != *s ) {
			continue;
		}

		if( strncmp( &m_pStr[ i ], s, n ) == 0 ) {
			return i;
		}
	}

	return -1;
}

void Ax::String::Remove( intptr first, intptr count )
{
	/*

		[ H e l l o !   ]
		  0 1 2 3 4 5 6

		Remove 2 .. 4 (2,2 "l l o")

		5 << 2,
		6 << 3

		len -= 2

	*/
	uintptr base, len;
	GetOffsetAndSize( base, len, first, count );

	memmove( &m_pStr[ base ], &m_pStr[ base + len ], m_cStr - ( base + len ) );
	m_cStr -= len;
	m_pStr[ m_cStr ] = '\0';
}
bool Ax::String::Replace( const char *search, const char *substitution )
{
	if( !search || *search == '\0' || !substitution ) {
		return false;
	}

	const uintptr n = strlen( search );
	intptr index = 0;
	bool didFind = false;
	for(;;) {
		index = Find( search, index );
		if( index == -1 ) {
			break;
		}

		didFind = true;
		Remove( index, n );
		if( !Insert( substitution, index ) ) {
			return false;
		}
	}

	return didFind;
}
void Ax::String::Replace( uintptr start, uintptr end, char value )
{
	const uintptr last = end < m_cStr ? end : m_cStr;
	for( uintptr i = start; i < last; ++i ) {
		m_pStr[ i ] = value;
	}
}

#if 0
static const char *NextChar( const char *src, char ch, int skip )
{
	if( *src == '\0' || *( src + skip ) == '\0' ) {
		return NULL;
	}

	const char *p = strchr( src + skip, ch );
	if( !p ) {
		return strchr( src + skip, '\0' );
	}

	return p;
}
bool Ax::String::AppendFormatV( const char *fmt, va_list args )
{
	const int oldLen = m_cStr;

	const char *s = fmt;
	for( const char *p = NextChar( fmt, '%', 0 ); p != NULL; p = NextChar( p, '%', 1 ) ) {
		if( !( p - s ) ) {
			s = p;
			continue;
		}

		if( !Append( s, p - s ) ) {
			goto bail;
		}

		s = p;
		++p;
		if( *p == '%' ) {
			if( !Append( "%" ) ) {
				goto bail;
			}
			s = p + 1;
			continue;
		}

		bool hasLengthArg = false;
		while( *p != '\0' && ( *p < 'a' || *p > 'z' ) && ( *p < 'A' && *p > 'Z' ) ) {
			if( *p == '*' ) {
				hasLengthArg = true;
			}

			++p;
		}

		char fmtbuf[ 512 ];
		int length = ( uintptr )( hasLengthArg ? va_arg( args, uintptr ) : -1 );

		if( ( p - s ) >= ( int )( sizeof( fmtbuf ) - 1 ) ) {
			goto bail;
		}

		memcpy( ( void * )fmtbuf, ( const void * )s, ( p + 1 ) - s );
		fmtbuf[ ( p + 1 ) - s ] = '\0';

		if( *p == 's' ) {
			const char *src = va_arg( args, const char * );
			if( length == -1 ) {
				length = src != NULL ? strlen( src ) : 0;
			}

			if( length == 0 ) {
				s = p + 1;
				continue;
			}

			if( !Reserve( m_cStr + length ) ) {
				goto bail;
			}

#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
			const int r = hasLengthArg ? sprintf_s( &m_pStr[ m_cStr ], m_cMax - m_cStr, fmtbuf, length, src ) : sprintf_s( &m_pStr[ m_cStr ], m_cMax - m_cStr, fmtbuf, src );
#else
			const int r = hasLengthArg ? snprintf( &m_pStr[ m_cStr ], m_cMax - m_cStr, fmtbuf, length, src ) : snprintf( &m_pStr[ m_cStr ], m_cMax - m_cStr, fmtbuf, src );
#endif

			if( r > 0 ) {
				m_cStr += r;
			}
			m_pStr[ m_cStr ] = '\0';

			s = p + 1;
			continue;
		}

		char buf[ 512 ];
		void *arg = va_arg( args, void * );
#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
		const int r = hasLengthArg ? sprintf_s( buf, fmtbuf, length, arg ) : sprintf_s( buf, fmtbuf, arg );
#else
		const int r = hasLengthArg ? snprintf( buf, sizeof( buf ), fmtbuf, length, arg ) : snprintf( buf, sizeof( buf ), fmtbuf, arg );
		buf[ sizeof( buf ) - 1 ] = '\0';
#endif
		if( r < 0 || !Append( buf, r ) ) {
			goto bail;
		}

		s = p + 1;
	}

	return true;

bail:
	if( m_pStr != NULL ) {
		m_pStr[ oldLen ] = '\0';
		m_cStr = oldLen;
	}

	return false;
}
#else
bool Ax::String::AppendFormatV( const char *fmt, va_list args )
{
	char buf[ 8192 ];

#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	vsprintf_s( buf, sizeof( buf ), fmt, args );
#else
	vsnprintf( buf, sizeof( buf ), fmt, args );
	buf[ sizeof( buf ) - 1 ] = '\0';
#endif

	return Append( buf );
}
#endif

Ax::String &Ax::String::ToLower()
{
	for( uintptr i = 0; i < m_cStr; ++i ) {
		if( m_pStr[ i ] >= 'A' && m_pStr[ i ] <= 'Z' ) {
			m_pStr[ i ] = m_pStr[ i ] - 'A' + 'a';
		}
	}
	
	return *this;
}
Ax::String &Ax::String::ToUpper()
{
	for( uintptr i = 0; i < m_cStr; ++i ) {
		if( m_pStr[ i ] >= 'a' && m_pStr[ i ] <= 'z' ) {
			m_pStr[ i ] = m_pStr[ i ] - 'a' + 'A';
		}
	}

	return *this;
}

int Ax::String::LexCaseCompare( const char *other, intptr first, intptr count ) const
{
	if( !m_pStr || !other ) {
		return -1;
	}

	uintptr base, len;
	GetOffsetAndSize( m_pStr, base, len, first, count );

#ifdef _WIN32
	return _strnicmp( &m_pStr[ base ], other, len );
#else
	return strncasecmp( &m_pStr[ base ], other, len );
#endif
}
int Ax::String::LexCompare( const char *other, intptr first, intptr count ) const
{
	if( !m_pStr || !other ) {
		return -1;
	}

	uintptr base, len;
	GetOffsetAndSize( m_pStr, base, len, first, count );

	return strncmp( &m_pStr[ base ], other, len );
}

int Ax::String::LexCaseCompare( const char *other ) const
{
	if( !m_pStr || !other ) {
		return -1;
	}

#ifdef _WIN32
	return _stricmp( m_pStr, other );
#else
	return strcasecmp( m_pStr, other );
#endif
}
int Ax::String::LexCompare( const char *other ) const
{
	if( !m_pStr || !other ) {
		return -1;
	}

	return strcmp( m_pStr, other );
}

Ax::String Ax::String::Escape() const
{
	String temp;
	uintptr first = 0;

	for( uintptr i = 0; i < m_cStr; ++i ) {
		char buf[ 5 ] = { '\0', '\0', '\0', '\0', '\0' };

		switch( m_pStr[ i ] ) {
		case '\a': buf[ 0 ] = '\\'; buf[ 1 ] = 'a'; break;
		case '\b': buf[ 0 ] = '\\'; buf[ 1 ] = 'b'; break;
		case '\f': buf[ 0 ] = '\\'; buf[ 1 ] = 'f'; break;
		case '\n': buf[ 0 ] = '\\'; buf[ 1 ] = 'n'; break;
		case '\r': buf[ 0 ] = '\\'; buf[ 1 ] = 'r'; break;
		case '\t': buf[ 0 ] = '\\'; buf[ 1 ] = 't'; break;
		case '\v': buf[ 0 ] = '\\'; buf[ 1 ] = 'v'; break;
		case '\'': buf[ 0 ] = '\\'; buf[ 1 ] = '\''; break;
		case '\"': buf[ 0 ] = '\\'; buf[ 1 ] = '\"'; break;
		case '\?': buf[ 0 ] = '\\'; buf[ 1 ] = '\?'; break;
		case '\\': buf[ 0 ] = '\\'; buf[ 1 ] = '\\'; break;
		default:
			if( m_pStr[ i ] < 0x20 || ( ( unsigned char * )m_pStr )[ i ] > 0x7F ) {
				const char *const digits = "0123456789ABCDEF";

				const int hi = m_pStr[ i ]/0x10;
				const int lo = m_pStr[ i ]%0x10;
				buf[ 0 ] = '\\';
				buf[ 1 ] = 'x';
				buf[ 2 ] = digits[ hi ];
				buf[ 3 ] = digits[ lo ];
			}
			break;
		}

		if( buf[ 0 ] == '\\' ) {
			if( first != i ) {
				temp.Append( *this, first, i - first );
			}

			first = i + 1;
			temp.Append( buf );
		}
	}

	if( first != m_cStr ) {
		temp.Append( *this, first, m_cStr - first );
	}

	return temp;
}
Ax::String Ax::String::Unescape() const
{
	String temp;

	if( !m_pStr ) {
		return temp;
	}

	intptr index = 0, prevIndex = 0;
	for(;;) {
		index = Find( '\\', prevIndex );
		if( index == -1 ) {
			index = Find( '\0', prevIndex );
		}

		temp.Append( *this, prevIndex, index - prevIndex );
		if( m_pStr[ index ] == '\0' ) {
			break;
		}

		const uintptr i = index + 1;
		prevIndex = i;

		switch( m_pStr[ i ] )
		{
		case 'a': temp.Append( "\a" ); prevIndex = i + 1; break;
		case 'b': temp.Append( "\b" ); prevIndex = i + 1; break;
		case 'f': temp.Append( "\f" ); prevIndex = i + 1; break;
		case 'n': temp.Append( "\n" ); prevIndex = i + 1; break;
		case 'r': temp.Append( "\r" ); prevIndex = i + 1; break;
		case 't': temp.Append( "\t" ); prevIndex = i + 1; break;
		case 'v': temp.Append( "\v" ); prevIndex = i + 1; break;
		// \xnn
		case 'x':
		{
			if( i + 2 < m_cStr ) {
				const int a = GetDigit( m_pStr[ i + 1 ], 16 );
				const int b = GetDigit( m_pStr[ i + 2 ], 16 );
				const char buf[] = { ( char )( a*16 + b ), '\0' };
				temp.Append( buf );
				prevIndex = i + 3;
			}
			break;
		}
		// \unnnn
		case 'u':
		{
			if( i + 4 < m_cStr ) {
				const int a = GetDigit( m_pStr[ i + 1 ], 16 );
				const int b = GetDigit( m_pStr[ i + 2 ], 16 );
				const int c = GetDigit( m_pStr[ i + 3 ], 16 );
				const int d = GetDigit( m_pStr[ i + 4 ], 16 );
				if( a >= 0 && b >= 0 && c >= 0 && d >= 0 ) {
					const int n = a*16*16*16 + b*16*16 + c*16 + d;
					temp.AppendUTF32Char( ( uint32 )n );
				}
				prevIndex = i + 5;
			}
			break;
		}
		// \Unnnnnnnn
		case 'U':
		{
			if( i + 8 < m_cStr ) {
				const int a = GetDigit( m_pStr[ i + 1 ], 16 );
				const int b = GetDigit( m_pStr[ i + 2 ], 16 );
				const int c = GetDigit( m_pStr[ i + 3 ], 16 );
				const int d = GetDigit( m_pStr[ i + 4 ], 16 );
				const int e = GetDigit( m_pStr[ i + 5 ], 16 );
				const int f = GetDigit( m_pStr[ i + 6 ], 16 );
				const int g = GetDigit( m_pStr[ i + 7 ], 16 );
				const int h = GetDigit( m_pStr[ i + 8 ], 16 );

				if( a >= 0 && b >= 0 && c >= 0 && d >= 0 && e >= 0 && f >= 0 && g >= 0 && h >= 0 ) {
					uint32 n = 0;

					n *= 16; n += ( uint32 )a;
					n *= 16; n += ( uint32 )b;
					n *= 16; n += ( uint32 )c;
					n *= 16; n += ( uint32 )d;
					n *= 16; n += ( uint32 )e;
					n *= 16; n += ( uint32 )f;
					n *= 16; n += ( uint32 )g;
					n *= 16; n += ( uint32 )h;

					temp.AppendUTF32Char( n );
				}
				prevIndex = i + 9;
			}
			break;
		}
		default:
			temp.Append( &m_pStr[ i ], 1 );
			prevIndex = i + 1;
			break;
		}
	}

	return temp;
}

Ax::String Ax::String::Quote() const
{
	return "\"" + *this + "\"";
}
Ax::String Ax::String::Unquote() const
{
	if( StartsWith( "\"" ) && EndsWith( "\"" ) ) {
		return Substring( 1, -1 );
	}

	return Copy();
}

bool Ax::String::TabSelf()
{
	if( !Prepend( "\t" ) ) {
		return false;
	}

	const uintptr OriginalLen = m_cStr;

	intptr LastIndex = -1;
	for(;;) {
		const intptr CheckIndex = Find( '\n', LastIndex + 1 );
		if( CheckIndex < 0 ) {
			break;
		}

		LastIndex = CheckIndex + 1;
		if( !Insert( '\t', LastIndex ) ) {
			return false;
		}
	}

	return true;
}
void Ax::String::UntabSelf()
{
	if( !m_cStr ) {
		return;
	}

	uintptr LineStart = 0;
	for(;;) {
		if( m_pStr[ LineStart ] == '\t' ) {
			Remove( intptr( LineStart ), 1 );
		}

		const intptr CheckIndex = Find( '\n', LineStart );
		if( CheckIndex < 0 ) {
			break;
		}

		LineStart = CheckIndex + 1;
	}
}

bool Ax::String::Split( Ax::TArray< String > &OutArray, const char *separator, EKeepEmpty KeepEmpty ) const
{
	OutArray.Clear();

	if( !separator || *separator == '\0' ) {
		return false;
	}

	const uintptr len = strlen( separator );

	intptr index = 0;
	do {
		const intptr newIndex = Find( separator, index );
		const uintptr range = newIndex == -1 ? Len() : newIndex;
		const uintptr next = newIndex == -1 ? range : newIndex + len;
		const uintptr count = range - ( uintptr )index;

		if( count != 0 || KeepEmpty == EKeepEmpty::Yes ) {
			if( !OutArray.Append() ) {
				return false;
			}

			Ax::String &Entry = OutArray.Last();
			if( !Entry.Assign( *this, index, ( intptr )count ) ) {
				return false;
			}
		}

		index = next;
	} while( index != Len() );

	return true;
}
Ax::TArray< Ax::String > Ax::String::Split( const char *separator, EKeepEmpty KeepEmpty ) const
{
	TArray< String > arr;
	
	if( !Split( arr, separator, KeepEmpty ) ) {
		return TArray< String >();
	}

	return arr;
}
Ax::TArray< Ax::String > Ax::String::SplitUnquoted( const char *separator, char chEscape, EKeepQuotes KeepQuotes ) const
{
	TArray< String > arr;

	if( !separator || *separator == '\0' || !m_pStr || m_cStr == 0 ) {
		return arr;
	}

	const uintptr len = strlen( separator );

	uintptr index = 0;
	uintptr p = 0;
	bool bInQuote = false;
	do {
		if( m_pStr[ p ] == '\"' ) {
			bInQuote ^= true;
			++p;
			if( !bInQuote ) {
				if( KeepQuotes == EKeepQuotes::Yes ) {
					arr.Append( Substring( index, p ) );
				} else {
					arr.Append( Substring( index + 1, p - 1 ) );
				}
				if( m_pStr[ p ] == *separator && strncmp( &m_pStr[ p ], separator, len ) == 0 ) {
					p += len;
				}
				if( p == m_cStr ) {
					break;
				}

				index = p;
			}
			continue;
		}

		if( ( m_pStr[ p ] == *separator && strncmp( &m_pStr[ p ], separator, len ) == 0 && !bInQuote ) || p == m_cStr ) {
			arr.Append( Substring( index, p ) );
			p += len;
			index = p;
			continue;
		}

		if( chEscape != '\0' && m_pStr[ p ] == chEscape ) {
			++p;
		}

		++p;
	} while( p <= m_cStr );

	return arr;
}
Ax::String &Ax::String::Merge( const TArray< String > &arr, const char *glue )
{
	Empty();

	const bool appendGlue = glue != NULL && *glue != '\0';
	for( uintptr i = 0; i < arr.Num(); ++i ) {
		Append( arr[ i ] );
		if( appendGlue ) {
			Append( glue );
		}
	}

	return *this;
}

Ax::intptr Ax::String::FindExtension() const
{
	const intptr i = FindLast( '.' );
	if( i == -1 ) {
		return -1;
	}

	if( Find( '/', uintptr( i ) + 1 ) != - 1 ) {
		return -1;
	}
#ifdef _WIN32
	if( Find( '\\', uintptr( i ) + 1 ) != -1 ) {
		return -1;
	}
#endif

	return i;
}

Ax::String Ax::String::ExtractExtension() const
{
	const intptr i = FindExtension();
	if( i == -1 ) {
		return String();
	}

	return String( *this, i, m_cStr - i );
}
Ax::String Ax::String::ExtractFilename() const
{
#ifdef _WIN32
	const intptr x = FindLast( '/' );
	const intptr y = FindLast( '\\' );
	const intptr i = ( x > y ? x : y ) + 1;
	if( !i ) {
		return *this;
	}

	return String( *this, i, m_cStr + i );
#else
	const intptr i = FindLast( '/' ) + 1;
	if( !i ) {
		return *this;
	}

	return String( *this, i, m_cStr - i );
#endif
}
Ax::String Ax::String::ExtractDirectory() const
{
#ifdef _WIN32
	const intptr x = FindLast( '/' );
	const intptr y = FindLast( '\\' );
	const intptr i = ( x > y ? x : y ) + 1;
	if( !i ) {
		return String();
	}

	return String( *this, ( intptr )0, i );
#else
	const int i = FindLast( '/' ) + 1;
	if( !i ) {
		return String();
	}

	return String( *this, ( intptr )0, i );
#endif
}
Ax::String Ax::String::ExtractBasename() const
{
#ifdef _WIN32
	const intptr x = FindLast( '/' );
	const intptr y = FindLast( '\\' );
	const intptr i = ( x > y ? x : y ) + 1;
#else
	const intptr i = FindLast( '/' ) + 1;
#endif

	return Substring( i, FindExtension() );
}

bool Ax::String::ReplaceExtension( const char *pszNewExt )
{
	const intptr i = FindExtension();
	if( i >= 0 ) {
		m_pStr[ i ] = '\0';
		m_cStr = i;
	}

	return Append( pszNewExt );
}

Ax::String &Ax::String::operator*=( int n )
{
	if( n <= 0 ) {
		Empty();
		return *this;
	}

	if( !Reserve( m_cStr*n ) ) {
		return *this;
	}

	String temp = *this;

	while( n > 1 ) {
		--n;
		Append( temp );
	}

	return *this;
}

namespace Ax { namespace Detail {
	struct SUnicodeRange
	{
		Ax::uint32					uStart;
		Ax::uint32					uEnd;
	};
}}
#define R1(CPValue_)				{ CPValue_, CPValue_ }
#define R2(CPStart_,CPEnd_)			{ CPStart_, CPEnd_   }

bool Ax::String::IsUnicodeNameStart( uint32 uCodePoint )
{
	static const Detail::SUnicodeRange Ranges[] = {
		R1(0x00A8), R1(0x00AA), R1(0x00AD), R1(0x00AF),
		R2(0x00B2,0x00B5), R2(0x00B7,0x00BA),
		R2(0x00BC,0x00BE), R2(0x00C0,0x00D6), R2(0x00D8,0x00F6), R2(0x00F8,0x00FF),
		R2(0x0100,0x02FF), R2(0x0370,0x167F), R2(0x1681,0x180D), R2(0x180F,0x1DBF),
		R2(0x1E00,0x1FFF),
		R2(0x200B,0x200D), R2(0x202A,0x202E), R2(0x203F,0x2040), R1(0x2054), R2(0x2060,0x20CF),
		R2(0x2100,0x218F), R2(0x2460,0x24FF), R2(0x2776,0x2793),
		R2(0x2C00,0x2DFF), R2(0x2E80,0x2FFF),
		R2(0x3004,0x3007), R2(0x3021,0x302F), R2(0x3031,0xD7FF),
		R2(0xF900,0xFD3D), R2(0xFD40,0xFDCF), R2(0xFDF0,0xFE1F), R2(0xFE30,0xFE44),
		R2(0xFE47,0xFFFD),
		R2(0x10000,0x1FFFD), R2(0x20000,0x2FFFD), R2(0x30000,0x3FFFD), R2(0x40000,0x4FFFD),
		R2(0x50000,0x5FFFD), R2(0x60000,0x6FFFD), R2(0x70000,0x7FFFD), R2(0x80000,0x8FFFD),
		R2(0x90000,0x9000D), R2(0xA0000,0xAFFFD), R2(0xB0000,0xBFFFD), R2(0xC0000,0xCFFFD),
		R2(0xD0000,0xDFFFD), R2(0xE0000,0xEFFFD)
	};

	for( uint32 i = 0; i < sizeof( Ranges )/sizeof( Ranges[ 0 ] ); ++i ) {
		if( uCodePoint >= Ranges[ i ].uStart && uCodePoint <= Ranges[ i ].uEnd ) {
			return true;
		}
	}

	return false;
}
bool Ax::String::IsUnicodeName( uint32 uCodePoint )
{
	static const Detail::SUnicodeRange Ranges[] = {
		R2(0x0300,0x036F), R2(0x1DC0,0x1DFF), R2(0x20D0,0x20FF), R2(0xFE20,0xFE2F)
	};

	if( IsUnicodeNameStart( uCodePoint ) ) {
		return true;
	}

	for( uint32 i = 0; i < sizeof( Ranges )/sizeof( Ranges[ 0 ] ); ++i ) {
		if( uCodePoint >= Ranges[ i ].uStart && uCodePoint <= Ranges[ i ].uEnd ) {
			return true;
		}
	}

	return false;
}

bool Ax::String::AppendUTF32Char( uint32 utf32Char )
{
	char buf[ 5 ] = { '\0', '\0', '\0', '\0', '\0' };

	// check whether a four-byte encoding is necessary
	if( utf32Char > 0x10000 ) {
		buf[ 0 ] = ( char )( 0xF0 | ( ( utf32Char>>18 ) & 0x07 ) );
		buf[ 1 ] = ( char )( 0x80 | ( ( utf32Char>>12 ) & 0x3F ) );
		buf[ 2 ] = ( char )( 0x80 | ( ( utf32Char>>6 ) & 0x3F ) );
		buf[ 3 ] = ( char )( 0x80 | ( utf32Char & 0x3F ) );
	// check whether a three-byte encoding is necessary
	} else if( utf32Char > 0x7FF ) {
		buf[ 0 ] = ( char )( 0xE0 | ( ( utf32Char>>12 ) & 0x0F ) );
		buf[ 1 ] = ( char )( 0x80 | ( ( utf32Char>>6 ) & 0x3F ) );
		buf[ 2 ] = ( char )( 0x80 | ( utf32Char & 0x3F ) );
	// check whether a two-byte encoding is necessary
	} else if( utf32Char > 0x7F ) {
		buf[ 0 ] = ( char )( 0xC0 | ( ( utf32Char>>6 ) & 0x1F ) );
		buf[ 1 ] = ( char )( 0x80 | ( utf32Char & 0x3F ) );
	// use a single-byte encoding
	} else {
		buf[ 0 ] = ( char )( utf32Char & 0x7F );
	}

	// append
	return Append( buf );
}
bool Ax::String::AppendUTF16( const wchar_t *src, intptr count )
{
	const wchar_t *const base = src;
	while( *src != L'\0' ) {
		if( count >= 0 && ( int )( src - base ) >= count ) {
			break;
		}

		uint32 utf32Char = 0;

		// check if the value is encoded as one UTF16 word
		if( *src < 0xD800 || *src > 0xDFFF ) {
			utf32Char = *src++;
		// check for an error (0xD800..0xDBFF)
		} else if( *src > 0xDBFF ) {
			// this is an invalid character; replace with U+FFFD
			utf32Char = 0xFFFD;
			// two characters expected; increment twice
			src += 2;
		// convert to UTF32
		} else {
			// if no character follows, or is out of range, then this is an invalid encoding
			if( *( src + 1 ) < 0xDC00 || *( src + 1 ) > 0xDFFF ) {
				// replace with U+FFFD
				utf32Char = 0xFFFD;
				++src;
				continue;
			}

			// encode
			utf32Char = 0x10000 + ( ( ( ( *src ) & 0x3FF ) << 10 ) | ( ( *( src + 1 ) ) & 0x3FF ) );
			src += 2;
		}

		// append the UTF-32 character
		if( !AppendUTF32Char( utf32Char ) ) {
			return false;
		}
	}

	return true;
}

Ax::uintptr Ax::String::UTF8Len() const
{
	uintptr n = 0;
	uintptr i = 0;

	while( i < m_cStr ) {
		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			i += 4;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			i += 3;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			i += 2;
		} else {
			i += 1;
		}

		++n;
	}

	return n;
}
Ax::intptr Ax::String::UTF8Index( uintptr charIndex ) const
{
	if( charIndex > m_cStr ) {
		return -1;
	}

	uintptr n = 0;
	uintptr i = 0;

	while( i < m_cStr ) {
		if( n == charIndex ) {
			return i;
		}

		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			i += 4;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			i += 3;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			i += 2;
		} else {
			i += 1;
		}

		++n;
	}

	if( n == charIndex ) {
		return i;
	}

	return -1;
}
Ax::uint32 Ax::String::UTF32Character( uintptr charIndex ) const
{
	intptr utf8Index = UTF8Index( charIndex );
	if( utf8Index < 0 ) {
		return 0;
	}
	
	return CodePointFromOffset( uintptr( utf8Index ) );
}
Ax::uint32 Ax::String::CodePointFromOffset( Ax::uintptr uByteOffset, Ax::uintptr *pOutNumBytesEncoded ) const
{
	if( uByteOffset >= m_cStr ) {
		return 0;
	}

	const uintptr i = uByteOffset;
	uint32 utf32Char;

	if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
		if( pOutNumBytesEncoded != nullptr ) {
			*pOutNumBytesEncoded = 4;
		}

		if( i + 3 >= m_cStr ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x07 ) ) << 18;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 12;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  6;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 3 ] & 0x3F ) ) <<  0;
	} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
		if( pOutNumBytesEncoded != nullptr ) {
			*pOutNumBytesEncoded = 3;
		}

		if( i + 2 >= m_cStr ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x0F ) ) << 12;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) <<  6;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  0;
	} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
		if( pOutNumBytesEncoded != nullptr ) {
			*pOutNumBytesEncoded = 2;
		}

		if( i + 1 >= m_cStr ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x1F ) ) << 6;
		utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 0;
	} else {
		if( pOutNumBytesEncoded != nullptr ) {
			*pOutNumBytesEncoded = 1;
		}

		utf32Char = ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x7F );
	}

	return utf32Char;
}
static bool IsUTF8LongEnough( const char *pszUTF8Source, Ax::uintptr cExpectedLength, Ax::uintptr *pOutNumBytesEncoded )
{
	if( !pszUTF8Source ) {
		return false;
	}

	for( Ax::uintptr i = 0; i < cExpectedLength; ++i ) {
		if( pszUTF8Source[ i ] == '\0' ) {
			if( pOutNumBytesEncoded != nullptr ) {
				*pOutNumBytesEncoded = i;
			}
			return false;
		}
	}

	if( pOutNumBytesEncoded != nullptr ) {
		*pOutNumBytesEncoded = cExpectedLength;
	}
	return true;
}
Ax::uint32 Ax::String::UTF8CodePoint( const char *pszUTF8Source, uintptr *pOutNumBytesEncoded )
{
	if( !pszUTF8Source ) {
		return 0;
	}

	const char *const p = pszUTF8Source;
	uint32 utf32Char;

	if( ( p[ 0 ] & 0xF0 ) == 0xF0 ) {
		if( !IsUTF8LongEnough( p, 4, pOutNumBytesEncoded ) ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( p[ 0 ] & 0x07 ) ) << 18;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 1 ] & 0x3F ) ) << 12;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 2 ] & 0x3F ) ) <<  6;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 3 ] & 0x3F ) ) <<  0;
	} else if( ( p[ 0 ] & 0xE0 ) == 0xE0 ) {
		if( !IsUTF8LongEnough( p, 3, pOutNumBytesEncoded ) ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( p[ 0 ] & 0x0F ) ) << 12;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 1 ] & 0x3F ) ) <<  6;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 2 ] & 0x3F ) ) <<  0;
	} else if( ( p[ 0 ] & 0xC0 ) == 0xC0 ) {
		if( !IsUTF8LongEnough( p, 2, pOutNumBytesEncoded ) ) {
			return 0xFFFD;
		}

		utf32Char  = ( ( uint32 )( uint8 )( p[ 0 ] & 0x1F ) ) << 6;
		utf32Char |= ( ( uint32 )( uint8 )( p[ 1 ] & 0x3F ) ) << 0;
	} else {
		if( pOutNumBytesEncoded != nullptr ) {
			*pOutNumBytesEncoded = 1;
		}

		utf32Char = ( uint32 )( uint8 )( p[ 0 ] & 0x7F );
	}

	return utf32Char;
}

Ax::uintptr Ax::String::CountUTF16CodePoints() const
{
	uintptr n = 0;
	uintptr i = 0;

	while( i < m_cStr ) {
		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			i += 2;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			i += 1;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			i += 1;
		} else {
			i += 1;
		}

		++n;
	}

	return n;
}
bool Ax::String::ConvertUTF16( uint16 *utf16, uintptr maxUtf16CodePoints, uintptr *pOutNumCodePoints ) const
{
	AX_ASSERT_NOT_NULL( utf16 );
	AX_ASSERT( maxUtf16CodePoints > 0 );

	uintptr i = 0;
	uintptr n = 0;
	while( i < m_cStr ) {
		uint32 utf32Char;

		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			if( i + 3 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x07 ) ) << 18;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 12;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 3 ] & 0x3F ) ) <<  0;

			i += 4;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			if( i + 2 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x0F ) ) << 12;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) <<  6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  0;

			i += 3;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			if( i + 1 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x1F ) ) << 6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 0;

			i += 2;
		} else {
			utf32Char = ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x7F );
			i += 1;
		}

		uint16 cp[ 2 ] = { 0, 0 };
		int numcp = 0;

		if( utf32Char < 0x10000 ) {
			cp[ 0 ] = ( uint16 )utf32Char;
			numcp = 1;
		} else {
			cp[ 0 ] = 0xD800 | ( uint16 )( ( utf32Char >> 10 ) & 0x3FF );
			cp[ 1 ] = 0xDC00 | ( uint16 )( ( utf32Char >>  0 ) & 0x3FF );
			numcp = 2;
		}

		if( n + numcp >= maxUtf16CodePoints ) {
			return false;
		}

		for( int j = 0; j < numcp; ++j ) {
			utf16[ n + j ] = cp[ j ];
		}
		n += numcp;
	}

	utf16[ n ] = L'\0';
	if( pOutNumCodePoints != nullptr ) {
		*pOutNumCodePoints = n + 1;
	}
	return true;
}
bool Ax::String::ConvertUTF32( uint32 *utf32, uintptr maxUtf32CodePoints, uintptr *pOutNumCodePoints ) const
{
	AX_ASSERT_NOT_NULL( utf32 );
	AX_ASSERT( maxUtf32CodePoints > 0 );

	uintptr i = 0;
	uintptr n = 0;
	while( i < m_cStr ) {
		uint32 utf32Char;

		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			if( i + 3 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x07 ) ) << 18;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 12;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 3 ] & 0x3F ) ) <<  0;

			i += 4;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			if( i + 2 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x0F ) ) << 12;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) <<  6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  0;

			i += 3;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			if( i + 1 >= m_cStr ) {
				break;
			}

			utf32Char  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x1F ) ) << 6;
			utf32Char |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 0;

			i += 2;
		} else {
			utf32Char = ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x7F );
			i += 1;
		}

		if( n + 1 >= maxUtf32CodePoints ) {
			return false;
		}

		utf32[ n ] = utf32Char;
		++n;
	}

	utf32[ n ] = 0;
	if( pOutNumCodePoints != nullptr ) {
		*pOutNumCodePoints = n + 1;
	}
	return true;
}

bool Ax::String::ToUTF16( TArray< uint16 > &UTF16Arr ) const
{
	uintptr numcodepoints;

	const uintptr maxcodepoints = Len();
	if( !UTF16Arr.Resize( maxcodepoints + 1, L'\0' ) ) {
		return false;
	}

	if( !ConvertUTF16( UTF16Arr.Pointer(), maxcodepoints + 1, &numcodepoints ) ) {
		return false;
	}

	UTF16Arr.Resize( numcodepoints );
	return true;
}
bool Ax::String::ToUTF32( TArray< uint32 > &UTF32Arr ) const
{
	uintptr numcodepoints;

	const uintptr maxcodepoints = Len();
	if( !UTF32Arr.Resize( maxcodepoints + 1, 0 ) ) {
		return false;
	}

	if( !ConvertUTF32( UTF32Arr.Pointer(), maxcodepoints, &numcodepoints ) ) {
		return false;
	}

	UTF32Arr.Resize( numcodepoints );
	return true;
}

Ax::TArray< Ax::uint16 > Ax::String::AsUTF16() const
{
	TArray< uint16 > arr;

	if( !ToUTF16( arr ) ) {
		return TArray< uint16 >();
	}

	return arr;
}
Ax::TArray< Ax::uint32 > Ax::String::AsUTF32() const
{
	TArray< uint32 > arr;

	if( !ToUTF32( arr ) ) {
		return TArray< uint32 >();
	}

	return arr;
}

Ax::TArray< Ax::uint8 > Ax::String::ConvertToEncoding( EEncoding Encoding, EByteOrderMark BOM ) const
{
	static const uint8 UTF8_BOM[] = { 0xEF, 0xBB, 0xBF };
	static const uint8 UTF16_BE_BOM[] = { 0xFE, 0xFF };
	static const uint8 UTF16_LE_BOM[] = { 0xFF, 0xFE };
	static const uint8 UTF32_BE_BOM[] = { 0x00, 0x00, 0xFE, 0xFF };
	static const uint8 UTF32_LE_BOM[] = { 0xFF, 0xFE, 0x00, 0x00 };
	TArray< uint8 > Data;

	if( Encoding == EEncoding::Unknown ) {
		return Data;
	}

	uintptr cpsize = 1;
	if( Encoding == EEncoding::UTF16_BE || Encoding == EEncoding::UTF16_LE ) {
		cpsize = 2;
	}
	if( Encoding == EEncoding::UTF32_BE || Encoding == EEncoding::UTF32_LE ) {
		cpsize = 4;
	}

	if( !Data.Reserve( 4 + m_cStr*cpsize ) ) {
		return Data;
	}

	bool bSuccessful = true;
	if( BOM == EByteOrderMark::Enabled ) {
		switch( Encoding )
		{
		case EEncoding::UTF8:
			Data.Append( UTF8_BOM );
			break;

		case EEncoding::UTF16_BE:
			Data.Append( UTF16_BE_BOM );
			break;
		case EEncoding::UTF16_LE:
			Data.Append( UTF16_LE_BOM );
			break;

		case EEncoding::UTF32_BE:
			Data.Append( UTF32_BE_BOM );
			break;
		case EEncoding::UTF32_LE:
			Data.Append( UTF32_LE_BOM );
			break;
		}
	}

	if( Encoding == EEncoding::UTF8 ) {
		Data.Append( m_cStr, ( const uint8 * )m_pStr );
		return Data;
	}

	uintptr i = 0;
	while( i < m_cStr ) {
		uint32 cp;

		uint8 bytes[ 4 ] = { 0, 0, 0, 0 };
		uintptr nbytes = 0;

		uint16 utf16[ 2 ] = { 0, 0 };

		// Get the Unicode code point at this offset
		if( ( m_pStr[ i ] & 0xF0 ) == 0xF0 ) {
			if( i + 3 >= m_cStr ) {
				break;
			}

			cp  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x07 ) ) << 18;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 12;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  6;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 3 ] & 0x3F ) ) <<  0;

			i += 4;
		} else if( ( m_pStr[ i ] & 0xE0 ) == 0xE0 ) {
			if( i + 2 >= m_cStr ) {
				break;
			}

			cp  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x0F ) ) << 12;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) <<  6;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 2 ] & 0x3F ) ) <<  0;

			i += 3;
		} else if( ( m_pStr[ i ] & 0xC0 ) == 0xC0 ) {
			if( i + 1 >= m_cStr ) {
				break;
			}

			cp  = ( ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x1F ) ) << 6;
			cp |= ( ( uint32 )( uint8 )( m_pStr[ i + 1 ] & 0x3F ) ) << 0;

			i += 2;
		} else {
			cp = ( uint32 )( uint8 )( m_pStr[ i + 0 ] & 0x7F );
			i += 1;
		}

		// Encode into the desired format
		switch( Encoding )
		{
		case EEncoding::UTF16_BE:
		case EEncoding::UTF16_LE:
			if( cp < 0x10000 ) {
				utf16[ 0 ] = ( uint16 )cp;
				nbytes = 2;
			} else {
				utf16[ 0 ] = 0xD800 | ( uint16 )( ( cp >> 10 ) & 0x3FF );
				utf16[ 1 ] = 0xDC00 | ( uint16 )( ( cp >>  0 ) & 0x3FF );
				nbytes = 4;
			}
			if( Encoding == EEncoding::UTF16_BE ) {
				bytes[ 0 ] = uint8( ( utf16[ 0 ] & 0xFF00 )>>8 );
				bytes[ 1 ] = uint8( ( utf16[ 0 ] & 0x00FF )>>0 );
				bytes[ 2 ] = uint8( ( utf16[ 1 ] & 0xFF00 )>>8 );
				bytes[ 3 ] = uint8( ( utf16[ 1 ] & 0x00FF )>>0 );
			} else {
				bytes[ 0 ] = uint8( ( utf16[ 0 ] & 0x00FF )>>0 );
				bytes[ 1 ] = uint8( ( utf16[ 0 ] & 0xFF00 )>>8 );
				bytes[ 2 ] = uint8( ( utf16[ 1 ] & 0x00FF )>>0 );
				bytes[ 3 ] = uint8( ( utf16[ 1 ] & 0xFF00 )>>8 );
			}
			break;

		case EEncoding::UTF32_BE:
			bytes[ 0 ] = uint8( ( cp & 0xFF000000 )>>24 );
			bytes[ 1 ] = uint8( ( cp & 0x00FF0000 )>>16 );
			bytes[ 2 ] = uint8( ( cp & 0x0000FF00 )>>8 );
			bytes[ 3 ] = uint8( ( cp & 0x000000FF )>>0 );
			nbytes = 4;
			break;

		case EEncoding::UTF32_LE:
			bytes[ 0 ] = uint8( ( cp & 0x000000FF )>>0 );
			bytes[ 1 ] = uint8( ( cp & 0x0000FF00 )>>8 );
			bytes[ 2 ] = uint8( ( cp & 0x00FF0000 )>>16 );
			bytes[ 3 ] = uint8( ( cp & 0xFF000000 )>>24 );
			nbytes = 4;
			break;

		default:
			// Should be impossible to reach
			break;
		}

		// Write the data
		Data.Append( nbytes, bytes );
	}

	// Done
	return Data;
}
bool Ax::String::ConvertFromEncoding( const TArray< uint8 > &Data, EEncoding InEncoding )
{
	uintptr cBytes = Data.Num();
	const uint8 *pBytes = Data.Pointer();

	EEncoding Encoding = InEncoding;
	if( Encoding == EEncoding::Unknown ) {
		uintptr cBOMBytes = 0;
		uint8 BOM[ 4 ];

		BOM[ 0 ] = cBytes > 0 ? pBytes[ 0 ] : 0;
		BOM[ 1 ] = cBytes > 1 ? pBytes[ 1 ] : 0;
		BOM[ 2 ] = cBytes > 2 ? pBytes[ 2 ] : 0;
		BOM[ 3 ] = cBytes > 3 ? pBytes[ 3 ] : 0;

		Encoding = DetectEncoding( BOM, &cBOMBytes );

		pBytes += cBOMBytes;
		cBytes -= cBOMBytes;
	}

	if( !Reserve( cBytes ) ) {
		return false;
	}

	m_cStr = 0;

	uintptr cBytesPerChar = 1;
	switch( Encoding ) {
	case EEncoding::Unknown:
	case EEncoding::UTF8:
		cBytesPerChar = 1;
		break;

	case EEncoding::UTF16_BE:
	case EEncoding::UTF16_LE:
		cBytesPerChar = 2;
		break;

	case EEncoding::UTF32_BE:
	case EEncoding::UTF32_LE:
		cBytesPerChar = 4;
		break;

	default:
		return false;
	}

	if( Encoding == EEncoding::Unknown || Encoding == EEncoding::UTF8 ) {
		memcpy( ( void * )m_pStr, ( const void * )pBytes, cBytes );
		m_pStr[ cBytes ] = '\0';
		m_cStr = cBytes;
		return true;
	}

	while( cBytes >= cBytesPerChar ) {
		uint32 cp = 0;

		// Decode from the input stream
		if( Encoding == EEncoding::UTF16_BE || Encoding == EEncoding::UTF16_LE ) {
			uint16 utf16[ 2 ];
			uintptr utf16n = 1;

			if( Encoding == EEncoding::UTF16_BE ) {
				utf16[ 0 ] = uint16( pBytes[ 0 ] )<<8 | pBytes[ 1 ];
			} else {
				utf16[ 0 ] = uint16( pBytes[ 1 ] )<<8 | pBytes[ 0 ];
			}

			pBytes += 2;
			cBytes -= 2;

			if( utf16[ 0 ] < 0xD800 || utf16[ 0 ] > 0xDFFF ) {
				cp = uint32( utf16[ 0 ] );
			} else if( utf16[ 0 ] > 0xDBFF ) {
				// Invalid character (replace with U+FFFD)
				cp = 0xFFFD;

				// Two characters expected; increment once more
				pBytes += 2;
				cBytes -= 2;
			} else {
				if( Encoding == EEncoding::UTF16_BE ) {
					utf16[ 1 ] = uint16( pBytes[ 0 ] )<<8 | pBytes[ 1 ];
				} else {
					utf16[ 1 ] = uint16( pBytes[ 1 ] )<<8 | pBytes[ 0 ];
				}

				pBytes += 2;
				cBytes -= 2;

				if( utf16[ 1 ] < 0xDC00 || utf16[ 1 ] > 0xDFFF ) {
					cp = 0xFFFD;
				} else {
					cp = 0x100000 + ( ( ( utf16[ 0 ] & 0x3FF ) << 10 ) | ( utf16[ 1 ] & 0x3FF ) );
				}
			}
		} else if( Encoding == EEncoding::UTF32_BE ) {
			cp = uint32( pBytes[ 0 ] )<<24 | uint32( pBytes[ 1 ] )<<16 | uint32( pBytes[ 2 ] )<<8 | uint32( pBytes[ 3 ] );
			pBytes += 4;
			cBytes -= 4;
		} else { //if( Encoding == EEncoding::UTF32_LE ) {
			cp = uint32( pBytes[ 3 ] )<<24 | uint32( pBytes[ 2 ] )<<16 | uint32( pBytes[ 1 ] )<<8 | uint32( pBytes[ 0 ] );
			pBytes += 4;
			cBytes -= 4;
		}

		// Encode into UTF-8
		if( cp > 0x10000 ) {
			m_pStr[ m_cStr + 0 ] = uint8( 0xF0 | ( ( cp>>18 ) & 0x07 ) );
			m_pStr[ m_cStr + 1 ] = uint8( 0x80 | ( ( cp>>12 ) & 0x3F ) );
			m_pStr[ m_cStr + 2 ] = uint8( 0x80 | ( ( cp>>6 ) & 0x3F ) );
			m_pStr[ m_cStr + 3 ] = uint8( 0x80 | ( ( cp>>0 ) & 0x3F ) );
			m_cStr += 4;
		} else if( cp > 0x7FF ) {
			m_pStr[ m_cStr + 0 ] = uint8( 0xE0 | ( ( cp>>12 ) & 0x0F ) );
			m_pStr[ m_cStr + 1 ] = uint8( 0x80 | ( ( cp>>6 ) & 0x3F ) );
			m_pStr[ m_cStr + 2 ] = uint8( 0x80 | ( ( cp>>0 ) & 0x3F ) );
			m_cStr += 3;
		} else if( cp > 0x7F ) {
			m_pStr[ m_cStr + 0 ] = uint8( 0xC0 | ( ( cp>>6 ) & 0x1F ) );
			m_pStr[ m_cStr + 1 ] = uint8( 0x80 | ( ( cp>>0 ) & 0x3F ) );
			m_cStr += 2;
		} else {
			m_pStr[ m_cStr + 0 ] = uint8( cp & 0x7F );
			m_cStr += 1;
		}
	}

	m_pStr[ m_cStr ] = '\0';
	return true;
}
static const Ax::uint32 g_KatakanaConversions[] = {
	0x0000FF67, // �	30A1
	0x0000FF71, // �	30A2
	0x0000FF68, // �	30A3
	0x0000FF72, // �	30A4
	0x0000FF69, // �	30A5
	0x0000FF73, // �	30A6
	0x0000FF6A, // �	30A7
	0x0000FF74, // �	30A8
	0x0000FF6B, // �	30A9
	0x0000FF75, // �	30AA
	0x0000FF76, // �	30AB
	0xFF9EFF76, // ��	30AC
	0x0000FF77, // �	30AD
	0xFF9EFF77, // ��	30AE
	0x0000FF78, // �	30AF
	0xFF9EFF78, // ��	30B0
	0x0000FF79, // �	30B1
	0xFF9EFF79, // ��	30B2
	0x0000FF7A, // �	30B3
	0xFF9EFF7A, // ��	30B4
	0x0000FF7B, // �	30B5
	0xFF9EFF7B, // ��	30B6
	0x0000FF7C, // �	30B7
	0xFF9EFF7C, // ��	30B8
	0x0000FF7D, // �	30B9
	0xFF9EFF7D, // ��	30BA
	0x0000FF7E, // �	30BB
	0xFF9EFF7E, // ��	30BC
	0x0000FF7F, // �	30BD
	0xFF9EFF7F, // ��	30BE
	0x0000FF80, // �	30BF
	0xFF9EFF80, // ��	30C0
	0x0000FF81, // �	30C1
	0xFF9EFF81, // ��	30C2
	0x0000FF6F, // �	30C3
	0x0000FF82, // �	30C4
	0xFF9EFF82, // ��	30C5
	0x0000FF83, // �	30C6
	0xFF9EFF83, // ��	30C7
	0x0000FF84, // �	30C8
	0xFF9EFF84, // ��	30C9
	0x0000FF85, // �	30CA
	0x0000FF86, // �	30CB
	0x0000FF87, // �	30CC
	0x0000FF88, // �	30CD
	0x0000FF89, // �	30CE
	0x0000FF8A, // �	30CF
	0xFF9EFF8A, // ��	30D0
	0xFF9FFF8A, // ��	30D1
	0x0000FF8B, // �	30D2
	0xFF9EFF8B, // ��	30D3
	0xFF9FFF8B, // ��	30D4
	0x0000FF8C, // �	30D5
	0xFF9EFF8C, // ��	30D6
	0xFF9FFF8C, // ��	30D7
	0x0000FF8D, // �	30D8
	0xFF9EFF8D, // ��	30D9
	0xFF9FFF8D, // ��	30DA
	0x0000FF8E, // �	30DB
	0xFF9EFF8E, // ��	30DC
	0xFF9FFF8E, // ��	30DD
	0x0000FF8F, // �	30DE
	0x0000FF90, // �	30DF
	0x0000FF91, // �	30E0
	0x0000FF92, // �	30E1
	0x0000FF93, // �	30E2
	0x0000FF6C, // �	30E3
	0x0000FF94, // �	30E4
	0x0000FF6D, // �	30E5
	0x0000FF95, // �	30E6
	0x0000FF6E, // �	30E7
	0x0000FF96, // �	30E8
	0x0000FF97, // �	30E9
	0x0000FF98, // �	30EA
	0x0000FF99, // �	30EB
	0x0000FF9A, // �	30EC
	0x0000FF9B, // �	30ED
	0x0000FF9C, // �	30EE	NOTE: No half-width small wa? This uses the half-width wa.
	0x0000FF9C, // �	30EF
	0x00000000, // .	30F0	NOTE: No half-width version (uncommon; wi)
	0x00000000, // .	30F1	NOTE: No half-width version (uncommon; we)
	0x0000FF66, // �	30F2
	0x0000FF9D, // �	30F3
	0x0000FF73, // ��	30F4	NOTE: vu = u + diacritic
	0x00000000, // .	30F5	NOTE: No half-width version (small ka)
	0x00000000, // .	30F6	NOTE: No half-width version (small ke)
	0xFF9EFF73, // ��	30F7	NOTE: va = wa + diacritic
	0x00000000, // .	30F8	NOTE: No half-width version (vi = wi + diacritic; no wi)
	0x00000000, // .	30F9	NOTE: No half-width version (ve = we + diacritic; no we)
	0xFF9EFF66, // ��	30FA	NOTE: vo = wo + diacritic
	0x0000FF65  // �	30FB	NOTE: This is a middle dot (half-width) not a period
};

static Ax::uint32 HalfwidthKatakanaToFullwidthKatakana( Ax::uint32 UTF32Char, Ax::uint32 *pDstMark = NULL )
{
	if( UTF32Char >= 0x30A1 && UTF32Char <= 0x30FB ) {
		const Ax::uint32 Data = g_KatakanaConversions[ UTF32Char - 0x30A1 ];

		if( pDstMark != NULL ) {
			*pDstMark = ( Data & 0xFFFF0000 ) >> 16;
		}
		
		return Data & 0x0000FFFF;
	}

	return UTF32Char;
}
static Ax::uint32 FullwidthKatakanaToHalfwidthKatakana( Ax::uint32 UTF32Char, Ax::uint32 UTF32DiacriticMark = 0 )
{
	if( UTF32Char < 0x0000FF65 || UTF32Char > 0x0000FF9D ) {
		return UTF32Char;
	}

	const Ax::uint32 TestValue = UTF32Char | ( UTF32DiacriticMark << 16 );
	for( Ax::uint32 i = 0; i < sizeof( g_KatakanaConversions )/sizeof( g_KatakanaConversions[ 0 ] ); ++i ) {
		if( g_KatakanaConversions[ i ] == TestValue ) {
			return i + 0x30A1;
		}
	}

	return UTF32Char;
}

static Ax::uint32 FullwidthToHalfwidth( Ax::uint32 UTF32Char )
{
	if( UTF32Char >= 0xFF01 && UTF32Char <= 0xFF5E ) {
		return UTF32Char - 0xFF01 + 0x0021;
	}

	return FullwidthKatakanaToHalfwidthKatakana( UTF32Char );
}
static Ax::uint32 HalfwidthToFullwidth( Ax::uint32 UTF32Char )
{
	if( UTF32Char >= 0x0021 && UTF32Char <= 0x007E ) {
		return UTF32Char - 0x0021 + 0xFF01;
	}

	return HalfwidthKatakanaToFullwidthKatakana( UTF32Char );
}

Ax::uint32 Ax::String::HanToZen( uint32 uCodePoint )
{
	return HalfwidthToFullwidth( uCodePoint );
}
Ax::uint32 Ax::String::ZenToHan( uint32 uCodePoint )
{
	return FullwidthToHalfwidth( uCodePoint );
}

Ax::String Ax::String::HanToZen() const
{
	String Result;
	if( !Result.Reserve( m_cStr + 1 ) ) {
		return String();
	}

	for( uintptr i = 0; i < m_cStr; ++i ) {
		uintptr n = 1;
		Result.AppendUTF32Char( HanToZen( CodePointFromOffset( i, &n ) ) );
		i += ( n - 1 );
	}

	return Result;
}
Ax::String Ax::String::ZenToHan() const
{
	String Result;
	if( !Result.Reserve( m_cStr + 1 ) ) {
		return String();
	}

	for( uintptr i = 0; i < m_cStr; ++i ) {
		uintptr n = 1;
		Result.AppendUTF32Char( ZenToHan( CodePointFromOffset( i, &n ) ) );
		i += ( n - 1 );
	}

	return Result;
}

static void SkipWhitespace( const char *&p )
{
	while( *( const unsigned char * )p <= ' ' && *p != '\0' ) {
		++p;
	}
}
int Ax::String::DecodeRadix( const char *&p, int radix )
{
	if( radix == kRadix_CStyle ) {
		radix = 10;

		if( *p == '0' ) {
			const char ch = *( p + 1 );
			if( ch == 'x' || ch == 'X' ) {
				radix = 16;
				p += 2;
			} else if( ch == 'b' || ch == 'B' ) {
				radix = 2;
				p += 2;
			} else if( ch != '.' ) {
				radix = 8;
				++p;
			}
		}
	} else if( radix == kRadix_BasicStyle ) {
		radix = 10;

		if( *p == '%' ) {
			radix = 2;
			++p;
		} else if( *p == '$' ) {
			radix = 16;
			++p;
		} else if( *p == '0' ) {
			const char ch = *( p + 1 );
			if( ch == 'b' || ch == 'B' ) {
				radix = 2;
				p += 2;
			} else if( ch == 'c' || ch == 'C' ) {
				radix = 8;
				p += 2;
			} else if( ch == 'd' || ch == 'D' ) {
				radix = 10;
				p += 2;
			} else if( ch == 'h' || ch == 'H' || ch == 'x' || ch == 'X' ) {
				radix = 16;
				p += 2;
			}
		} else {
			int testradix = 0;
			const char *check = p;
			while( *check >= '0' && *check <= '9' ) {
				testradix *= 10;
				testradix += +( *check - '0' );
				++check;
			}
			if( ( *check == 'x' || *check == 'X' ) && testradix >= 2 && testradix <= 36 ) {
				radix = testradix;
				p = check + 1;
			}
		}
	}

	return radix;
}
static int ReadSignPart( const char *&p )
{
	if( *p == '-' ) {
		++p;
		return -1;
	} else if( *p == '+' ) {
		++p;
	}

	return 1;
}
static Ax::uint64 ReadUIntPart( const char *&p, unsigned int radix, char chDigitSep )
{
	Ax::uint64 n = 0;

	while( *p != '\0' ) {
		const int digit = Ax::String::GetDigit( *p == chDigitSep && *p != '\0' ? *( p + 1 ) : *p, radix );
		if( digit == -1 ) {
			break;
		}

		if( *p == chDigitSep ) {
			++p;
		}
		++p;

		n *= radix;
		n += digit;
	}

	return n;
}

Ax::uint64 Ax::String::ToUnsignedInteger( const char *p, int radix, char chDigitSep )
{
	if( !p ) {
		return 0;
	}

	SkipWhitespace( p );
	radix = DecodeRadix( p, radix );

	return ReadUIntPart( p, radix, chDigitSep );
}
Ax::int64 Ax::String::ToInteger( const char *p, int radix, char chDigitSep )
{
	if( !p ) {
		return 0;
	}

	SkipWhitespace( p );
	const int m = ReadSignPart( p );

	return int64( ToUnsignedInteger( p, radix, chDigitSep ) )*m;
}
double Ax::String::ToFloat( const char *p, int radix, char chDigitSep )
{
	if( !p ) {
		return 0;
	}

	SkipWhitespace( p );
	radix = DecodeRadix( p, radix );

	const int m = ReadSignPart( p );

	SkipWhitespace( p );

	if( *p == '.' && GetDigit( *( p + 1 ), radix ) == -1 ) {
		return 0;
	}

	const uint64 whole = ReadUIntPart( p, radix, chDigitSep );
	if( *p == '.' ) {
		++p;
	}
	const uint64 fract = ReadUIntPart( p, radix, chDigitSep );

	int esign = 1;
	uint64 e = 0;
	if( ( radix <= 10 && ( *p == 'e' || *p == 'E' ) ) || ( radix <= 26 && ( *p == 'p' || *p == 'P' ) ) ) {
		++p;
		if( *p == '-' ) {
			++p;
			esign = -1;
		} else if( *p == '+' ) {
			++p;
		}

		e = ReadUIntPart( p, radix, chDigitSep );
	}

	typedef long double BigFloat;

	BigFloat f = BigFloat( whole );

	uint64 fractmag = 1;
	uint64 tmpfract = fract;
	while( tmpfract > 0 ) {
		fractmag *= radix;
		tmpfract /= radix;
	}

	f += BigFloat( fract )/BigFloat( fractmag );
	while( e > 0 ) {
		if( esign > 0 ) {
			f *= 10;
		} else {
			f /= 10;
		}

		--e;
	}

	return f;
}

static const char *const g_pszUpperDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char *const g_pszLowerDigits = "0123456789abcdefghijklmnopqrstuvwxyz";

Ax::String Ax::String::FromUnsignedInteger( uint64 value, int radix, uintptr numDigitsForSep, char chDigitSep )
{
	static char buf[ 256 ];

	if( value == 0 ) {
		return Ax::String( "0" );
	}
	
	if( radix < 2 || radix > 36 ) {
		return Ax::String();
	}
	const uint64 r = uint64( radix );

	uintptr remainingDigitsForSep = numDigitsForSep > 0 ? numDigitsForSep : ~uintptr( 0 );
	char *p = &buf[ sizeof( buf ) - 1 ];
	*p = '\0';

	uint64 n = value;
	while( n > 0 ) {
		if( remainingDigitsForSep-- == 0 ) {
			remainingDigitsForSep = numDigitsForSep;
			*--p = chDigitSep;
		}

		*--p = g_pszUpperDigits[ n%r ];
		n /= r;
	}

	return Ax::String( p );
}
Ax::String Ax::String::FromInteger( int64 value, int radix, uintptr numDigitsForSep, char chDigitSep )
{
	if( value < 0 ) {
		return Ax::String( "-" ) + FromUnsignedInteger( uint64( value ), radix, numDigitsForSep, chDigitSep );
	}

	return FromUnsignedInteger( uint64( value ), radix, numDigitsForSep, chDigitSep );
}
Ax::String Ax::String::FromFloat( double value, int radix, uintptr maxTrailingDigits, uintptr numDigitsForSep, char chDigitSep )
{
	if( radix < 2 || radix > 36 ) {
		return Ax::String();
	}
	const uint64 r = uint64( radix );

	const intptr maxDigits = maxTrailingDigits > 1024 ? 1024 : intptr( maxTrailingDigits );

	const int64 whole = int64( value );

	double f = value - whole;
	uint64 fract = 0;
	while( f > 0.0 ) {
		fract *= r;

		f *= r;
		fract += uint64( f*r );

		f = f - uint64( f );
	}

	Ax::String Result = FromInteger( whole, radix, numDigitsForSep, chDigitSep );
	Result.Append( FromUnsignedInteger( fract, radix, ~uintptr( 0 ), '\0' ).Left( maxDigits ) );

	return Result;
}

void Ax::String::GetOffsetAndSize( const char *p, uintptr &dstOffset, uintptr &dstCount, intptr first, intptr count )
{
	const uintptr n = p != NULL ? strlen( p ) : 0;
	if( !n ) {
		dstOffset = 0;
		dstCount = 0;

		return;
	}

	dstOffset = first < 0 ? n + first : first;
	dstCount = count < 0 ? n + 1 + count : count;
	if( dstOffset > n ) { dstOffset = n; }
	if( dstCount + dstOffset > n ) { dstCount = n - dstOffset; }
}
void Ax::String::GetOffsetAndSize( uintptr &dstOffset, uintptr &dstCount, intptr first, intptr count ) const
{
	const uintptr n = m_cStr;
	if( !n ) {
		dstOffset = 0;
		dstCount = 0;

		return;
	}

	dstOffset = first < 0 ? n + first : first;
	dstCount = count < 0 ? n + 1 + count : count;
	if( dstOffset > n ) { dstOffset = n; }
	if( dstOffset + dstCount > n ) { dstCount = n - dstOffset; }
}

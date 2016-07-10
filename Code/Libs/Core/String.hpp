#pragma once

#ifdef _WIN32
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif

#ifndef PATH_MAX
# ifdef MAX_PATH
#  define PATH_MAX MAX_PATH
# elif defined( _MAX_PATH )
#  define PATH_MAX _MAX_PATH
# else
#  define PATH_MAX 256
# endif
#endif
#ifndef MAX_PATH
# define MAX_PATH PATH_MAX
#endif

#ifndef AX_DIRSEP
# if defined( _WIN32 )
#  define AX_DIRSEP "\\"
# else
#  define AX_DIRSEP "/"
# endif
#endif

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "Types.hpp"
#include "../Platform/Platform.hpp"
#include "../Allocation/Allocator.hpp"
#include "../Collections/Array.hpp"


/* TODO: Switch over to the ax_string library -- this uses pieces of it */

#define AXSTR_FUNC
#define AXSTR_CALL

/*
 *	Encoding of the string.
 *
 *	Strings are internally encoded with UTF-8, always. There are routines to
 *	convert to an alternate Unicode encoding, however.
 */
typedef enum
{
	axstr_enc_unknown,

	axstr_enc_utf8,
	axstr_enc_utf16_le,
	axstr_enc_utf16_be,
	axstr_enc_utf32_le,
	axstr_enc_utf32_be
} axstr_encoding_t;

/*
 *	Byte-Order-Mark mode for Unicode data.
 *
 *	Indicates whether a byte-order-mark (BOM) should be (or is) present.
 */
typedef enum
{
	axstr_bom_disabled,
	axstr_bom_enabled
} axstr_byteordermark_t;

#define axstr_len(Ptr_) ((axstr_size_t)(strlen((Ptr_))))

typedef bool axstr_bool_t;
typedef Ax::uint8 axstr_utf8_t;
typedef Ax::uint16 axstr_utf16_t;
typedef Ax::uint32 axstr_utf32_t;
typedef Ax::uintptr axstr_size_t;
typedef Ax::intptr axstr_ptrdiff_t;

#define AXSTR_UNKNOWN_LENGTH ( ~( axstr_size_t )0 )

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
AXSTR_FUNC axstr_utf32_t AXSTR_CALL axstr_step_utf8_decode( const axstr_utf8_t **ppUTF8Src, const axstr_utf8_t *pUTF8SrcEnd );
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
AXSTR_FUNC axstr_utf32_t AXSTR_CALL axstr_step_utf16_decode( const axstr_utf16_t **ppUTF16Src, const axstr_utf16_t *pUTF16SrcEnd );

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
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_step_utf8_encode( axstr_utf8_t **ppUTF8Dst, axstr_utf8_t *pUTF8DstEnd, axstr_utf32_t uCodepoint );
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
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_step_utf16_encode( axstr_utf16_t **ppUTF16Dst, axstr_utf16_t *pUTF16DstEnd, axstr_utf32_t uCodepoint );

/*
 *	Convert a chunk of UTF-8 data to UTF-16 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf16( axstr_utf16_t *pUTF16Dst, axstr_size_t cMaxDstUTF16Chars, const axstr_utf8_t *pUTF8Src );
/*
 *	Convert a chunk of UTF-8 data to UTF-16 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf16_n( axstr_utf16_t *pUTF16Dst, axstr_size_t cMaxDstUTF16Chars, const axstr_utf8_t *pUTF8Src, const axstr_utf8_t *pUTF8SrcEnd );
/*
 *	Convert a chunk of UTF-8 data to UTF-32 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf32( axstr_utf32_t *pUTF32Dst, axstr_size_t cMaxDstUTF32Chars, const axstr_utf8_t *pUTF8Src );
/*
 *	Convert a chunk of UTF-8 data to UTF-32 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf8_to_utf32_n( axstr_utf32_t *pUTF32Dst, axstr_size_t cMaxDstUTF32Chars, const axstr_utf8_t *pUTF8Src, const axstr_utf8_t *pUTF8SrcEnd );
/*
 *	Convert a chunk of UTF-16 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf16_to_utf8( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf16_t *pUTF16Src );
/*
 *	Convert a chunk of UTF-16 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf16_to_utf8_n( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf16_t *pUTF16Src, const axstr_utf16_t *pUTF16SrcEnd );
/*
 *	Convert a chunk of UTF-32 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf32_to_utf8( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf32_t *pUTF32Src );
/*
 *	Convert a chunk of UTF-32 data to UTF-8 data. (Assumes byte-order of machine.)
 */
AXSTR_FUNC axstr_bool_t AXSTR_CALL axstr_utf32_to_utf8_n( axstr_utf8_t *pUTF8Dst, axstr_size_t cMaxDstUTF8Bytes, const axstr_utf32_t *pUTF32Src, const axstr_utf32_t *pUTF32SrcEnd );
/*
 *	Write a byte-order-mark (BOM) to a buffer
 *
 *	If bom is set to axstr_bom_enabled then the appropriate byte-order-mark for
 *	the encoding `enc` is used.
 *
 *	return: Number of bytes written to pDstBuf. (Will not exceed cDstBytes.)
 */
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_write_bom( void *pDstBuf, axstr_size_t cDstBytes, axstr_encoding_t enc, axstr_byteordermark_t bom );

/*
 *	Convert a UTF-8 stream to an arbitrary binary encoding with an optional
 *	byte-order-mark. Does not explicitly write a null terminator. (Useful when
 *	writing to a (potentially memory mapped) file.)
 *
 *	This returns the number of bytes written.
 */
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_to_encoding_n( void *pDstBuf, axstr_size_t cDstBytes, const char *pszSrc, axstr_size_t cSrcBytes, axstr_encoding_t enc, axstr_byteordermark_t bom );
AXSTR_FUNC axstr_size_t AXSTR_CALL axstr_to_encoding( void *pDstBuf, axstr_size_t cDstBytes, const char *pszSrc, axstr_encoding_t enc, axstr_byteordermark_t bom );

namespace Ax
{

	enum ERadix
	{
		// 0b<n> = bin, 0<n> = oct, 0x<n> = hex; else dec
		kRadix_CStyle = -1,
		// %<n>, 0b<n> = bin, 0c<n> = oct, 0x<n> = hex, <r>x<n> = <r>; else dec
		kRadix_BasicStyle = -2,

		kRadix_Binary = 2,
		kRadix_Octal = 8,
		kRadix_Decimal = 10,
		kRadix_Hexadecimal = 16
	};

	enum class EEncoding
	{
		Unknown,

		UTF8,
		UTF16_LE,
		UTF16_BE,
		UTF32_LE,
		UTF32_BE
	};
	enum class EByteOrderMark
	{
		Disabled,
		Enabled
	};

	EEncoding DetectEncoding( const uint8( &buf )[ 4 ], uintptr *pOutLength = nullptr );

	bool CaseCompare( const char *first, const char *second, intptr length = -1 );
	bool Compare( const char *first, const char *second, intptr length = -1 );

	inline bool CaseCmp( const char *first, const char *second, intptr length = -1 ) { return CaseCompare( first, second, length ); }
	inline bool Cmp( const char *first, const char *second, intptr length = -1 ) { return Compare( first, second, length ); }
	
	inline bool Compare( ECase Casing, const char *first, const char *second, intptr length = -1 )
	{
		switch( Casing ) {
		case ECase::Sensitive:
			return Compare( first, second, length );

		case ECase::Insensitive:
			return CaseCompare( first, second, length );
		}

		return false;
	}
	inline bool Cmp( ECase Casing, const char *first, const char *second, intptr length = -1 )
	{
		switch( Casing ) {
		case ECase::Sensitive:
			return Compare( first, second, length );

		case ECase::Insensitive:
			return CaseCompare( first, second, length );
		}

		return false;
	}

	char *StrDup( char *&dst, const char *src, int memtag = AX_DEFAULT_MEMTAG );
	char *StrDupN( char *&dst, const char *src, uintptr srcn, int memtag = AX_DEFAULT_MEMTAG );

	inline char *StrDup( const char *src, int memtag = AX_DEFAULT_MEMTAG ) { char *dst = nullptr; return StrDup( dst, src, memtag ); }
	inline char *StrDupN( const char *src, uintptr srcn, int memtag = AX_DEFAULT_MEMTAG ) { char *dst = nullptr; return StrDupN( dst, src, srcn, memtag ); }

	const char *StrCpy( char *dst, uintptr dstn, const char *src );
	const char *StrCpyN( char *dst, uintptr dstn, const char *src, uintptr srcn );
	void AppendString( char *dst, uintptr dstn, uintptr *index, const char *src, uintptr srcn );
	void AppendString( char *dst, uintptr dstn, uintptr *index, const char *src );
	bool CmpPathChar( char a, char b );
	bool IsPathSep( char a );

	int FormatV( char *dst, uintptr dstn, const char *fmt, va_list args );
	inline int Format( char *dst, uintptr dstn, const char *fmt, ... )
	{
		va_list args;

		va_start( args, fmt );
		const int r = FormatV( dst, dstn, fmt, args );
		va_end( args );
		
		return r;
	}

	inline void StrCat( char *dst, uintptr dstn, const char *src )
	{
		uintptr Index = strlen( dst );
		AppendString( dst, dstn, &Index, src );
	}

	template< uintptr tDstSize >
	inline const char *StrCpy( char( &dst )[ tDstSize ], const char *src )
	{
		return StrCpy( dst, tDstSize, src );
	}
	template< uintptr tDstSize >
	inline const char *StrCpyN( char( &dst )[ tDstSize ], const char *src, uintptr srcn )
	{
		return StrCpyN( dst, tDstSize, src, srcn );
	}
	template< uintptr tDstSize >
	inline void StrCat( char( &dst )[ tDstSize ], const char *src )
	{
		uintptr Index = strlen( dst );
		AppendString( dst, tDstSize, &Index, src );
	}
	template< uintptr tDstSize >
	inline void AppendString( char( &dst )[ tDstSize ], uintptr *index, const char *src )
	{
		AppendString( dst, tDstSize, index, src );
	}
	template< uintptr tDstSize >
	inline void AppendString( char( &dst )[ tDstSize ], uintptr &index, const char *src )
	{
		AppendString( dst, tDstSize, &index, src );
	}
	template< uintptr tDstSize >
	inline void AppendString( char( &dst )[ tDstSize ], uintptr *index, const char *src, uintptr srcn )
	{
		AppendString( dst, tDstSize, index, src, srcn );
	}
	template< uintptr tDstSize >
	inline void AppendString( char( &dst )[ tDstSize ], uintptr &index, const char *src, uintptr srcn )
	{
		AppendString( dst, tDstSize, &index, src, srcn );
	}

	template< uintptr tDstSize >
	inline int FormatV( char( &dst )[ tDstSize ], const char *fmt, va_list args )
	{
		return FormatV( dst, tDstSize, fmt, args );
	}
	template< uintptr tDstSize >
	inline int Format( char( &dst )[ tDstSize ], const char *fmt, ... )
	{
		va_list args;
		va_start( args, fmt );
		const int r = FormatV( dst, tDstSize, fmt, args );
		va_end( args );
		return r;
	}

	// retrieve the relative path from one file to another
	//
	// dst: destination buffer
	// dstn: size of destination buffer in bytes
	// srcfrom: the file with the base path
	// srcto: the file you are getting to from "srcfrom"
	//
	// in the event of truncation a terminating NULL character will be placed as
	// the last byte within the buffer (within the bounds dictated by dstn).
	//
	// return: dst if inputs are good
	const char *GetRelativePath( char *dst, uintptr dstn, const char *srcfrom, const char *srcto );
	const char *GetAbsolutePath( char *dst, uintptr dstn, const char *relpath );

	template< uintptr tDstSize >
	inline const char *GetRelativePath( char( &dst )[ tDstSize ], const char *srcfrom, const char *srcto )
	{
		return GetRelativePath( dst, tDstSize, srcfrom, srcto );
	}
	template< uintptr tDstSize >
	inline const char *GetAbsolutePath( char( &dst )[ tDstSize ], const char *relpath )
	{
		return GetAbsolutePath( dst, tDstSize, relpath );
	}

	//	Search for a string of a specific length
	const char *StrNStr( const char *a, const char *b, uintptr n );
	//	Search for the longest match of a string
	const char *FindLongestMatch( const char *a, const char *b, uintptr alen, uintptr *matchLen );

	//	Search for the longest common contiguous text chunk in a set of strings
	//
	//	e.g., "CEntity," "IEntity," "CEntity_Pivot" should yield "Entity"
	//	Note that "CEntity_Local," "CEntity_Vehicle," "CEntity_Player" would
	//	yield "CEntity_" as the longest match.
	bool FindCommonText( uintptr nbuff, char *buff, uintptr ntexts, const char *const *texts );

	template< uintptr tBuffSize >
	inline bool FindCommonText( char( &buff )[ tBuffSize ], uintptr ntexts, const char *const *texts )
	{
		return FindCommonText( tBuffSize, buff, ntexts, texts );
	}
	template< uintptr tBuffSize, uintptr TTextCount >
	inline bool FindCommonText( char( &buff )[ tBuffSize ], const char *( &texts )[ TTextCount ] )
	{
		return FindCommonText( tBuffSize, buff, TTextCount, texts );
	}

	bool CopyTextToClipboard( const char *pszText, uintptr cText = ~uintptr( 0 ) );
	bool PasteTextFromClipboard( char *pszOutText, uintptr cMaxOutText );

	template< uintptr tSize >
	AX_FORCEINLINE bool PasteTextFromClipboard( char ( &szOutText )[ tSize ] ) {
		return PasteTextFromClipboard( szOutText, tSize );
	}

	enum class EKeepQuotes
	{
		No,
		Yes
	};

	enum class EKeepEmpty
	{
		No,
		Yes
	};

	class String
	{
	public:
		String( const SMemtag &memtag = SMemtag() );
		String( const String &x, const SMemtag &memtag = SMemtag() );
		String( const String &x, intptr first, intptr count = -1, const SMemtag &memtag = SMemtag() );
		String( const char *p, intptr count = -1, const SMemtag &memtag = SMemtag() );
		String( const char *p, intptr first, intptr count, const SMemtag &memtag = SMemtag() );
		String( const char *s, const char *e, const SMemtag &memtag = SMemtag() );
		String( const wchar_t *p, intptr count = -1, const SMemtag &memtag = SMemtag() );
		String( const wchar_t *p, intptr first, intptr count, const SMemtag &memtag = SMemtag() );
		String( const wchar_t *s, const wchar_t *e, const SMemtag &memtag = SMemtag() );
		String( char ch, const SMemtag &memtag = SMemtag() );
		String( wchar_t ch, const SMemtag &memtag = SMemtag() );
		String( bool b, const SMemtag &memtag = SMemtag() );
		String( int i, const SMemtag &memtag = SMemtag() );
		String( float f, int decimalPlaces = 0, const SMemtag &memtag = SMemtag() );
		String( void *p, const SMemtag &memtag = SMemtag() );
#if ( AX_ARCH & AX_ARCH_64 ) && defined( _MSC_VER ) && _MSC_VER >= 1800
		inline String( const String &x, int first, int count = -1, const SMemtag &memtag = SMemtag() )
		: String( x, ( intptr )first, ( intptr )count, memtag )
		{
		}
		inline String( const char *p, int count, const SMemtag &memtag = SMemtag() )
		: String( p, ( intptr )count, memtag )
		{
		}
		inline String( const char *p, int first, int count, const SMemtag &memtag = SMemtag() )
		: String( p, ( intptr )first, ( intptr )count, memtag )
		{
		}
		inline String( const wchar_t *p, int count, const SMemtag &memtag = SMemtag() )
		: String( p, ( intptr )count, memtag )
		{
		}
		inline String( const wchar_t *p, int first, int count, const SMemtag &memtag = SMemtag() )
		: String( p, ( intptr )first, ( intptr )count, memtag )
		{
		}
#endif
#if AX_CXX_N2118
		inline String( String &&x )
		: m_cStr( x.m_cStr )
		, m_cMax( x.m_cMax )
		, m_pStr( x.m_pStr )
		, m_iTag( x.m_iTag )
		{
			x.m_cStr = 0;
			x.m_cMax = 0;
			x.m_pStr = nullptr;
		}
#endif

		inline String Copy() const { return String( *this ); }

		String &Swap( String &x );

		bool Reserve( uintptr numChars );
		bool SetCapacity( uintptr numBytes );

		inline void CheckLength() { m_cStr = m_pStr != NULL ? ( int )strlen( m_pStr ) : 0; }

		bool Assign( const char *p, intptr first, intptr count );
		inline bool Assign( const char *p, intptr count = -1 ) { return Assign( p, 0, count ); }
		inline bool Assign( const String &x, intptr first = 0, intptr count = -1 ) { return Assign( ( const char * )x, first, count ); }
		inline bool Assign( char ch ) { return Assign( &ch, 1 ); }
		inline bool Assign( const char *s, const char *e ) { return Assign( s, 0, intptr( e - s ) ); }

		bool Append( const char *p, intptr first, intptr count );
		inline bool Append( const char *p, intptr count = -1 ) { return Append( p, 0, count ); }
		inline bool Append( const String &x, intptr first = 0, intptr count = -1 ) { return Append( ( const char * )x, first, count ); }
		inline bool Append( char ch ) { return Append( &ch, 1 ); }
		inline bool Append( const char *s, const char *e ) { return Append( s, 0, intptr( e - s ) ); }

		bool Prepend( const char *p, intptr first, intptr count );
		inline bool Prepend( const char *p, intptr count = -1 ) { return Prepend( p, 0, count ); }
		inline bool Prepend( const String &x, intptr first = 0, intptr count = -1 ) { return Prepend( ( const char * )x, first, count ); }
		inline bool Prepend( char ch ) { return Prepend( &ch, 1 ); }
		inline bool Prepend( const char *s, const char *e ) { return Prepend( s, 0, intptr( e - s ) ); }

		bool Insert( const char *p, intptr before, intptr first, intptr count );
		inline bool Insert( const char *p, intptr before, intptr count = -1 ) { return Insert( p, before, 0, count ); }
		inline bool Insert( const String &x, intptr before, intptr first = 0, intptr count = -1 ) { return Insert( ( const char * )x, before, first, count ); }
		inline bool Insert( char ch, intptr before ) { return Insert( &ch, before, 0, 1 ); }
		inline bool Insert( const char *s, const char *e, intptr before ) { return Insert( s, before, 0, intptr( e - s ) ); }

		intptr Find( char c, uintptr startIndex = 0 ) const;
		intptr Find( const char *s, uintptr startIndex = 0 ) const;
		intptr FindLast( char c ) const;
		intptr FindLast( const char *s ) const;

		void Remove( intptr first, intptr count );
		inline bool Remove( const char *search ) { return Replace( search, "" ); }
		bool Replace( const char *search, const char *substitution );
		void Replace( uintptr start, uintptr end, char value );
		inline String &BackSlashesToForwardSlashes() { ( void )Replace( "\\", "/" ); return *this; }
		inline String &ForwardSlashesToBackSlashes() { ( void )Replace( "/", "\\" ); return *this; }
		inline String &CleanPathSlashes()
		{
#ifdef _WIN32
			ForwardSlashesToBackSlashes();
#endif

			while( Replace( AX_DIRSEP AX_DIRSEP, AX_DIRSEP ) ) {
			}

			return *this;
		}
		inline String Removed( intptr first, intptr count ) const { String temp = *this; temp.Remove( first, count ); return temp; }
		inline String Removed( const char *search ) const { String temp = *this; temp.Remove( search ); return temp; }
		inline String Replaced( const char *search, const char *substitution ) const { String temp = *this; temp.Replace( search, substitution ); return temp; }

		bool AppendFormatV( const char *fmt, va_list args );
		inline bool AppendFormat( const char *fmt, ... ) { va_list args; va_start( args, fmt ); const bool r = AppendFormatV( fmt, args ); va_end( args ); return r; }

		inline bool FormatV( const char *fmt, va_list args ) { Empty(); return AppendFormatV( fmt, args ); }
		inline bool Format( const char *fmt, ... ) { va_list args; va_start( args, fmt ); const bool r = FormatV( fmt, args ); va_end( args ); return r; }

		static inline String FormattedV( const char *fmt, va_list args ) { String r; r.AppendFormatV( fmt, args ); return r; }
		static inline String Formatted( const char *fmt, ... ) { va_list args; va_start( args, fmt ); String r = FormattedV( fmt, args ); va_end( args ); return r; }

		inline bool PrependFormatV( const char *fmt, va_list args ) { String x; x.AppendFormatV( fmt, args ); x.Append( *this ); return Assign( x ); }
		inline bool PrependFormat( const char *fmt, ... ) { va_list args; va_start( args, fmt ); const bool r = PrependFormatV( fmt, args ); va_end( args ); return r; }

		String &ToLower();
		String &ToUpper();
		inline String Lower() const { return Copy().ToLower(); }
		inline String Upper() const { return Copy().ToUpper(); }

		intptr FindExtension() const;

		String ExtractExtension() const;
		String ExtractFilename() const;
		String ExtractDirectory() const;
		String ExtractBasename() const;

		bool ReplaceExtension( const char *pszNewExt );

		int LexCaseCompare( const char *other, intptr first, intptr count ) const;
		int LexCompare( const char *other, intptr first, intptr count ) const;

		int LexCaseCompare( const char *other ) const;
		int LexCompare( const char *other ) const;

		inline bool CaseCompare( const char *other, intptr first, intptr count ) const { return LexCaseCompare( other, first, count ) == 0; }
		inline bool Compare( const char *other, intptr first, intptr count ) const { return LexCompare( other, first, count ) == 0; }

		inline bool CaseCompare( const char *other ) const { return LexCaseCompare( other ) == 0; }
		inline bool Compare( const char *other ) const { return LexCompare( other ) == 0; }

		inline bool CaseCmp( const char *other, intptr first, intptr count ) const { return CaseCompare( other, first, count ); }
		inline bool Cmp( const char *other, intptr first, intptr count ) const { return Compare( other, first, count ); }

		inline bool CaseCmp( const char *other ) const { return CaseCompare( other ); }
		inline bool Cmp( const char *other ) const { return Compare( other ); }

		inline bool StartsWith( const char *other ) const { return other != NULL ? Compare( other, 0, ( int )strlen( other ) ) : false; }
		inline bool EndsWith( const char *other ) const { if( !other ) { return false; } int len = ( int )strlen( other ); return Compare( other, -len, len ); }
		inline bool Contains( const char *other ) const { return Find( other ) != -1; }

		inline bool StartsWith( const String &other ) const { return Compare( other, 0, ( intptr )other.Len() ); }
		inline bool EndsWith( const String &other ) const { return Compare( other, -( intptr )other.Len(), other.Len() ); }

		inline bool CaseStartsWith( const char *other ) const { return other != NULL ? CaseCompare( other, 0, ( int )strlen( other ) ) : false; }
		inline bool CaseEndsWith( const char *other ) const { if( !other ) { return false; } int len = ( int )strlen( other ); return CaseCompare( other, -len, len ); }
		inline bool CaseStartsWith( const String &other ) const { return CaseCompare( other, 0, ( intptr )other.Len() ); }
		inline bool CaseEndsWith( const String &other ) const { return CaseCompare( other, -( intptr )other.Len(), other.Len() ); }
		
		inline bool AppendDirectorySeparator() { if( !IsEmpty() && !EndsWith( "/" ) && !EndsWith( "\\" ) ) { return Append( AX_DIRSEP ); } return true; }
		inline bool AppendPath( const char *other ) { if( !AppendDirectorySeparator() ) { return false; } return Append( other ); }

		inline String &operator/=( const char *other ) { AppendPath( other ); return *this; }
		inline String operator/( const char *other ) const { String temp( *this ); temp.AppendPath( other ); return temp; }

		String Escape() const;
		String Unescape() const;

		String Quote() const;
		String Unquote() const;

		bool TabSelf();
		void UntabSelf();

		inline String Tab() const { String Temp = String( *this ); if( !Temp.TabSelf() ) { return String(); } return Temp; }
		inline String Untab() const { String Temp = String( *this ); Temp.UntabSelf(); return Temp; }
		
		// Splits the string into an array based on the given separator
		//
		// e.g., "apples;oranges;bananas" split by ";" would yield an array of
		//       three elements.
		//
		//         [0] = "apples"
		//         [1] = "oranges"
		//         [2] = "bananas"
		//
		// The KeepEmpty parameter controls whether empty strings will be added
		// to the array. Empty strings are not added by default (EKeepEmpty::No)
		bool Split( Ax::TArray< String > &OutArray, const char *separator, EKeepEmpty KeepEmpty = EKeepEmpty::No ) const;

		// Splits the string into an array based on the given separator
		//
		// e.g., "apples;oranges;bananas" split by ";" would yield an array of
		//       three elements.
		//
		//         [0] = "apples"
		//         [1] = "oranges"
		//         [2] = "bananas"
		//
		// The KeepEmpty parameter controls whether empty strings will be added
		// to the array. Empty strings are not added by default (EKeepEmpty::No)
		TArray< String > Split( const char *separator, EKeepEmpty KeepEmpty = EKeepEmpty::No ) const;
		// Splits the string into an array based on the given separator except
		// in quoted strings. A given escape character can be used to respect
		// escape sequences in the string. If the escape character precedes the
		// separator or a quote then no special processing is done for either.
		//
		// The returned array will keep all escape characters, without any
		// special processing.
		TArray< String > SplitUnquoted( const char *separator, char chEscape = '\0', EKeepQuotes KeepQuotes = EKeepQuotes::Yes ) const;
		inline TArray< String > SplitUnquoted( const char *separator, EKeepQuotes KeepQuotes, char chEscape = '\0' ) const { return SplitUnquoted( separator, chEscape, KeepQuotes ); }
		String &Merge( const TArray< String > &arr, const char *glue = NULL );

		inline uintptr Len() const { return m_cStr; }
		inline uintptr Num() const { return m_pStr != NULL ? m_cStr + 1 : 0; }
		inline uintptr NumAllocated() const { return m_cMax; }

		inline bool IsEmpty() const { return m_cStr == 0; }
		inline bool operator!() const { return IsEmpty(); }

		inline void Empty() { if( m_pStr != NULL ) { *m_pStr = '\0'; m_cStr = 0; } }
		inline void Clear() { Empty(); }

		inline String Left( intptr count ) const { return String( *this, ( intptr )0, count ); }
		inline String Right( intptr count ) const { return String( *this, -count, count ); }
		inline String Mid( intptr offset, intptr count = 1 ) const { return String( *this, offset, count ); }

		inline String Substring( intptr start, intptr end ) const
		{
			const uintptr startIndex = start < 0 ? Len() + start : start;
			const uintptr endIndex = end < 0 ? Len() + 1 + end : end;
			
			return String( *this, startIndex, ( intptr )( endIndex - startIndex ) );
		}
		inline String Extract( intptr start, intptr end )
		{
			const uintptr startIndex = start < 0 ? Len() + start : start;
			const uintptr endIndex = end < 0 ? Len() + 1 + end : end;

			const String result = String( *this, startIndex, ( intptr )( endIndex - startIndex ) );
			Remove( startIndex, endIndex );

			return result;
		}

		inline String &TrimLeft() { uintptr i = 0; while( i < m_cStr && m_pStr[ i ] <= ' ' ) { ++i; } if( i > 0 ) { Remove( 0, i ); } return *this; }
		inline String &TrimRight() { uintptr i = m_cStr; while( i > 0 && m_pStr[ i - 1 ] <= ' ' ) { --i; } if( i < m_cStr ) { i = m_cStr - i; Remove( -( intptr )i, i ); } return *this; }
		inline String &Trim() { return TrimLeft().TrimRight(); }

		inline String TrimmedLeft() const { return Copy().TrimLeft(); }
		inline String TrimmedRight() const { return Copy().TrimRight(); }
		inline String Trimmed() const { return Copy().Trim(); }

		inline operator char *() { return m_pStr; }
		inline operator const char *() const { return m_pStr != nullptr ? m_pStr : ""; }
		inline const char *c_str() const { return m_pStr != nullptr ? m_pStr : ""; }
		inline const char *CString() const { return m_pStr != nullptr ? m_pStr : ""; }

		inline char &operator[]( uintptr index ) { return m_pStr[ index ]; }
		inline char operator[]( uintptr index ) const { return ( index < m_cStr ) ? m_pStr[ index ] : '\0'; }
		inline char At( uintptr index ) const { return ( index < m_cStr ) ? m_pStr[ index ] : '\0'; }

		inline String &operator=( const char *p ) { Assign( p ); return *this; }
		inline String &operator=( const String &x ) { Assign( x ); return *this; }

		inline String &operator+=( const char *p ) { Append( p ); return *this; }
		inline String &operator+=( const String &x ) { Append( x ); return *this; }

		String &operator*=( int n );

		inline String &operator-=( const char *p ) { Remove( p ); return *this; }
		inline String &operator-=( const String &x ) { Remove( x ); return *this; }

		inline String operator+( const char *p ) const { return String( *this ) += p; }
		inline String operator+( const String &x ) const { return String( *this ) += x; }

		inline String operator-( const char *p ) const { return String( *this ) -= p; }
		inline String operator-( const String &x ) const { return String( *this ) -= x; }

		inline String operator*( int n ) const { return String( *this ) *= n; }

		inline bool operator==( const char *p ) const { return Compare( p ); }
		inline bool operator==( const String &p ) const { return Compare( p ); }

		inline bool operator!=( const char *p ) const { return !Compare( p ); }
		inline bool operator!=( const String &p ) const { return !Compare( p ); }

		inline bool operator<( const char *p ) const { return LexCompare( p ) < 0; }
		inline bool operator<( const String &p ) const { return LexCompare( p ) < 0; }

		inline bool operator>( const char *p ) const { return LexCompare( p ) > 0; }
		inline bool operator>( const String &p ) const { return LexCompare( p ) > 0; }

		inline bool operator<=( const char *p ) const { return LexCompare( p ) <= 0; }
		inline bool operator<=( const String &p ) const { return LexCompare( p ) <= 0; }

		inline bool operator>=( const char *p ) const { return LexCompare( p ) >= 0; }
		inline bool operator>=( const String &p ) const { return LexCompare( p ) >= 0; }

		static inline int GetDigit( char ch, int radix = 10 )
		{
			if( radix < 2 ) {
				return -1;
			}

			const int radixlo = radix > 10 ? 10 : radix;
			const int radixhi = radix > 10 ? radix - 10 : 0;

			if( ch >= '0' && ch < '0' + radixlo ) {
				return ( int )( ch - '0' );
			}
			if( ch >= 'a' && ch < 'a' + radixhi ) {
				return 10 + ( int )( ch - 'a' );
			}
			if( ch >= 'A' && ch < 'A' + radixhi ) {
				return 10 + ( int )( ch - 'A' );
			}

			return -1;
		}
		static inline bool IsDigit( char ch, int radix = 10 )
		{
			return GetDigit( ch, radix ) != -1;
		}

		static int DecodeRadix( const char *&p, int radix = kRadix_CStyle );
		static uint64 ToUnsignedInteger( const char *p, int radix = kRadix_CStyle, char chDigitSep = '\'' );
		static int64 ToInteger( const char *p, int radix = kRadix_CStyle, char chDigitSep = '\'' );
		static double ToFloat( const char *p, int radix = kRadix_CStyle, char chDigitSep = '\'' );

		static String FromUnsignedInteger( uint64 value, int radix = 10, uintptr numDigitsForSep = 3, char chDigitSep = ',' );
		static String FromInteger( int64 value, int radix = 10, uintptr numDigitsForSep = 3, char chDigitSep = ',' );
		static String FromFloat( double value, int radix = 10, uintptr maxTrailingDigits = 7, uintptr numDigitsForSep = 3, char chDigitSep = ',' );

		inline uint64 ToUnsignedInteger( int radix = kRadix_CStyle, char chDigitSep = '\'' ) const
		{
			return ToUnsignedInteger( m_pStr, radix, chDigitSep );
		}
		inline int64 ToInteger( int radix = kRadix_CStyle, char chDigitSep = '\'' ) const
		{
			return ToInteger( m_pStr, radix, chDigitSep );
		}
		inline double ToFloat( int radix = kRadix_CStyle, char chDigitSep = '\'' ) const
		{
			return ToFloat( m_pStr, radix, chDigitSep );
		}

		inline operator uint64() const            { return ToUnsignedInteger(); }
		inline operator int64() const             { return ToInteger(); }
		inline operator unsigned int() const      { return ( unsigned int )ToUnsignedInteger(); }
		inline operator int() const               { return ( int )ToInteger(); }

		static inline bool IsLower( char ch )     { return ch >= 'a' && ch <= 'z'; }
		static inline bool IsUpper( char ch )     { return ch >= 'A' && ch <= 'Z'; }
		static inline bool IsAlpha( char ch )     { return IsLower( ch ) || IsUpper( ch ); }
		static inline bool IsAlnum( char ch )     { return IsAlpha( ch ) || ( ch >= '0' && ch <= '9' ); }
		static inline bool IsNameMisc( char ch )  { return ch == '_'; }
		static inline bool IsNameStart( char ch ) { return IsAlpha( ch ) || IsNameMisc( ch ); }
		static inline bool IsName( char ch )      { return IsAlnum( ch ) || IsNameMisc( ch ); }
		static bool IsUnicodeNameStart( uint32 uCodePoint );
		static bool IsUnicodeName( uint32 uCodePoint );

		static uint32 HanToZen( uint32 uCodePoint );
		static uint32 ZenToHan( uint32 uCodePoint );

		String HanToZen() const;
		String ZenToHan() const;

		// NOTE: These convert to UTF32
		bool AppendUTF32Char( uint32 utf32Char );
		bool AppendUTF16( const wchar_t *utf16, intptr count = -1 );
		inline bool AppendUTF16( wchar_t utf16 ) { const wchar_t wc[ 2 ] = { utf16, L'\0' }; return AppendUTF16( wc ); }

		inline bool AssignUTF16( const wchar_t *utf16 ) { Empty(); return AppendUTF16( utf16 ); }
		inline bool AssignUTF16( wchar_t utf16 ) { const wchar_t wc[ 2 ] = { utf16, L'\0' }; return AssignUTF16( wc ); }

		uintptr UTF8Len() const;
		intptr UTF8Index( uintptr charIndex ) const;

		uint32 UTF32Character( uintptr charIndex ) const;
		uint32 CodePointFromOffset( uintptr uByteOffset, uintptr *pOutNumBytesEncoded = nullptr ) const;
		static uint32 UTF8CodePoint( const char *pszUTF8Source, uintptr *pOutNumBytesEncoded = nullptr );

		uintptr CountUTF16CodePoints() const;
		bool ConvertUTF16( uint16 *utf16, uintptr maxUtf16CodePoints, uintptr *pOutNumCodePoints = nullptr ) const;
		bool ConvertUTF32( uint32 *utf32, uintptr maxUtf32CodePoints, uintptr *pOutNumCodePoints = nullptr ) const;

		TArray< uint8 > ConvertToEncoding( EEncoding Encoding, EByteOrderMark BOM = EByteOrderMark::Enabled ) const;
		bool ConvertFromEncoding( const TArray< uint8 > &Data, EEncoding InEncoding = EEncoding::Unknown );
		static inline String FromEncoding( const TArray< uint8 > &Data, EEncoding Encoding = EEncoding::Unknown )
		{
			String Result;
			Result.ConvertFromEncoding( Data, Encoding );
			return Result;
		}
		
		bool ToUTF16( TArray< uint16 > &UTF16Arr ) const;
		bool ToUTF32( TArray< uint32 > &UTF32Arr ) const;

		TArray< uint16 > AsUTF16() const;
		TArray< uint32 > AsUTF32() const;

		inline wchar_t *ToWStr( wchar_t *buf, uintptr maxbuf ) const
		{
			ax_static_assert( sizeof( wchar_t ) == 2 || sizeof( wchar_t ) == 4, "Unexpected wchar_t type" );

			if( sizeof( wchar_t ) == 2 ) {
				return ConvertUTF16( ( uint16 * )buf, maxbuf ) ? buf : nullptr;
			}

			return ConvertUTF32( ( uint32 * )buf, maxbuf ) ? buf : nullptr;
		}
		template< uintptr tMaxBuf >
		inline wchar_t *ToWStr( wchar_t( &buf )[ tMaxBuf ] ) const
		{
			return ToWStr( buf, tMaxBuf );
		}

	private:
		uintptr						m_cStr;
		uintptr						m_cMax;
		char *						m_pStr;
		const int					m_iTag;

		static void GetOffsetAndSize( const char *p, uintptr &dstOffset, uintptr &dstCount, intptr first, intptr count );
		void GetOffsetAndSize( uintptr &dstOffset, uintptr &dstCount, intptr first, intptr count ) const;
	};

	inline String operator+( const char *p, const String &x )
	{
		return String( p ) += x;
	}
	inline String operator-( const char *p, const String &x )
	{
		return String( p ) -= x;
	}
	inline String operator*( int n, const String &x )
	{
		return x*n;
	}

	inline bool operator==( const char *p, const String &x )
	{
		return x.Compare( p );
	}
	inline bool operator!=( const char *p, const String &x )
	{
		return !x.Compare( p );
	}

	inline bool operator<( const char *p, const String &x )
	{
		return x.LexCompare( p ) >= 0;
	}
	inline bool operator>( const char *p, const String &x )
	{
		return x.LexCompare( p ) <= 0;
	}
	inline bool operator<=( const char *p, const String &x )
	{
		return x.LexCompare( p ) > 0;
	}
	inline bool operator>=( const char *p, const String &x )
	{
		return x.LexCompare( p ) < 0;
	}

	inline String &String::Swap( String &x )
	{
		Ax::Swap( m_cStr, x.m_cStr );
		Ax::Swap( m_cMax, x.m_cMax );
		Ax::Swap( m_pStr, x.m_pStr );

		return *this;
	}

	inline axstr_encoding_t GetEncoding( EEncoding Enc )
	{
		switch( Enc )
		{
		case EEncoding::Unknown:
			break;

		case EEncoding::UTF8:
			return axstr_enc_utf8;

		case EEncoding::UTF16_BE:
			return axstr_enc_utf16_be;
		case EEncoding::UTF16_LE:
			return axstr_enc_utf16_le;

		case EEncoding::UTF32_BE:
			return axstr_enc_utf32_be;
		case EEncoding::UTF32_LE:
			return axstr_enc_utf32_le;
		}

		return axstr_enc_unknown;
	}
	inline axstr_byteordermark_t GetBOM( EByteOrderMark BOM )
	{
		if( BOM == EByteOrderMark::Enabled ) {
			return axstr_bom_enabled;
		}

		return axstr_bom_disabled;
	}

	inline wchar_t *ToWStr( wchar_t *pBuf, uintptr cMaxBufWChars, const char *pszSource )
	{
		static_assert( sizeof( wchar_t ) == 2 || sizeof( wchar_t ) == 4, "Invalid wchar_t" );

		if( sizeof( wchar_t ) == 4 ) {
			return axstr_utf8_to_utf32( ( axstr_utf32_t * )pBuf, cMaxBufWChars, ( const axstr_utf8_t * )pszSource ) ? pBuf : nullptr;
		} else if( sizeof( wchar_t ) == 2 ) {
			return axstr_utf8_to_utf16( ( axstr_utf16_t * )pBuf, cMaxBufWChars, ( const axstr_utf8_t * )pszSource ) ? pBuf : nullptr;
		}

		return nullptr;
	}
	template< uintptr tMaxBufWChars >
	inline wchar_t *ToWStr( wchar_t( &Buf )[ tMaxBufWChars ], const char *pszSource )
	{
		return ToWStr( &Buf[0], tMaxBufWChars, pszSource );
	}

}

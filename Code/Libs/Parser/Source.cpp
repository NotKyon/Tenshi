#include "Source.hpp"

namespace Ax { namespace Parser {

	/*
	===========================================================================

		SOURCE PROCESSOR

		The source processor is responsible for transforming text before and
		during tokenization of any source file. This is a part of the
		representation of the language. It keeps track of things like language
		side keywords, operators, etc. For many languages these are static and
		never changing, while for others they can be changed dynamically within
		the source files. The source processor holds the majority of the
		configuration of these sorts of settings.

		The tokenizer (CSource) will invoke the source processor to apply
		necessary preprocessing steps (such as line concatenation), as well as
		when a token needs to be translated to an actual data value. For
		example, when a string needs to have its escape sequences processed, or
		encoded into UTF-16, or UTF-32 data.

	===========================================================================
	*/

	// TODO: Add string escape *PROCESSING* (the configuration is there)
	// ----- This seems to have been added. Double check. (141105)

	// TODO: Add source file processing (for line concatenation and similar)

	// FIXME: Trigraph expansion changes columns which will give somewhat
	//        incorrect reports.

	const CSourceProcessor &CSourceProcessor::GetCxxDefault()
	{
		static CSourceProcessor Instance;
		static bool bDidInit = false;

		if( bDidInit ) {
			return Instance;
		}

		bDidInit = true;

		Instance.UseCLineCat();

		Instance.AddCOperators();
		Instance.AddCPreprocessorOperators();
		Instance.AddC99Digraphs();
		Instance.AddCxxDigraphs();
		Instance.AddCTrigraphs();

		Instance.UseCStringEscapes();

		return Instance;
	}
	
	CSourceProcessor::CSourceProcessor( const char *pszAllowed, ECase Casing )
	: m_LineCat()
	, m_LineSep()
	, m_SpecialEndings( "#$" )
	, m_Punctuation()
	, m_Digraphs()
	, m_NamedOperators()
	, m_Keywords()
	, m_KeywordEntries()
	, m_SingleLineComments()
	, m_MultiLineComments()
	, m_StringEscapeChar( '\0' )
	, m_StringEscapePrefixChar( '\0' )
	, m_Flags( 0 )
	{
		for( char &x : m_TrigraphMap ) {
			x = '\0';
		}
		for( char &x : m_StringEscapeMap ) {
			x = '\0';
		}
		for( EStringEscape &x : m_StringEscapeMode ) {
			x = EStringEscape::Disabled;
		}
		AX_VERIFY_MSG( m_Keywords.Init( pszAllowed, Casing ), "Dictionary failed to initialize" );
	}
	CSourceProcessor::~CSourceProcessor()
	{
		for( uintptr i = 0; i < m_KeywordEntries.Num(); ++i ) {
			SKeywordEntry *const pEntry = m_KeywordEntries[ i ];
			if( !pEntry ) {
				continue;
			}

			delete pEntry->pData;
			pEntry->pData = nullptr;
		}
	}

	String CSourceProcessor::ExpandTrigraphs( const String &Source ) const
	{
		String Result;

		AX_VERIFY_MSG( Result.Reserve( Source.Len() ), "Out of memory" );

		const char *s = Source.CString();
		const char *p = s;

		while( *p != '\0' ) {
			if( *p != '?' || *( p + 1 ) != '?' ) {
				++p;
				continue;
			}

			Result.Append( s, intptr( p - s ) );

			const uint8 index = ( ( const uint8 * )p )[2] - 0x21;
			p += 3;
			s = p;

			if( index < 0x7F - 0x21 && m_TrigraphMap[ index ] != '\0' ) {
				Result.Append( m_TrigraphMap[ index ] );
			}
		}

		Result.Append( s, intptr( p - s ) );
		return Result;
	}
	String CSourceProcessor::ProcessText( const String &Source, TArray< uintptr > &OutLines ) const
	{
		String TrigraphExpansion;
		String Result;
		
		//
		//	TODO: Respect the kSrcF_NoCommentLineConcat flag (to avoid
		//	-     concatenating comment lines)
		//

		const bool bTrigraphs = AllFlagsOn( kSrcF_ExpandTrigraphs );
		const bool bDoLineCat = AllFlagsOn( kSrcF_CombineLines );

		AX_VERIFY_MSG( Result.Reserve( Source.Num() ), "Out of memory" );

		OutLines.Clear();
		OutLines.Append( 0 );

		if( bTrigraphs ) {
			AX_VERIFY_MSG( TrigraphExpansion.Assign( ExpandTrigraphs( Source ) ), "Out of memory" );
		}

		const char *const pszBase = bTrigraphs ? TrigraphExpansion : Source;
		const char *s = pszBase;
		const char *p = pszBase;

		while( *p != '\0' ) {
			const char *const e = p;

			const char *q = e;

			const bool bLineCat = bDoLineCat && *q == m_LineCat[ 0 ] && Cmp( q, m_LineCat, m_LineCat.Len() );
			if( bLineCat ) {
				q += m_LineCat.Len();
			}

			bool bNewline = false;
			if( *q == '\r' ) {
				bNewline = true;
				++q;
			}
			if( *q == '\n' ) {
				bNewline = true;
				++q;
			}

			if( !bNewline ) {
				++p;
				continue;
			}

			Result.Append( s, intptr( e - s ) );
			Result.Append( '\n' );
			OutLines.Append( Result.Len() );

			p = q;
			s = p;
		}

		Result.Append( s, intptr( p - s ) );
		return Result;
	}

	void CSourceProcessor::FlagsOn( uint32 Flags )
	{
		m_Flags |= Flags;
	}
	void CSourceProcessor::FlagsOff( uint32 Flags )
	{
		m_Flags &= ~Flags;
	}
	bool CSourceProcessor::AnyFlagsOn( uint32 Flags ) const
	{
		return ( m_Flags & Flags ) != 0;
	}
	bool CSourceProcessor::AllFlagsOn( uint32 Flags ) const
	{
		return ( m_Flags & Flags ) == Flags;
	}

	void CSourceProcessor::UseCLineCat()
	{
		m_LineCat = "\\";
	}
	
	void CSourceProcessor::AddOperators( const char *const *ppszOperators, uintptr cOperators )
	{
		AX_ASSERT_NOT_NULL( ppszOperators );
		AX_EXPECT( m_Punctuation.Reserve( m_Punctuation.Num() + cOperators ) );
		for( uint32 i = 0; i < cOperators; ++i ) {
			AX_ASSERT_NOT_NULL( ppszOperators[ i ] );
			AX_EXPECT( m_Punctuation.Append( ppszOperators[ i ] ) );
		}
	}

	static bool AppendEntry( char chEntry, char *&pszEntryBuf, uintptr &cEntryBufBytes )
	{
		if( cEntryBufBytes <= 1 ) {
			return false;
		}

		*pszEntryBuf++ = chEntry;
		--cEntryBufBytes;

		*pszEntryBuf = '\0';
		return true;
	}
	static bool EncodeEntry( uint32 uCodePoint, char *&pszEntryBuf, uintptr &cEntryBufBytes )
	{
		static const char *const pszDigits = "0123456789ABCDEF";
		uintptr cChars;
		if( uCodePoint > 0xFFFF ) {
			if( uCodePoint > 0xFFFFF ) {
				return false;
			}

			if( !AppendEntry( '\xF5', pszEntryBuf, cEntryBufBytes ) ) {
				return false;
			}

			cChars = 5;
		} else {
			if( !AppendEntry( '\xF4', pszEntryBuf, cEntryBufBytes ) ) {
				return false;
			}

			cChars = 4;
		}

		if( cEntryBufBytes < cChars + 1 ) {
			return false;
		}

		uintptr i = 0;
		uint32 n = uCodePoint;
		while( i < cChars ) {
			AX_ASSERT( i < cChars );

			const uint32 x = n%16;
			n /= 16;

			pszEntryBuf[ cChars - ++i ] = pszDigits[ x ];
		}

		pszEntryBuf += cChars;
		cEntryBufBytes -= cChars;

		*pszEntryBuf = '\0';

		return true;
	}
	static bool EncodeKeyword( const char *pszKeyword, char *pszEntryBuf, uintptr cEntryBufBytes )
	{
		AX_ASSERT_NOT_NULL( pszKeyword );
		AX_ASSERT_NOT_NULL( pszEntryBuf );
		AX_ASSERT( cEntryBufBytes > 16 );

		char *pbuf = pszEntryBuf;
		uintptr nbuf = cEntryBufBytes;

		*pbuf = '\0';

		for( const char *p = pszKeyword; *p != '\0'; ++p ) {
			if( *p >= 0x20 && *p < 0x7F ) {
				if( !AppendEntry( *p, pbuf, nbuf ) ) {
					return false;
				}

				continue;
			}

			uintptr ncp = 1;
			const uint32 cp = String::UTF8CodePoint( p, &ncp );
			p += ncp - 1;

			if( !EncodeEntry( cp, pbuf, nbuf ) ) {
				return false;
			}
		}

		return true;
	}

	void CSourceProcessor::AddKeywords( const char *const *ppszKeywords, uintptr cKeywords, EKeywordExists Mode )
	{
		char szEntryBuf[ 128 ];
		AX_ASSERT_NOT_NULL( ppszKeywords );
		AX_EXPECT( m_KeywordEntries.Reserve( m_KeywordEntries.Num() + cKeywords ) );
		for( uint32 i = 0; i < cKeywords; ++i ) {
			AX_ASSERT_NOT_NULL( ppszKeywords[ i ] );
			AX_EXPECT_MSG( EncodeKeyword( ppszKeywords[ i ], szEntryBuf, sizeof( szEntryBuf ) ), "Invalid keyword" );
			SKeywordEntry *const pEntry = m_Keywords.Lookup( szEntryBuf );
			AX_EXPECT_MSG( pEntry != nullptr, "Invalid string format or out of memory" );
			AX_EXPECT_MSG( pEntry->pData == nullptr || Mode != EKeywordExists::Error, "Keyword already exists" );

			if( pEntry->pData != nullptr && Mode == EKeywordExists::Ignore ) {
				continue;
			}

			delete pEntry->pData;
			pEntry->pData = new SKeyword( ppszKeywords[ i ] );
			AX_EXPECT_MEMORY( pEntry->pData );
			AX_EXPECT( m_KeywordEntries.Append( pEntry ) );
		}
	}
	void CSourceProcessor::AddKeywords( const SKeyword *pKeywords, uintptr cKeywords, EKeywordExists Mode )
	{
		char szEntryBuf[ 128 ];
		AX_ASSERT_NOT_NULL( pKeywords );
		AX_EXPECT( m_KeywordEntries.Reserve( m_KeywordEntries.Num() + cKeywords ) );
		for( uint32 i = 0; i < cKeywords; ++i ) {
			AX_ASSERT_NOT_NULL( pKeywords[ i ].pszName );
			AX_EXPECT_MSG( EncodeKeyword( pKeywords[ i ].pszName, szEntryBuf, sizeof( szEntryBuf ) ), "Invalid keyword" );
			SKeywordEntry *const pEntry = m_Keywords.Lookup( szEntryBuf );
			AX_EXPECT_MSG( pEntry != nullptr, "Invalid string format or out of memory" );
			AX_EXPECT_MSG( pEntry->pData == nullptr || Mode != EKeywordExists::Error, "Keyword already exists" );

			if( pEntry->pData != nullptr && Mode == EKeywordExists::Ignore ) {
				continue;
			}

			delete pEntry->pData;
			pEntry->pData = new SKeyword( pKeywords[ i ] );
			AX_EXPECT_MEMORY( pEntry->pData );
			AX_EXPECT( m_KeywordEntries.Append( pEntry ) );
		}
	}
	SKeyword *CSourceProcessor::FindKeyword( const char *pszKeyword ) const
	{
		char szEntryBuf[ 256 ];

		if( !EncodeKeyword( pszKeyword, szEntryBuf, sizeof( szEntryBuf ) ) ) {
			return nullptr;
		}

		SKeywordEntry *const pEntry = m_Keywords.Find( szEntryBuf );
		return pEntry != nullptr ? pEntry->pData : nullptr;
	}
	
	void CSourceProcessor::AddDigraph( const char *pszFind, const char *pszReplace )
	{
		const SExpansion Expansion = { pszFind, pszReplace };
		AX_EXPECT( m_Digraphs.Append( Expansion ) );
	}
	void CSourceProcessor::AddTrigraph( char chFind, char chReplace )
	{
		const uint8 uFindIndex = uint8( chFind );
		AX_ASSERT( uFindIndex >= 0x21 && uFindIndex < 0x7F );

		m_TrigraphMap[ uFindIndex - 0x21 ] = chReplace;
	}
	void CSourceProcessor::AddNamedOperator( const char *pszFind, const char *pszReplace )
	{
		const SExpansion Expansion = { pszFind, pszReplace };
		AX_EXPECT( m_NamedOperators.Append( Expansion ) );
	}

	void CSourceProcessor::AddCOperators()
	{
		static const char *const pszOperators[] = {
			"<<=", ">>=", "...",

			"~=", "!=", "%=", "^=", "&=", "*=", "-=", "+=", "==", "|=",
			"<=", ">=", "/=", "&&", "||", "<<", ">>", "->", "++", "--",

			"~", "!", "%", "^", "&", "*", "(", ")", "-", "+", "=", "/",
			"[", "]", "{", "}", "|", ";", ":", ",", ".", "<", ">", "?"
		};

		AddOperators( pszOperators );
	}
	void CSourceProcessor::AddCxxOperators()
	{
		static const char *const pszOperators[] = {
			"::"
		};

		AddOperators( pszOperators );
	}
	void CSourceProcessor::AddCPreprocessorOperators()
	{
		static const char *const pszOperators[] = {
			"##",

			"#"
		};

		AddOperators( pszOperators );
	}

	void CSourceProcessor::AddC99Digraphs()
	{
		AddDigraph( "<:", "[" );
		AddDigraph( ":>", "]" );
		AddDigraph( "<%", "{" );
		AddDigraph( "%>", "}" );
		AddDigraph( "%:", "#" );
	}
	void CSourceProcessor::AddCxxDigraphs()
	{
		AddDigraph( "%:%:", "##" );
	}
	void CSourceProcessor::AddCxxNamedOperators()
	{
		AddNamedOperator( "and", "&&" );
		AddNamedOperator( "and_eq", "&=" );
		AddNamedOperator( "bitand", "&" );
		AddNamedOperator( "bitor", "|" );
		AddNamedOperator( "compl", "~" );
		AddNamedOperator( "not", "!" );
		AddNamedOperator( "not_eq", "!=" );
		AddNamedOperator( "or", "||" );
		AddNamedOperator( "or_eq", "|=" );
		AddNamedOperator( "xor", "^" );
		AddNamedOperator( "xor_eq", "^=" );
	}
	void CSourceProcessor::AddPascalDigraphs()
	{
		AddDigraph( "(.", "[" );
		AddDigraph( ".)", "]" );
		AddDigraph( "(*", "{" );
		AddDigraph( "*)", "}" );
	}

	void CSourceProcessor::AddCTrigraphs()
	{
		AddTrigraph( '=', '#' );
		AddTrigraph( '/', '\\' );
		AddTrigraph( '\'', '^' );
		AddTrigraph( '(', '[' );
		AddTrigraph( ')', ']' );
		AddTrigraph( '!', '|' );
		AddTrigraph( '<', '{' );
		AddTrigraph( '>', '}' );
		AddTrigraph( '-', '~' );
	}

	void CSourceProcessor::AddCKeywords()
	{
		static const char *const ppszKeywords[] = {
			"break",
			"case",
			"char",
			"const",
			"continue",
			"default",
			"do",
			"double",
			"else",
			"enum",
			"extern",
			"float",
			"for",
			"goto",
			"if",
			"int",
			"long",
			"register",
			"return",
			"short",
			"signed",
			"sizeof",
			"static",
			"struct",
			"switch",
			"typedef",
			"union",
			"unsigned",
			"void",
			"volatile",
			"while"
		};

		AddKeywords( ppszKeywords );
	}
	void CSourceProcessor::AddC99Keywords()
	{
		static const char *const ppszKeywords[] = {
			"_Pragma",
			"inline"
		};

		AddKeywords( ppszKeywords );
	}
	void CSourceProcessor::AddCxxKeywords()
	{
		static const char *const ppszKeywords[] = {
			"class",
			"bool",
			"true",
			"false",
			"catch",
			"const_cast",
			"delete",
			"dynamic_cast",
			"explicit",
			"friend",
			"inline",
			"mutable",
			"namespace",
			"new",
			"operator",
			"private",
			"protected",
			"public",
			"reinterpret_cast",
			"static_cast",
			"template",
			"this",
			"throw",
			"try",
			"typeid",
			"typename",
			"using",
			"virtual"
		};

		AddKeywords( ppszKeywords );
	}
	void CSourceProcessor::AddCxx11Keywords()
	{
		static const char *const ppszKeywords[] = {
			"auto",
			"decltype",
			"final",
			"nullptr",
			"override"
		};

		AddKeywords( ppszKeywords );
	}

	void CSourceProcessor::AddGNUCKeywords()
	{
		static const char *const ppszKeywords[] = {
			"__attribute__",
			"__asm__",
			"__volatile__"
		};

		AddKeywords( ppszKeywords );
	}
	void CSourceProcessor::AddMSVCKeywords()
	{
		static const char *const ppszKeywords[] = {
			"__declspec",
			"__alignof",
			"__asm",
			"__based",
			"__try",
			"__except",
			"__finally",
			"__leave",
			"__forceinline",
			"__inline",
			"__int8",
			"__int16",
			"__int32",
			"__int64",
			"__identifier",
			"__if_exists",
			"__if_not_exists",
			"__interface",
			"__single_inheritance",
			"__multiple_inheritance",
			"__virtual_inheritance",
			"__uuidof",
			"__super",
			"__pragma",
			"__cdecl",
			"__stdcall",
			"__fastcall",
			"__thiscall"
		};

		AddKeywords( ppszKeywords );
	}

	void CSourceProcessor::ResetEscapes()
	{
		for( uintptr i = 0; i < 0x7F - 0x21; ++i ) {
			m_StringEscapeMap[ i ] = '\0';
			m_StringEscapeMode[ i ] = EStringEscape::Disabled;
		}
		m_StringEscapeChar = '\0';
		m_StringEscapePrefixChar = '\0';
	}

	void CSourceProcessor::SetStringEscape( char chEscapeCode, char chReplaceCharacter )
	{
		AX_ASSERT( chEscapeCode >= 0x21 && chEscapeCode < 0x7F );

		const uintptr index = uintptr( +( chEscapeCode - 0x21 ) );

		m_StringEscapeMap[ index ] = chReplaceCharacter;
		m_StringEscapeMode[ index ] = EStringEscape::Replace;
	}
	void CSourceProcessor::SetStringEscapeMode( char chEscapeCode, EStringEscape Mode )
	{
		AX_ASSERT( chEscapeCode >= 0x21 && chEscapeCode < 0x7F );
		AX_ASSERT( Mode != EStringEscape::Replace );

		const uintptr index = uintptr( +( chEscapeCode - 0x21 ) );
		m_StringEscapeMode[ index ] = Mode;
	}
	void CSourceProcessor::SetStringEscapeChar( char chEscape, char chEscapePrefix )
	{
		m_StringEscapeChar = chEscape;
		m_StringEscapePrefixChar = chEscapePrefix == '\0' ? chEscape : chEscapePrefix;
	}

	void CSourceProcessor::UseCStringEscapes()
	{
		ResetEscapes();

		SetStringEscapeChar( '\\' );

		SetStringEscape( '0', '\0' );
		SetStringEscape( 'a', '\a' );
		SetStringEscape( 'b', '\b' );
		SetStringEscape( 'f', '\f' );
		SetStringEscape( 'n', '\n' );
		SetStringEscape( 'r', '\r' );
		SetStringEscape( 't', '\t' );
		SetStringEscape( 'v', '\v' );
		SetStringEscape( '\'', '\'' );
		SetStringEscape( '\"', '\"' );
		SetStringEscape( '\?', '\?' );
		SetStringEscape( '\\', '\\' );

		SetStringEscapeMode( 'x', EStringEscape::InsertByteValue );
		SetStringEscapeMode( 'u', EStringEscape::InsertUnicode4 );
		SetStringEscapeMode( 'U', EStringEscape::InsertUnicode8 );
	}
	void CSourceProcessor::UseBlitzEscapes()
	{
		ResetEscapes();

		SetStringEscapeChar( '~' );

		SetStringEscape( '0', '\0' );
		SetStringEscape( 'n', '\n' );
		SetStringEscape( 'q', '\"' );
		SetStringEscape( 'r', '\r' );
		SetStringEscape( 't', '\t' );
		SetStringEscape( '~', '~' );
	}
	void CSourceProcessor::SetSpecialEndChars( const char *pszEndChars )
	{
		AX_EXPECT_MSG( m_SpecialEndings.Assign( pszEndChars ), "Out of memory" );
	}
	void CSourceProcessor::SetLineConcat( const char *pszConcat )
	{
		AX_EXPECT_MSG( m_LineCat.Assign( pszConcat ), "Out of memory" );
	}
	void CSourceProcessor::AddLineComment( const char *pszLineComment )
	{
		AX_EXPECT_MSG( m_SingleLineComments.Append( String( pszLineComment ) ), "Out of memory" );
	}
	void CSourceProcessor::AddMultiComment( const char *pszOpen, const char *pszClose )
	{
		AX_EXPECT_MSG( m_MultiLineComments.Resize( m_MultiLineComments.Num() + 1 ), "Out of memory" );
		m_MultiLineComments.Last().bNests = false;
		AX_EXPECT_MSG( m_MultiLineComments.Last().Enter.Assign( pszOpen ), "Out of memory" );
		AX_EXPECT_MSG( m_MultiLineComments.Last().Leave.Assign( pszClose ), "Out of memory" );
	}
	void CSourceProcessor::AddNestComment( const char *pszOpen, const char *pszClose )
	{
		AX_EXPECT_MSG( m_MultiLineComments.Resize( m_MultiLineComments.Num() + 1 ), "Out of memory" );
		m_MultiLineComments.Last().bNests = true;
		AX_EXPECT_MSG( m_MultiLineComments.Last().Enter.Assign( pszOpen ), "Out of memory" );
		AX_EXPECT_MSG( m_MultiLineComments.Last().Leave.Assign( pszClose ), "Out of memory" );
	}

	void CSourceProcessor::SetDigitSeparator( char chDigitSep )
	{
		m_DigitSeparatorChar = chDigitSep;
	}

	void CSourceProcessor::SetLineSeparator( const char *pszLineSep )
	{
		if( pszLineSep != nullptr ) {
			AX_EXPECT_MSG( m_LineSep.Assign( pszLineSep ), "Out of memory" );
		} else {
			m_LineSep.Clear();
		}
	}
	const String &CSourceProcessor::GetLineSeparator() const
	{
		return m_LineSep;
	}

	const char *CSourceProcessor::SkipWhitespace( const char *pszSrc, bool &bOutDidCrossLine )
	{
		AX_ASSERT_NOT_NULL( pszSrc );

		while( *( unsigned char * )pszSrc <= ' ' && *pszSrc != '\0' ) {
			if( *pszSrc == '\n' ) {
				bOutDidCrossLine = true;
			}

			++pszSrc;
		}

		return pszSrc;
	}
	const char *CSourceProcessor::SkipLine( const char *pszSrc, bool &bOutDidCrossLine )
	{
		AX_ASSERT_NOT_NULL( pszSrc );

		while( *pszSrc != '\0' ) {
			if( *pszSrc++ == '\n' ) {
				bOutDidCrossLine = true;
				break;
			}
		}

		return pszSrc;
	}
	const char *CSourceProcessor::SkipNestedSection( const char *pszSrc, const SMultilineComment &Section, bool &bOutDidCrossLine, ECase Casing )
	{
		AX_ASSERT_NOT_NULL( pszSrc );

		uint32 nestLevel = 0;

		do {
			if( ( nestLevel == 0 || Section.bNests ) && Cmp( Casing, pszSrc, Section.Enter, Section.Enter.Len() ) ) {
				pszSrc += Section.Enter.Len();
				++nestLevel;
				continue;
			} else if( nestLevel == 0 ) {
				break;
			}

			if( Cmp( Casing, pszSrc, Section.Leave, Section.Leave.Len() ) ) {
				pszSrc += Section.Leave.Len();
				--nestLevel;
				continue;
			}

			if( *pszSrc == '\n' ) {
				bOutDidCrossLine = true;
			}

			++pszSrc;
		} while( nestLevel > 0 );

		return pszSrc;
	}
	const char *CSourceProcessor::SkipName( const char *pszSrc, bool bSupportUnicode, const char *pszSpecialEndings, char *&pszEntryBuf, uintptr &cEntryBufBytes )
	{
		AX_ASSERT_NOT_NULL( pszSrc );

		uintptr cSkipBytes = 1;
		uint32 uCodePoint = 0;

		const char *const pszEndings = pszSpecialEndings != nullptr ? pszSpecialEndings : "";

		if( !String::IsNameStart( *pszSrc ) ) {
			if( !bSupportUnicode ) {
				return pszSrc;
			}

			uCodePoint = String::UTF8CodePoint( pszSrc, &cSkipBytes );
			if( !String::IsUnicodeNameStart( uCodePoint ) ) {
				return pszSrc;
			}

			AX_VERIFY( EncodeEntry( uCodePoint, pszEntryBuf, cEntryBufBytes ) );
		} else {
			AX_VERIFY( AppendEntry( *pszSrc, pszEntryBuf, cEntryBufBytes ) );
		}
		pszSrc += cSkipBytes;

		for(;;) {
			if( String::IsName( *pszSrc ) ) {
				AX_VERIFY( AppendEntry( *pszSrc, pszEntryBuf, cEntryBufBytes ) );
				++pszSrc;
				continue;
			} else if( !bSupportUnicode ) {
				break;
			}

			cSkipBytes = 1;
			uCodePoint = String::UTF8CodePoint( pszSrc, &cSkipBytes );
			if( !String::IsUnicodeName( uCodePoint ) ) {
				break;
			}

			AX_VERIFY( EncodeEntry( uCodePoint, pszEntryBuf, cEntryBufBytes ) );
			pszSrc += cSkipBytes;
		}

		while( strchr( pszEndings, *pszSrc ) != nullptr && *pszSrc != '\0' ) {
			AX_VERIFY( AppendEntry( *pszSrc, pszEntryBuf, cEntryBufBytes ) );
			++pszSrc;
		}

		return pszSrc;
	}
	
	static const char *FindWord( const char *pszSource, const char *pszWord, uintptr cWordBytes = ~uintptr( 0 ) )
	{
		AX_ASSERT_NOT_NULL( pszSource );
		AX_ASSERT_NOT_NULL( pszWord );

		const char *p = pszSource;
		const char chLower = *pszWord >= 'A' && *pszWord <= 'Z' ? *pszWord - 'A' + 'a' : *pszWord;
		const char chUpper = *pszWord >= 'a' && *pszWord <= 'z' ? *pszWord - 'a' + 'A' : *pszWord;

		const uintptr n = cWordBytes == ~uintptr( 0 ) ? strlen( pszWord ) : cWordBytes;

		while( *p != '\0' ) {
			const char *const srch1 = strchr( p, chLower );
			const char *const srch2 = strchr( p, chUpper );
			const char *q = srch1 < srch2 && srch1 != nullptr ? srch1 : srch2;
			if( q == NULL ) {
				break;
			}

			if( !CaseCmp( q, pszWord, n ) ) {
				p = q + 1;
				continue;
			}

			const char *r = q + cWordBytes;

			const bool bWhitespacePreceded = ( q > pszSource && *( unsigned char * )( q - 1 ) <= ' ' ) || ( q == pszSource );
			const bool bWhitespaceFollowed = *( unsigned char * )r <= ' ';

			if( !bWhitespacePreceded || !bWhitespaceFollowed ) {
				p = r;
				continue;
			}

			return q;
		}

		return NULL;
	}
	const char *CSourceProcessor::SkipWhitespace( const char *pszSource, CSource &SourceObj, TokenList &Tokens, bool &bOutDidCrossLine ) const
	{
		AX_ASSERT_NOT_NULL( pszSource );

		SToken CommentTok;

		const char *pszCurrent = pszSource;
		const char *pszOld;
		do {
			pszOld = pszCurrent;

			pszCurrent = SkipWhitespace( pszCurrent, bOutDidCrossLine );

			for( uintptr i = 0; i < m_SingleLineComments.Num(); ++i ) {
				if( *pszCurrent != m_SingleLineComments[ i ].At( 0 ) ) {
					continue;
				}

				if( !Cmp( pszCurrent, m_SingleLineComments[ i ], m_SingleLineComments[ i ].Len() ) ) {
					continue;
				}

				if( AllFlagsOn( kSrcF_KeepComments ) && CommentTok.Type == ETokenType::None ) {
					CommentTok.Type = ETokenType::Comment;
					CommentTok.CommentType = ECommentTokenType::Normal;
					CommentTok.Flags = 0;
					CommentTok.uOffset = pszCurrent - SourceObj.GetText();
				}

				pszCurrent += m_SingleLineComments[ i ].Len();
				pszCurrent = SkipLine( pszCurrent, bOutDidCrossLine );

				if( AllFlagsOn( kSrcF_KeepComments ) ) {
					CommentTok.cLength = ( pszCurrent - SourceObj.GetText() ) - CommentTok.uOffset;
				}
			}

			for( uintptr i = 0; i < m_MultiLineComments.Num(); ++i ) {
				if( *pszCurrent != m_MultiLineComments[ i ].Enter.At( 0 ) ) {
					continue;
				}

				const char *const pszOldCurrent = pszCurrent;
				pszCurrent = SkipNestedSection( pszCurrent, m_MultiLineComments[ i ], bOutDidCrossLine );
				if( pszCurrent != pszOldCurrent && AllFlagsOn( kSrcF_KeepComments ) ) {
					if( CommentTok.Type == ETokenType::None ) {
						CommentTok.Type = ETokenType::Comment;
						CommentTok.CommentType = ECommentTokenType::Normal;
						CommentTok.Flags = 0;
						CommentTok.uOffset = pszOldCurrent - SourceObj.GetText();
					}

					CommentTok.cLength = ( pszCurrent - SourceObj.GetText() ) - CommentTok.uOffset;
				}
			}

			if( AllFlagsOn( kSrcF_RemComments ) && CaseCmp( pszCurrent, "rem", 3 ) ) {
				int mode = 0;
				const char *pszCheck = pszCurrent + 3;

				if( CaseCmp( pszCheck, "start", 5 ) ) {
					pszCheck += 5;
					mode = 1;
				} else if( CaseCmp( pszCheck, "end", 3 ) ) {
					pszCheck += 3;
					mode = 2;
				}

				if( *( const unsigned char * )pszCheck > ' ' ) {
					continue;
				}

				// Ignore unmatched REMEND's (user might have purposely uncommented a REMSTART)
				if( mode == 2 ) {
					continue;
				}

				if( mode == 0 ) {
					pszCheck = SkipLine( pszCheck, bOutDidCrossLine );
				} else {
					const char *const pszNext = FindWord( pszCheck, "remend", 6 );
					if( pszNext != NULL ) {
						pszCheck = pszNext + 6;
					} else {
						pszCheck = strchr( pszCheck, '\0' );
					}
				}

				if( AllFlagsOn( kSrcF_KeepComments ) ) {
					if( CommentTok.Type == ETokenType::None ) {
						CommentTok.Type = ETokenType::Comment;
						CommentTok.CommentType = ECommentTokenType::Normal;
						CommentTok.Flags = 0;
						CommentTok.uOffset = pszCheck - SourceObj.GetText();
					}

					CommentTok.cLength = ( pszCurrent - SourceObj.GetText() ) - CommentTok.uOffset;
				}

				pszCurrent = pszCheck;
			}
		} while( pszOld != pszCurrent );

		if( AllFlagsOn( kSrcF_KeepComments ) && CommentTok.Type != ETokenType::None ) {
			CommentTok.pSource = &SourceObj;
			AX_VERIFY( Tokens.AddTail( CommentTok ) != Tokens.end() );
		}

		return pszCurrent;
	}

	static bool CollectDigit( const char *&p, int &iOutDigit )
	{
		if( *p == '\0' ) {
			iOutDigit = -1;
			return false;
		}

		iOutDigit = String::GetDigit( *p, 16 );
		++p;

		return iOutDigit != -1;
	}
	template< uintptr tNumDigits >
	static bool AppendCodePoint( const int( &iDigit )[ tNumDigits ], uintptr cDigits, String *pOutData )
	{
		if( !pOutData ) {
			return true;
		}

		uint32 CodePoint = 0;
		for( uintptr i = 0; i < cDigits; ++i ) {
			CodePoint *= 16;
			CodePoint += iDigit[ i ];
		}

		return pOutData->AppendUTF32Char( CodePoint );
	}
	const char *CSourceProcessor::SkipString( const char *pszSource, bool &bOutDidCrossLine, EStringTokenType &OutEncoding, String *pOutData ) const
	{
		AX_ASSERT_NOT_NULL( pszSource );

		const char *p = pszSource;

		OutEncoding = EStringTokenType::Unqualified;

		if( AllFlagsOn( kSrcF_CStringEncodings ) ) {
			if( *p == 'L' ) {
				++p;
				OutEncoding = EStringTokenType::Wide;
			} else if( *p == 'u' ) {
				++p;
				if( *p == '8' ) {
					OutEncoding = EStringTokenType::UTF8;
					++p;
				} else {
					OutEncoding = EStringTokenType::UTF16LE;
				}
			} else if( *p == 'U' ) {
				++p;
				OutEncoding = EStringTokenType::UTF32LE;
			}
		}

		bool bIsEscapedString = AllFlagsOn( kSrcF_StringEscapes );
		if( bIsEscapedString && AllFlagsOn( kSrcF_StringEscapesPrefixed ) ) {
			if( *p == m_StringEscapePrefixChar ) {
				++p;
			} else {
				bIsEscapedString = false;
			}
		}

		if( *p != '\"' ) {
			return pszSource;
		}

		++p;
		const char *s = p;
		const char *e = nullptr;

		if( !bIsEscapedString ) {
			e = strchr( p, '\"' );
			if( !e ) {
				e = strchr( p, '\0' );
			}

			if( pOutData != nullptr ) {
				AX_VERIFY_MSG( pOutData->Append( s, intptr( e - s ) ), "Out of memory" );
			}

			if( *e == '\"' ) {
				++e;
			}
			return e;
		}

		int iDigit[ 8 ];

		L_string_collection:
		while( *p != '\"' && *p != '\0' ) {
			if( *p == '\n' ) {
				if( AllFlagsOn( kSrcF_StringLineBreaks ) ) {
					bOutDidCrossLine = true;
					++p;
					continue;
				}

				// TODO: Warn about the unexpected new-line
				break;
			}

			if( *p != m_StringEscapeChar ) {
				++p;
				continue;
			}

			e = p;

			if( pOutData != nullptr ) {
				AX_VERIFY_MSG( pOutData->Append( s, intptr( e - s ) ), "Out of memory" );
			}

			++p;
			if( *( const uint8 * )p < 0x21 || *( const uint8 * )p >= 0x7F ) {
				// TODO: Warn about this invalid escape
				++p;
				continue;
			}

			const uint8 uEscapeIndex = ( *( const uint8 * )p ) - 0x21;
			++p;

			bool bIsValid = true;
			uint32 cDigits = 0;

			switch( m_StringEscapeMode[ uEscapeIndex ] ) {
			case EStringEscape::Disabled:
				// TODO: Warn about an unused escape
				break;

			case EStringEscape::Replace:
				if( pOutData != nullptr ) {
					AX_VERIFY_MSG( pOutData->Append( uint8( m_StringEscapeMap[ uEscapeIndex ] ) ), "Out of memory" );
				}
				break;

			case EStringEscape::InsertByteValue:
				bIsValid &= CollectDigit( p, iDigit[ 0 ] );
				bIsValid &= CollectDigit( p, iDigit[ 1 ] );
				if( !bIsValid ) {
					// TODO: Warn about the invalid sequence
					break;
				}
				if( pOutData != nullptr ) {
					const uint8 uValue = uint8( iDigit[ 0 ] )*16 + uint8( iDigit[ 1 ] );
					AX_VERIFY_MSG( pOutData->Append( uValue ), "Out of memory" );
				}
				break;

			case EStringEscape::InsertUnicode2:
				bIsValid &= CollectDigit( p, iDigit[ 0 ] );
				bIsValid &= CollectDigit( p, iDigit[ 1 ] );
				if( !bIsValid ) {
					// TODO: Warn about the invalid Unicode sequence
					break;
				}
				AX_VERIFY_MSG( AppendCodePoint( iDigit, 2, pOutData ), "Out of memory" );
				break;
			case EStringEscape::InsertUnicode4:
				bIsValid &= CollectDigit( p, iDigit[ 0 ] );
				bIsValid &= CollectDigit( p, iDigit[ 1 ] );
				bIsValid &= CollectDigit( p, iDigit[ 2 ] );
				bIsValid &= CollectDigit( p, iDigit[ 3 ] );
				if( !bIsValid ) {
					// TODO: Warn about the invalid Unicode sequence
					break;
				}
				AX_VERIFY_MSG( AppendCodePoint( iDigit, 4, pOutData ), "Out of memory" );
				break;
			case EStringEscape::InsertUnicode5:
				bIsValid &= CollectDigit( p, iDigit[ 0 ] );
				bIsValid &= CollectDigit( p, iDigit[ 1 ] );
				bIsValid &= CollectDigit( p, iDigit[ 2 ] );
				bIsValid &= CollectDigit( p, iDigit[ 3 ] );
				bIsValid &= CollectDigit( p, iDigit[ 4 ] );
				if( !bIsValid ) {
					// TODO: Warn about the invalid Unicode sequence
					break;
				}
				AX_VERIFY_MSG( AppendCodePoint( iDigit, 5, pOutData ), "Out of memory" );
				break;
			case EStringEscape::InsertUnicode8:
				bIsValid &= CollectDigit( p, iDigit[ 0 ] );
				bIsValid &= CollectDigit( p, iDigit[ 1 ] );
				bIsValid &= CollectDigit( p, iDigit[ 2 ] );
				bIsValid &= CollectDigit( p, iDigit[ 3 ] );
				bIsValid &= CollectDigit( p, iDigit[ 4 ] );
				bIsValid &= CollectDigit( p, iDigit[ 5 ] );
				bIsValid &= CollectDigit( p, iDigit[ 6 ] );
				bIsValid &= CollectDigit( p, iDigit[ 7 ] );
				if( !bIsValid ) {
					// TODO: Warn about the invalid Unicode sequence
					break;
				}
				AX_VERIFY_MSG( AppendCodePoint( iDigit, 8, pOutData ), "Out of memory" );
				break;
			case EStringEscape::InsertUnicode:
				if( *p == '{' ) {
					++p;
					cDigits = 0;
					while( *p != '\0' && *p != '\"' && *p != '}' && cDigits < 8 ) {
						bIsValid &= CollectDigit( p, iDigit[ cDigits ] );
						++cDigits;
					}
					if( *p == '}' ) {
						++p;
					}
				} else {
					bIsValid = false;
				}
				if( !bIsValid ) {
					// TODO: Warn about the invalid Unicode sequence
					break;
				}
				AX_VERIFY_MSG( AppendCodePoint( iDigit, cDigits, pOutData ), "Out of memory" );
				break;
			}

			s = p;
		}

		e = p;
		if( pOutData != nullptr ) {
			AX_VERIFY_MSG( pOutData->Append( s, intptr( e -s ) ), "Out of memory" );
		}

		if( *e == '\"' ) {
			++e;
		}

		if( AllFlagsOn( kSrcF_CombineQuotes ) ) {
			const char *x = e;
			while( *x <= ' ' && *x != '\0' ) {
				if( *x == '\n' ) {
					bOutDidCrossLine = true;
				}

				++x;
			}

			if( *x == '\"' ) {
				p = x + 1;
				goto L_string_collection;
			}
		}

		return e;
	}
	const char *CSourceProcessor::SkipIdentifier( const char *pszSource, SKeyword **ppOutKeyword ) const
	{
		AX_ASSERT_NOT_NULL( pszSource );

		char szEntryBuf[ 256 ];
		uintptr cEntryBufBytes = sizeof( szEntryBuf );
		char *pszEntryBuf = &szEntryBuf[ 0 ];
		SKeywordEntry *pLastGoodEntry = nullptr;

		const bool bSupportUnicode = AllFlagsOn( kSrcF_UnicodeNames );
		const char *const pszSpecialEndings = AllFlagsOn( kSrcF_SpecialEndings ) ? m_SpecialEndings.CString() : nullptr;

		//
		//	TODO: Add support for kSrcF_PathstyleNames (e.g., enemy/grue)
		//

		//
		//	TODO: Add support for kSrcF_KeywordEscapes (to explicitly mark an
		//	-     identifier as NOT a keyword)
		//	----> Would be good to also support, e.g., "__identifier()" style
		//	-     commands.
		//	----> The identifier still needs to be a valid identifier, so a
		//	-     check will need to be performed. It's merely the case that it
		//	-     won't be checked against a set of keywords.
		//

		const char *pszUserWord = nullptr;

		const char *pszCheckFrom = pszSource;
		do {
			if( pszCheckFrom != pszSource ) {
				AX_EXPECT_MSG( AppendEntry( ' ', pszEntryBuf, cEntryBufBytes ), "Identifier is too long" );
			}

			const char *const pszNext = SkipName( pszCheckFrom, bSupportUnicode, pszSpecialEndings, pszEntryBuf, cEntryBufBytes );
			if( !pszNext || pszNext == pszCheckFrom ) {
				break;
			}

			if( !pszUserWord ) {
				pszUserWord = pszNext;
			}

			SKeywordEntry *const pEntry = m_Keywords.Find( szEntryBuf );
			if( pEntry != nullptr && pEntry->pData != nullptr ) {
				pLastGoodEntry = pEntry;
				pszSource = pszNext;
			}

			pszCheckFrom = AllFlagsOn( kSrcF_SpacedNames ) && *pszNext == ' ' ? pszNext + 1 : nullptr;
		} while( pszCheckFrom != nullptr );

		if( ppOutKeyword != nullptr ) {
			*ppOutKeyword = pLastGoodEntry != nullptr ? pLastGoodEntry->pData : nullptr;
		}

		if( !pLastGoodEntry && pszUserWord != nullptr ) {
			return pszUserWord;
		}

		return pszSource;
	}

	static uint32 DecodeRadix( const char *&p, uint32 flags )
	{
		if( *p == '%' && String::GetDigit( *( p + 1 ), 2 ) != -1 && ( flags & kSrcF_BasicBinaries ) ) {
			++p;
			return 2;
		}
		
		if( *p == '$' && String::GetDigit( *( p + 1 ), 16 ) != -1 && ( flags & kSrcF_BasicHexadecimals ) ) {
			++p;
			return 16;
		}
		
		if( *p == '0' ) {
			const char ch = *( p + 1 );
			
			if( ch == 'b' || ch == 'B' ) {
				p += 2;
				return 2;
			}
			
			if( ( ch == 'c' || ch == 'C' ) && ( flags & kSrcF_BasicOctals ) ) {
				p += 2;
				return 8;
			}
			
			if( ch == 'x' || ch == 'X' ) {
				p += 2;
				return 16;
			}
			
			if( ~flags & kSrcF_BasicOctals ) {
				++p;
				return 8;
			}
		}
		
		if( flags & kSrcF_ArbitraryBases ) {
			int testradix = 0;
			const char *check = p;
			while( *check >= '0' && *check <= '9' ) {
				testradix *= 10;
				testradix += +( *check - '0' );
				++check;
			}
			if( ( *check == 'x' || *check == 'X' ) && testradix >= 2 && testradix <= 36 ) {
				p = check + 1;
				return testradix;
			}
		}

		return 10;
	}
	static uint64 ReadUIntPart( const char *&p, uint32 radix, char chDigitSep )
	{
		uint64 n = 0;

		while( *p != '\0' ) {
			const int digit = String::GetDigit( *p == chDigitSep && *p != '\0' ? *( p + 1 ) : *p, radix );
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
	const char *CSourceProcessor::SkipNumber( const char *pszSource, ENumberTokenType &OutType, ENumberTokenQualifier &OutQualifier, uint32 &OutFlags, uint64 &OutData ) const
	{
		AX_ASSERT_NOT_NULL( pszSource );

		const char *p = pszSource;

		const uint32 radix = DecodeRadix( p, m_Flags );
		const uint64 whole = ReadUIntPart( p, radix, m_DigitSeparatorChar );

		if( p == pszSource && !( *p == '.' && p[ 1 ] >= '0' && p[ 1 ] <= '9' ) ) {
			return pszSource;
		}

		const bool bHasDot = ( *p == '.' );
		if( bHasDot ) {
			++p;
		}

		const uint64 fract = bHasDot ? ReadUIntPart( p, radix, m_DigitSeparatorChar ) : 0;

		if( p == pszSource ) {
			// TODO: Check whether this is necessary
			return pszSource;
		}

		bool bHasSciNot = false;

		int expsign = 1;
		uint64 exp = 0;
		if( ( radix < 16 && ( *p == 'e' || *p == 'E' ) ) || ( radix < 26 && ( *p == 'p' || *p == 'P' ) ) ) {
			++p;
			if( *p == '-' ) {
				expsign = -1;
				++p;
			} else if( *p == '+' ) {
				++p;
			}
			exp = ReadUIntPart( p, radix, m_DigitSeparatorChar );
			bHasSciNot = true;
		}
		bool bIsFloat = bHasDot || bHasSciNot;
		
		OutQualifier = ENumberTokenQualifier::Unqualified;
		if( !AllFlagsOn( kSrcF_NoNumberQualifiers ) ) {
			char szQBuf[ 8 ] = { '\0' };
			for( uintptr i = 0; i < sizeof( szQBuf ) - 1; ++i ) {
				if( !( ( *p>='a' && *p<='z' ) || ( *p>='A' && *p<='Z' ) || ( *p>='0' && *p<='9' ) ) ) {
					break;
				}

				szQBuf[ i + 0 ] = *p++;
				szQBuf[ i + 1 ] = '\0';
			}

			if( szQBuf[ 0 ] != '\0' ) {
				if( CaseCmp( szQBuf, "F" ) ) {
					bIsFloat = true;
					OutQualifier = ENumberTokenQualifier::Float;
				} else if( CaseCmp( szQBuf, "L" ) ) {
					OutQualifier = ENumberTokenQualifier::Long;
				} else if( CaseCmp( szQBuf, "LL" ) ) {
					OutQualifier = ENumberTokenQualifier::LongLong;
				} else if( CaseCmp( szQBuf, "U" ) ) {
					OutQualifier = ENumberTokenQualifier::Unsigned;
				} else if( CaseCmp( szQBuf, "UL" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedLong;
				} else if( CaseCmp( szQBuf, "ULL" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedLongLong;
				} else if( CaseCmp( szQBuf, "I8" ) ) {
					OutQualifier = ENumberTokenQualifier::SignedInt8;
				} else if( CaseCmp( szQBuf, "I16" ) ) {
					OutQualifier = ENumberTokenQualifier::SignedInt16;
				} else if( CaseCmp( szQBuf, "I32" ) ) {
					OutQualifier = ENumberTokenQualifier::SignedInt32;
				} else if( CaseCmp( szQBuf, "I64" ) ) {
					OutQualifier = ENumberTokenQualifier::SignedInt64;
				} else if( CaseCmp( szQBuf, "U8" ) || CaseCmp( szQBuf, "UI8" ) || CaseCmp( szQBuf, "I8U" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedInt8;
				} else if( CaseCmp( szQBuf, "U16" ) || CaseCmp( szQBuf, "UI16" ) || CaseCmp( szQBuf, "I16U" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedInt16;
				} else if( CaseCmp( szQBuf, "U32" ) || CaseCmp( szQBuf, "UI32" ) || CaseCmp( szQBuf, "I32U" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedInt32;
				} else if( CaseCmp( szQBuf, "U64" ) || CaseCmp( szQBuf, "UI64" ) || CaseCmp( szQBuf, "I64U" ) ) {
					OutQualifier = ENumberTokenQualifier::UnsignedInt64;
				} else {
					//
					//	TODO: Warn on unknown qualifier
					//
				}
			}
		}

		if( bIsFloat ) {
			OutType = ENumberTokenType::Real;

			long double f = long double( whole );

			uint64 fractmag = 1;
			uint64 tmpfract = fract;
			while( tmpfract > 0 ) {
				fractmag *= radix;
				tmpfract /= radix;
			}

			f += long double( fract )/long double( fractmag );
			if( expsign > 0 ) {
				while( exp > 0 ) {
					f *= 10;
					--exp;
				}
			} else {
				while( exp > 0 ) {
					f /= 10;
					--exp;
				}
			}

			const double n = double( f );
			OutData = *( const uint64 * )&n;
		} else {
			OutType = ENumberTokenType::Integer;
			OutData = whole;
		}

		OutFlags = 0;
		switch( radix )
		{
		case 2:
			OutFlags |= kNumTokF_Binary;
			break;

		case 8:
			OutFlags |= kNumTokF_Octal;
			break;

		case 10:
			OutFlags |= kNumTokF_Decimal;
			break;

		case 16:
			OutFlags |= kNumTokF_Hexadecimal;
			break;

		default:
			OutFlags |= kNumTokF_ArbitraryBase;
			break;
		}

		if( bHasDot ) {
			OutFlags |= kNumTokF_HasDot;
		}
		if( bHasSciNot ) {
			OutFlags |= kNumTokF_SciNot;
		}

		return p;
	}
	const char *CSourceProcessor::SkipPunctuation( const char *pszSource ) const
	{
		for( uintptr i = 0; i < m_Punctuation.Num(); ++i ) {
			const String &Punct = m_Punctuation[ i ];
			if( Cmp( pszSource, Punct, Punct.Len() ) ) {
				return pszSource + Punct.Len();
			}
		}

		return pszSource;
	}

	
	/*
	===========================================================================

		SOURCE

	===========================================================================
	*/

	CSource::CSource()
	: m_Type( ESourceType::Memory )
	, m_Filename()
	, m_pParent( nullptr )
	, m_ParentIncludePos( kDefaultPosition )
	, m_pProcessor( nullptr )
	, m_ExpandedSources()
	, m_Text()
	, m_Current( 0 )
	, m_Lines()
	, m_LineDirectives()
	, m_Tokens()
	, m_CurrentToken( m_Tokens.end() )
	{
	}
	CSource::~CSource()
	{
	}

	void CSource::SetProcessor( const CSourceProcessor &SrcProc )
	{
		m_pProcessor = &SrcProc;
	}

	void CSource::SetText( const String &Filename, const String &Text, ESourceType Type )
	{
		m_Filename = Filename;
		m_Text = m_pProcessor != nullptr ? m_pProcessor->ProcessText( Text, m_Lines ) : Text;
		m_Type = Type;

		if( !m_pProcessor ) {
			//
			//	TODO: Find all the lines
			//
		}

		m_bDidCrossLine = true;
		m_Current = 0;
	}

	void CSource::LineDirective( const String &Name, uintptr LineNumber )
	{
		if( !m_LineDirectives.IsEmpty() ) {
			m_LineDirectives.Last()->LeavePos = m_Current;
		}

		if( Name.IsEmpty() && LineNumber == 0 ) {
			return;
		}

		TList< SLineDirective >::Iterator Directive = m_LineDirectives.AddTail();
		AX_EXPECT_MSG( Directive != m_LineDirectives.end(), "Out of memory" );

		Directive->Filename = Name;
		Directive->Line = LineNumber;
		Directive->EnterPos = m_Current;
		Directive->LeavePos = m_Text.Len();
	}

	TokenIter CSource::ReadToken()
	{
		// If there are unread tokens available then pull from there
		if( !m_Tokens.IsEmpty() && m_CurrentToken != m_Tokens.Last() ) {
			m_CurrentToken.Advance();
			return m_CurrentToken;
		}

		TokenIter BaseToken = m_CurrentToken;

		// Skip whitespace
		//
		// If the current token is altered it's because comments are being kept,
		// so we need to return the comment-block token found
		if( SkipWhitespace() && BaseToken != m_CurrentToken ) {
			BaseToken.Advance();
			return BaseToken;
		}

		// Check for a string
		//
		// A string needs to be checked for next because some strings can begin
		// with what looks like an identifier (e.g., L"Wide string")
		if( SkipString() ) {
			m_CurrentToken = m_Tokens.Last();
			return m_CurrentToken;
		}

		// Check for a number
		if( SkipNumber() ) {
			m_CurrentToken = m_Tokens.Last();
			return m_CurrentToken;
		}

		//
		//	TODO: Identifiers matching named operators need to be converted to
		//	-     the appropriate punctuation
		//

		// Check for an identifier
		if( SkipIdentifier() ) {
			m_CurrentToken = m_Tokens.Last();
			return m_CurrentToken;
		}

		// Check for punctuation
		if( SkipPunctuation() ) {
			m_CurrentToken = m_Tokens.Last();
			return m_CurrentToken;
		}

		//
		//	TODO: Generate an "unknown character" token
		//

		// Return an empty token
		return TokenIter();
	}
	void CSource::UnreadToken()
	{
		// Don't jump back too far
		if( m_CurrentToken == m_Tokens.begin() ) {
			return;
		}

		// This should probably never be the case
		if( m_CurrentToken == m_Tokens.end() ) {
			m_CurrentToken = m_Tokens.Last();
			return;
		}

		// Jump back in the stream
		m_CurrentToken.Retreat();
	}

	void CSource::SetCurrentToken( TokenIter Tok )
	{
		// Set the current token
		if( Tok == m_Tokens.end() ) {
			m_CurrentToken = m_Tokens.Last();
			return;
		}

#if AX_DEBUG_ENABLED
		if( !m_Tokens.IsEmpty() ) {
			AX_ASSERT_NOT_NULL( Tok.pLink );
			AX_ASSERT_NOT_NULL( m_Tokens.begin().pLink );

			AX_ASSERT_MSG( Tok.pLink->List() == m_Tokens.begin().pLink->List(), "Invalid token iterator" );
		}
#endif

		m_CurrentToken = Tok;
	}

	void CSource::SkipLine()
	{
		const uintptr cText = m_Text.Len();
		while( m_Current < cText && m_Text[ m_Current ] != '\n' ) {
			++m_Current;
		}
	}

	SLineInfo CSource::CalculateLineInfo( uintptr Pos ) const
	{
		if( m_Lines.IsEmpty() ) {
			AX_ASSERT_MSG( false, "Should not calculate line info on empty or unprocessed source" );

			SLineInfo Info;

			Info.pFilename = nullptr;
			Info.Line = 0;
			Info.Column = 0;

			return Info;
		}

		// Check for #line directives first
		for( const SLineDirective &LineDirective : m_LineDirectives ) {
			if( Pos < LineDirective.EnterPos || Pos >= LineDirective.LeavePos ) {
				continue;
			}

			SLineInfo Info;

			Info.pFilename = &LineDirective.Filename;
			Info.Line = LineDirective.Line;
			Info.Column = 0;

			return Info;
		}

		// Find the latest line number
		uintptr i =  0;
		while( i < m_Lines.Num() ) {
			const uintptr LinePos = m_Lines[ i ];
			if( LinePos > Pos ) {
				break;
			}

			++i;
		}

		if( i == m_Lines.Num() ) {
			--i;
		}

		SLineInfo Info;

		Info.pFilename = &m_Filename;
		Info.Line = i;
		//Info.Column = Pos - m_Lines[ i ] + 1;
		Info.Column = 0; // TODO: Calculate this accurately

		return Info;
	}
	void CSource::Report( ESeverity Sev, const String &Message, uintptr Pos )
	{
		//
		//	TODO: Report up the chain (parent files and such)
		//	TODO: Figure out "function"
		//	TODO: Figure out "from" (which system is reporting)
		//

		const SLineInfo LineInfo = CalculateLineInfo( Pos );

		SReportDetails Details;

		Details.From = 0;
		Details.Severity = Sev;
		Details.pszFile = LineInfo.pFilename != nullptr ? LineInfo.pFilename->CString() : nullptr;
		Details.uLine = uint32( LineInfo.Line );
		Details.uColumn = uint32( LineInfo.Column );
		Details.pszFunction = nullptr;

		Ax::Report( Details, Message );
	}

	uint32 CSource::ProcessLineFlag()
	{
		if( m_bDidCrossLine ) {
			m_bDidCrossLine = false;
			return kTokF_StartsLine;
		}

		return 0;
	}

	bool CSource::SkipWhitespace()
	{
		const char *const pSrc = m_Text.CString();
		if( !pSrc ) {
			return false;
		}

		const char *p = pSrc + m_Current;

		if( m_pProcessor != nullptr ) {
			p = m_pProcessor->SkipWhitespace( p, *this, m_Tokens, m_bDidCrossLine );
		} else {
			while( *p <= ' ' && *p != '\0' ) {
				if( *p == '\n' ) {
					m_bDidCrossLine = true;
				}

				++p;
			}
		}

		const uintptr NewPos = uintptr( p - pSrc );
		const bool bFoundWhitespace = NewPos != m_Current;
		m_Current = NewPos;

		return bFoundWhitespace;
	}
	bool CSource::SkipString()
	{
		const char *const pSrc = m_Text.CString();
		if( !pSrc ) {
			return false;
		}

		const char *p = pSrc + m_Current;

		EStringTokenType Encoding = EStringTokenType::Unqualified;
		String Data;

		if( m_pProcessor != nullptr ) {
			p = m_pProcessor->SkipString( p, m_bDidCrossLine, Encoding, &Data );
		} else {
			if( *p == '\"' ) {
				++p;
				while( *p != '\0' && *p != '\"' ) {
					if( *p == '\n' ) {
						m_bDidCrossLine = true;
					}
					++p;
				}
			}
		}

		const uintptr NewPos = uintptr( p - pSrc );
		const bool bFoundString = NewPos != m_Current;

		if( bFoundString ) {
			TokenIter Tok = m_Tokens.AddTail();
			AX_VERIFY_MSG( Tok != m_Tokens.end(), "Out of memory" );

			Tok->Type = ETokenType::String;
			Tok->StringType = Encoding;
			Tok->Flags = ProcessLineFlag(); // TODO: String flags (kStrTokF_HasEscapes, etc)
			Tok->Qualifier = ENumberTokenQualifier::Unqualified;
			Tok->pKeyword = nullptr;
			Tok->uOffset = m_Current;
			Tok->cLength = NewPos - m_Current;
			Tok->pSource = this;

			// TODO: Do byte swapping if necessary

			Tok->Data.uOffset = m_ProcessedData.Num();
			switch( Encoding ) {
			case EStringTokenType::Unqualified:
			case EStringTokenType::Multibyte:
			case EStringTokenType::UTF8:
				AX_VERIFY_MSG( m_ProcessedData.Append( Data.Len() + 1, ( const uint8 * )Data.CString() ), "Out of memory" );
				break;

			case EStringTokenType::Wide:
			case EStringTokenType::UTF16BE:
			case EStringTokenType::UTF16LE:
				{
					const TArray< uint16 > UTF16Chars = Data.AsUTF16();
					AX_VERIFY_MSG( m_ProcessedData.Append( UTF16Chars.Num()*2, ( const uint8 * )UTF16Chars.Pointer() ), "Out of memory" );
				}
				break;

			case EStringTokenType::UTF32BE:
			case EStringTokenType::UTF32LE:
				{
					const TArray< uint32 > UTF32Chars = Data.AsUTF32();
					AX_VERIFY_MSG( m_ProcessedData.Append( UTF32Chars.Num()*4, ( const uint8 * )UTF32Chars.Pointer() ), "Out of memory" );
				}
				break;
			}
			Tok->Data.cBytes = m_ProcessedData.Num() - Tok->Data.uOffset;
		}

		m_Current = NewPos;
		return bFoundString;
	}
	bool CSource::SkipNumber()
	{
		const char *const pSrc = m_Text.CString();
		if( !pSrc ) {
			return false;
		}

		const char *p = pSrc + m_Current;

		ENumberTokenType NumberType = ENumberTokenType::Integer;
		ENumberTokenQualifier Qualifier = ENumberTokenQualifier::Unqualified;
		uint32 Flags = 0;

		uint64 uLiteral = 0;

		SKeyword *pKeyword = nullptr;
		if( m_pProcessor != nullptr ) {
			p = m_pProcessor->SkipNumber( p, NumberType, Qualifier, Flags, uLiteral );
		} else {
			Flags = kNumTokF_Decimal;
			while( *p >= '0' && *p <= '9' ) {
				++p;
			}
		}

		const uintptr NewPos = uintptr( p - pSrc );
		const bool bFoundNumber = NewPos != m_Current;

		if( bFoundNumber ) {
			TokenIter Tok = m_Tokens.AddTail();
			AX_VERIFY_MSG( Tok != m_Tokens.end(), "Out of memory" );

			Tok->Type = ETokenType::Number;
			Tok->NumberType = NumberType;
			Tok->Flags = Flags | ProcessLineFlag();
			Tok->Qualifier = Qualifier;
			Tok->pKeyword = nullptr;
			Tok->uOffset = m_Current;
			Tok->cLength = NewPos - m_Current;
			Tok->pSource = this;
			Tok->uLiteral = uLiteral;
		}
		
		m_Current = NewPos;
		return bFoundNumber;
	}
	bool CSource::SkipIdentifier()
	{
		const char *const pSrc = m_Text.CString();
		if( !pSrc ) {
			return false;
		}

		const char *p = pSrc + m_Current;

		SKeyword *pKeyword = nullptr;
		if( m_pProcessor != nullptr ) {
			p = m_pProcessor->SkipIdentifier( p, &pKeyword );
		} else {
			if( String::IsNameStart( *p ) ) {
				++p;
				while( String::IsName( *p ) ) {
					++p;
				}
			}
		}

		const uintptr NewPos = uintptr( p - pSrc );
		const bool bFoundIdent = NewPos != m_Current;

		if( bFoundIdent ) {
			TokenIter Tok = m_Tokens.AddTail();
			AX_VERIFY_MSG( Tok != m_Tokens.end(), "Out of memory" );

			Tok->Type = ETokenType::Name;
			Tok->NameType = pKeyword != nullptr ? ENameTokenType::Keyword : ENameTokenType::User;
			Tok->Flags = ProcessLineFlag();
			Tok->Qualifier = ENumberTokenQualifier::Unqualified;
			Tok->pKeyword = pKeyword;
			Tok->uOffset = m_Current;
			Tok->cLength = NewPos - m_Current;
			Tok->pSource = this;
		}
		
		m_Current = NewPos;
		return bFoundIdent;
	}
	bool CSource::SkipPunctuation()
	{
		const char *const pSrc = m_Text.CString();
		if( !pSrc ) {
			return false;
		}

		const char *p = pSrc + m_Current;

		if( m_pProcessor != nullptr ) {
			p = m_pProcessor->SkipPunctuation( p );
		} else {
			if( strchr( "~!@#$%^&*()-+=[]{}\\|;:<>,./?" , *p ) != nullptr ) {
				++p;
			}
		}

		const uintptr NewPos = uintptr( p - pSrc );
		const bool bFoundPunct = NewPos != m_Current;

		if( bFoundPunct ) {
			TokenIter Tok = m_Tokens.AddTail();
			AX_VERIFY_MSG( Tok != m_Tokens.end(), "Out of memory" );

			Tok->Type = ETokenType::Punctuation;
			Tok->Flags = ProcessLineFlag();
			Tok->Qualifier = ENumberTokenQualifier::Unqualified;
			Tok->pKeyword = nullptr;
			Tok->uOffset = m_Current;
			Tok->cLength = NewPos - m_Current;
			Tok->pSource = this;

			if( m_pProcessor != nullptr ) {
				const String &LineSep = m_pProcessor->GetLineSeparator();
				if( !LineSep.IsEmpty() && Tok->Cmp( LineSep ) ) {
					m_bDidCrossLine = true;
				}
			}
		}
		
		m_Current = NewPos;
		return bFoundPunct;
	}

}}

#pragma once

#include "../Core/Types.hpp"
#include "../Core/String.hpp"
#include "../Core/Logger.hpp"

#include "../Collections/Dictionary.hpp"
#include "../Collections/List.hpp"
#include "../Collections/Array.hpp"

#include "Token.hpp"

//
//	TODO: Allow user to provide their own dictionary look-up system
//	-     This is desired for Tenshi as the scope system is a bit complex
//

namespace Ax { namespace Parser {

	// NOTE: \xF4 is used to encode 4-digit unicode values (e.g., \xF45EF0)
	// NOTE: \xF5 is used to encode 5-digit unicode values (e.g., \xF5A1234)
#define AX_PARSER_DICTIONARY_DEFAULT\
	"abcdefghijklmnopqrstuvwxyz" \
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
	"_0123456789\xF4\xF5"

	/*

		C Digraphs (since C99)

			<:			[
			:>			]
			<%			{
			%>			}
			%:			#

		C++ Digraphs (sort-of)

			%:%:		##

		C Trigraphs

			??=			#
			??/			\
			??'			^
			??(			[
			??)			]
			??!			|
			??<			{
			??>			}
			??-			~

		C++ Named Operators

			and			&&
			bitor		|
			or			||
			xor			^
			compl		~
			bitand		&
			and_eq		&=
			or_eq		|=
			xor_eq		^=
			not			!
			not_eq		!=

		Pascal Digraphs

			(.			[
			.)			]
			(*			{
			*)			}

		NOTE: Digraphs are handled during tokenization
		-     Trigraphs are handled prior to any other processing


		When Unicode Identifiers Are Enabled
		====================================
		The following characters are acceptable for any identifier, including at the
		start of the identifier:

			- a-z, A-Z
			- U+00A8, U+00AA, U+00AD, U+00AF
			- U+00B2-U+00B5, U+00B7-U+00BA
			- U+00BC-U+00BE, U+00C0-U+00D6, U+00D8-U+00F6, U+00F8-U+00FF
			- U+0100-U+02FF, U+0370-U+167F, U+1681-U+180D, U+180F-U+1DBF
			- U+1E00-U+1FFF
			- U+200B-U+200D, U+202A-U+202E, U+203F-U+2040, U+2054, U+2060-U+206F
			- U+2070-U+20CF, U+2100-U+218F, U+2460-U+24FF, U+2776-U+2793
			- U+2C00-U+2DFF, U+2E80-U+2FFF
			- U+3004-U+3007, U+3021-U+302F, U+3031-U+303F, U+3040-U+D7FF
			- U+F900-U+FD3D, U+FD40-U+FDCF, U+FDF0-U+FE1F, U+FE30-U+FE44
			- U+FE47-U+FFFD
			- U+10000-U+1FFFD, U+20000-U+2FFFD, U+30000-U+3FFFD, U+40000-U+4FFFD
			- U+50000-U+5FFFD, U+60000-U+6FFFD, U+70000-U+7FFFD, U+80000-U+8FFFD
			- U+90000-U+9FFFD, U+A0000-U+AFFFD, U+B0000-U+BFFFD, U+C0000-U+CFFFD
			- U+D0000-U+DFFFD, U+E0000-U+EFFFD

		In addition to the above, the following characters are acceptable for any part
		of an identifier after the initial character:

			- 0-9
			- U+0300-U+036F, U+1DC0-U+1DFF, U+20D0-U+20FF, U+FE20-U+FE2F

	*/

	// Flags for controlling source tokenization
	enum ESourceFlags : uint32
	{
		// Combine lines separated by the concatenation symbol (default: '\')
		kSrcF_CombineLines			= 1<<0,
		// Expand digraphs into their corresponding symbols (e.g., '%:' -> '#')
		kSrcF_ExpandDigraphs		= 1<<1,
		// Expand trigraphs into their corresponding symbols (e.g., '??^' -> something)
		kSrcF_ExpandTrigraphs		= 1<<2,
		// Expand separate quotes into one string (e.g., "Hello " "world" -> "Hello world")
		kSrcF_CombineQuotes			= 1<<3,
		// Expand string escape sequences
		kSrcF_StringEscapes			= 1<<4,
		// Support <n>x notations, e.g., 16xA, 8x12, 2x1010
		kSrcF_ArbitraryBases		= 1<<5,
		// Support Unicode identifiers
		kSrcF_UnicodeNames			= 1<<6,
		// Support path-styled identifiers (e.g., enemy/grue is a single token)
		kSrcF_PathstyleNames		= 1<<7,
		// Generate tokens for comments
		kSrcF_KeepComments			= 1<<8,
		// Expand named operators into their corresponding symbols (e.g., "and" -> "&&")
		kSrcF_NamedOperators		= 1<<9,
		// Support spaces in identifier names (e.g., `make cube` is a single token)
		kSrcF_SpacedNames			= 1<<10,
		// Support use of a character (default "`") to escape keywords (e.g., `if`) 
		kSrcF_KeywordEscapes		= 1<<11,
		// Expand Swift-style in-string expressions (e.g., "Hello \(name)" -> "Hello " + ( name ))
		kSrcF_StringExpressions		= 1<<12,
		// Use 0c for octal notation instead of 0<number>
		kSrcF_BasicOctals			= 1<<13,
		// Use % for binary notation in addition to 0b<number>
		kSrcF_BasicBinaries			= 1<<14,
		// When combining lines (kSrcF_CombineLines) do not do so within comments
		kSrcF_NoCommentLineConcat	= 1<<15,
		// Do not treat 'x' as a character string
		kSrcF_NoCharStrings			= 1<<16,
		// Expand string escapes only if an escape prefix is given (default: '~')
		kSrcF_StringEscapesPrefixed	= 1<<17,
		// Enable REM, REMSTART, and REMEND comments
		kSrcF_RemComments			= 1<<18,
		// Strings can select their encoding with L (wide), u8 (UTF-8), u (UTF-16), or U (UTF-32)
		kSrcF_CStringEncodings		= 1<<19,
		// Strings can have line breaks midstring
		kSrcF_StringLineBreaks		= 1<<20,
		// Allow any of the set characters (default: "#$") to be the ending of an identifier
		kSrcF_SpecialEndings		= 1<<21,
		// Named operators are case insensitive (e.g., "and" is the same as "AND" and "AnD"; all of which are "&&")
		kSrcF_CaselessNamedOps		= 1<<22,
		// Use $ for hexadecimal notation in addition to 0x<number>
		kSrcF_BasicHexadecimals		= 1<<23,
		// Do not allow qualifiers on numbers (e.g., 1f)
		kSrcF_NoNumberQualifiers	= 1<<24
	};

	// Type of the source (where it's from)
	enum class ESourceType
	{
		// Source comes from a file (either on disk or through the network)
		File,
		// Source comes from RAM (unknown origin)
		Memory,
		// Source comes from a macro expansion
		MacroExpansion,
		// Source is the result of kSrcF_CombineQuotes, kSrcF_StringEscapes, or kSrcF_StringExpressions
		StringExpansion,
		// Source is the result of kSrcF_ExpandDigraphs
		DigraphExpansion
	};

	// String escape mode
	enum class EStringEscape : uint8
	{
		// This sequence does nothing
		Disabled,
		// A simple character substitution
		Replace,
		// Byte value insertion (for \xFF)
		InsertByteValue,
		// Unicode 2-digit sequence
		InsertUnicode2,
		// Unicode 4-digit sequence (for \u)
		InsertUnicode4,
		// Unicode 5-digit sequence
		InsertUnicode5,
		// Unicode 8-digit sequence (for \U)
		InsertUnicode8,
		// Unicode variable length sequence (Swift style)
		InsertUnicode,
		// Subexpression sequence (Swift's "John has \(apples) apples")
		Subexpression
	};

	// What to do when a keyword being added already exists
	enum class EKeywordExists
	{
		Error,
		Ignore,
		Overwrite
	};

	class CSourceProcessor;
	class CSource;

	// #line directive entries
	struct SLineDirective
	{
		// Name of the file given
		String						Filename;
		// Line number
		uintptr						Line;
		// Starting position (in the source) of the directive
		uintptr						EnterPos;
		// One character past the end of the directive's range
		uintptr						LeavePos;
	};

	// Line information
	struct SLineInfo
	{
		const String *				pFilename;
		uintptr						Line;
		uintptr						Column;
	};

	// System for processing tokens into other tokens based on configurable information
	class CSourceProcessor
	{
	friend class CSource;
	public:
		// Retrieve the system used by C++ (by default)
		static const CSourceProcessor &GetCxxDefault();

		// Default constructor
		CSourceProcessor( const char *pszAllowed = AX_PARSER_DICTIONARY_DEFAULT, ECase Casing = ECase::Sensitive );
		// Destructor
		~CSourceProcessor();

		// Expand trigraphs
		String ExpandTrigraphs( const String &Source ) const;
		// Process text
		String ProcessText( const String &Source, TArray< uintptr > &OutLines ) const;

		// Turn on a set of flags
		void FlagsOn( uint32 Flags );
		// Turn off a set of flags
		void FlagsOff( uint32 Flags );
		// Check if any of the flags in the given set are on
		bool AnyFlagsOn( uint32 Flags ) const;
		// Check if all of the flags in the given set are on
		bool AllFlagsOn( uint32 Flags ) const;

		// Add punctuation (delimiters / operators) to be treated as such
		void AddOperators( const char *const *ppszOperators, uintptr cOperators );
		template< uint32 tNumOperators >
		inline void AddOperators( const char *const( &ppszOperators )[ tNumOperators ] )
		{
			AddOperators( ppszOperators, tNumOperators );
		}
		inline void AddOperator( const char *pszOperator )
		{
			AddOperators( &pszOperator, 1 );
		}

		// Add recognized (core) keywords
		void AddKeywords( const char *const *ppszKeywords, uintptr cKeywords, EKeywordExists Mode = EKeywordExists::Error );
		void AddKeywords( const SKeyword *pKeywords, uintptr cKeywords, EKeywordExists Mode = EKeywordExists::Error );
		template< uint32 tNumKeywords >
		inline void AddKeywords( const char *const( &ppszKeywords )[ tNumKeywords ], EKeywordExists Mode = EKeywordExists::Error )
		{
			AddKeywords( ppszKeywords, tNumKeywords, Mode );
		}
		template< uint32 tNumKeywords >
		inline void AddKeywords( const SKeyword( &Keywords )[ tNumKeywords ], EKeywordExists Mode = EKeywordExists::Error )
		{
			AddKeywords( Keywords, tNumKeywords, Mode );
		}
		inline void AddKeyword( const char *pszKeyword, EKeywordExists Mode = EKeywordExists::Error )
		{
			AddKeywords( &pszKeyword, 1, Mode );
		}
		inline void AddKeyword( const SKeyword &Keyword, EKeywordExists Mode = EKeywordExists::Error )
		{
			AddKeywords( &Keyword, 1, Mode );
		}
		// Find a keyword
		SKeyword *FindKeyword( const char *pszKeyword ) const;

		// Add a digraph to be processed (e.g., "%:" -> "#")
		void AddDigraph( const char *pszFind, const char *pszReplace );
		// Add a trigraph to be processed (e.g., "??=" -> "#")
		void AddTrigraph( char chFind, char chReplace );
		// Add a named operator to be processed (e.g., "and" -> "&&")
		void AddNamedOperator( const char *pszFind, const char *pszReplace );

		// Use C-style line concatenation (the '\' symbol)
		void UseCLineCat();

		// Add unextended C89 operators
		void AddCOperators();
		// Add unextended C++98 operators
		void AddCxxOperators();

		// Add unextended C89 preprocessor operators ('#' and '##')
		void AddCPreprocessorOperators();

		// Add C99 digraphs
		void AddC99Digraphs();
		// Add C++ digraphs ('%:%:' -> '##')
		void AddCxxDigraphs();
		// Add C++ named operators
		void AddCxxNamedOperators();
		// Add Pascal digraphs
		void AddPascalDigraphs();

		// Add C89 trigraphs
		void AddCTrigraphs();

		// Add C89 keywords
		void AddCKeywords();
		// Add C99 keywords
		void AddC99Keywords();
		// Add C++98 keywords
		void AddCxxKeywords();
		// Add C++11 keywords
		void AddCxx11Keywords();

		// Add GNU C keywords
		void AddGNUCKeywords();
		// Add MSVC++ keywords
		void AddMSVCKeywords();

		// Restore all escapes
		void ResetEscapes();
		// Set a specific string escape sequence
		void SetStringEscape( char chEscapeCode, char chReplaceCharacter );
		// Set a string escape sequence to a specific mode
		void SetStringEscapeMode( char chEscapeCode, EStringEscape Mode );
		// Set the string escape character
		void SetStringEscapeChar( char chEscape, char chEscapePrefix = '\0' );

		// Enable C++ string escapes
		void UseCStringEscapes();
		// Enable Blitz string escapes
		void UseBlitzEscapes();

		// Set special identifier ending characters (e.g., "#$" for Basic-like languages)
		void SetSpecialEndChars( const char *pszEndChars );

		// Set a line concatenator
		void SetLineConcat( const char *pszConcat );
		// Add a single-line comment (e.g., "//")
		void AddLineComment( const char *pszLineComment );
		// Add a multi-line comment (e.g., "/*" "*/")
		void AddMultiComment( const char *pszOpen, const char *pszClose );
		// Add a multi-line nestable comment (e.g., "--[[" "--]]")
		void AddNestComment( const char *pszOpen, const char *pszClose );

		// Set the digit separator character
		void SetDigitSeparator( char chDigitSep );

		// Set the line separator
		void SetLineSeparator( const char *pszLineSep );
		// Retrieve the line separator
		const String &GetLineSeparator() const;

	protected:
		const char *SkipWhitespace( const char *pszSource, CSource &SourceObj, TokenList &Tokens, bool &bOutDidCrossLine ) const;
		const char *SkipString( const char *pszSource, bool &bOutDidCrossLine, EStringTokenType &OutEncoding, String *pOutData ) const;
		const char *SkipIdentifier( const char *pszSource, SKeyword **ppOutKeyword = nullptr ) const;
		const char *SkipNumber( const char *pszSource, ENumberTokenType &OutType, ENumberTokenQualifier &OutQualifier, uint32 &OutFlags, uint64 &OutData ) const;
		const char *SkipPunctuation( const char *pszSource ) const;

	private:
		typedef TDictionary< SKeyword >::SEntry SKeywordEntry;
		struct SExpansion
		{
			String					Find;
			String					Replace;
		};
		struct SMultilineComment
		{
			String					Enter;
			String					Leave;
			bool					bNests;
		};

		String						m_LineCat;
		String						m_LineSep;

		String						m_SpecialEndings;

		TArray< String >			m_Punctuation;
		TArray< SExpansion >		m_Digraphs;
		char						m_TrigraphMap		[ 0x7F - 0x21 ];
		TArray< SExpansion >		m_NamedOperators;
		TDictionary< SKeyword >		m_Keywords;
		TArray< SKeywordEntry * >	m_KeywordEntries;
		TArray< String >			m_SingleLineComments;
		TArray< SMultilineComment >	m_MultiLineComments;
		char						m_StringEscapeMap	[ 0x7F - 0x21 ];
		EStringEscape				m_StringEscapeMode	[ 0x7F - 0x21 ];
		char						m_StringEscapeChar;
		char						m_StringEscapePrefixChar;
		char						m_DigitSeparatorChar;
		uint32						m_Flags;

		static const char *SkipWhitespace( const char *pszSrc, bool &bOutDidCrossLine );
		static const char *SkipLine( const char *pszSrc, bool &bOutDidCrossLine );
		static const char *SkipNestedSection( const char *pszSrc, const SMultilineComment &Section, bool &bOutDidCrossLine, ECase Casing = ECase::Sensitive );
		static const char *SkipName( const char *pszSrc, bool bSupportUnicode, const char *pszSpecialEndings, char *&pszEntryBuf, uintptr &cEntryBufBytes );

		// Copy constructor
		CSourceProcessor( const CSourceProcessor & ) AX_DELETE_FUNC;
		// Assignment operator
		CSourceProcessor &operator=( const CSourceProcessor & ) AX_DELETE_FUNC;
	};

	// Single source file
	class CSource
	{
	public:
		static const uintptr		kDefaultPosition = ~uintptr( 0 );

		// Default constructor
		CSource();
		// Destructor
		~CSource();

		// Set the source processor
		void SetProcessor( const CSourceProcessor &SrcProc );

		// Set the text of this source
		void SetText( const String &Filename, const String &Text, ESourceType Type = ESourceType::File );
		// Retrieve the name of the file
		const char *GetFilename() const;
		// Retrieve the type of this source
		ESourceType GetType() const;

		// Retrieve a direct pointer to the source data
		const char *GetText() const;
		// Retrieve the data array (retrieves encoded literals, such as strings, floats, etc)
		const TArray< uint8 > &GetData() const;

		// Do a line directive
		void LineDirective( const String &Name, uintptr LineNumber );

		// Read a token from the source
		TokenIter ReadToken();
		// Unread the last token read
		void UnreadToken();
		// Jump back to a specific token in the list
		void SetCurrentToken( TokenIter Tok );
		// Retrieve the current token without parsing
		TokenIter GetCurrentToken() const;

		// Skip the line we're currently on
		void SkipLine();

		// Retrieve line information for the position given
		SLineInfo CalculateLineInfo( uintptr Pos = kDefaultPosition ) const;
		// Submit a report to the logger
		void Report( ESeverity Sev, const String &Message, uintptr Pos = kDefaultPosition );
		// Submit an error
		void Error( const String &Message, uintptr Pos = kDefaultPosition );
		// Submit a warning
		void Warn( const String &Message, uintptr Pos = kDefaultPosition );

	private:
		// Where this source came from
		ESourceType					m_Type;
		// Stores the name of the given file
		String						m_Filename;
		// Parent source file
		CSource *					m_pParent;
		// Where this file was included from in the parent
		uintptr						m_ParentIncludePos;
		// Source processor associated with this
		const CSourceProcessor *	m_pProcessor;
		// Created (and owned) source files
		TList< CSource >			m_ExpandedSources;
		// The primary text of the source
		String						m_Text;
		// Current position within the source
		uintptr						m_Current;
		// Position of each line (each index in the array corresponds to a line number with each item being the position)
		TArray< uintptr >			m_Lines;
		// Ranges of each #line directive (this includes the filename and line)
		TList< SLineDirective >		m_LineDirectives;
		// All of the tokens read so far
		TokenList					m_Tokens;
		// Current position in the token list
		TokenIter					m_CurrentToken;
		// Character used to escape keywords (default is `)
		char						m_EscapeKeywordChar;
		// Character used to indicate an escaped string (default is ~)
		char						m_EscapeStringChar;
		// Whether any lines have been crossed since the last token was added
		bool						m_bDidCrossLine;
		// Processed data for this source file
		TArray< uint8 >				m_ProcessedData;

		// Process the current starts-line flag
		uint32 ProcessLineFlag();

		// Skip whitespace and comments
		//
		// If kSrcF_KeepComments is set then a comment token will be created
		//
		// Returns true if whitespace was skipped, and false if not
		bool SkipWhitespace();
		// Skip strings
		//
		// This will also process the string data as it goes
		//
		// Returns true if a string was processed, and false if not
		bool SkipString();
		// Skip numbers
		//
		// Encodes the number into the internal data if it's found
		//
		// Returns true if a number was processed, and false if not
		bool SkipNumber();
		// Skip identifiers
		//
		// This checks for keywords as well (if a processor is set)
		//
		// If kSrcF_SpacedNames is set then the longest keyword registered with
		// the processor found in the text will be stored as a single token with
		// the keyword pointer set. Otherwise, there will be separate tokens
		//
		// Returns true if an identifier was skipped, and false if not
		bool SkipIdentifier();
		// Skip punctuation
		//
		// This does not check for named punctuation (e.g., "and")
		//
		// Returns true if punctuation was skipped, and false if not
		bool SkipPunctuation();
	};

	inline const char *CSource::GetFilename() const
	{
		return m_Filename.CString();
	}
	inline ESourceType CSource::GetType() const
	{
		return m_Type;
	}

	inline const char *CSource::GetText() const
	{
		return m_Text.CString();
	}
	inline const TArray< uint8 > &CSource::GetData() const
	{
		return m_ProcessedData;
	}

	inline TokenIter CSource::GetCurrentToken() const
	{
		return m_CurrentToken;
	}

	inline void CSource::Error( const String &Message, uintptr Pos )
	{
		Report( ESeverity::Error, Message, Pos );
	}
	inline void CSource::Warn( const String &Message, uintptr Pos )
	{
		Report( ESeverity::Warning, Message, Pos );
	}

}}

#pragma once

#include "../Core/Logger.hpp"
#include "../Core/Types.hpp"
#include "../Core/String.hpp"

#include "../Collections/List.hpp"

namespace Ax { namespace Parser {

	class CSource;

	// Keyword or command identification
	struct SKeyword
	{
		const char *const			pszName;
		const uint32				Identifier;
		void *						Data;

		inline SKeyword( const char *pszName, uint32 Identifier = 0, void *Data = nullptr )
		: pszName( pszName )
		, Identifier( Identifier )
		, Data( Data )
		{
		}
		inline SKeyword( const SKeyword &Keyword )
		: pszName( Keyword.pszName )
		, Identifier( Keyword.Identifier )
		, Data( Keyword.Data )
		{
		}

		inline bool Is( uint32 CheckIdentifier ) const
		{
			return Identifier == CheckIdentifier;
		}

		template< typename tItem >
		inline const tItem *As() const
		{
			return reinterpret_cast< tItem * >( Data );
		}
		template< typename tItem >
		inline tItem *As()
		{
			return reinterpret_cast< tItem * >( Data );
		}
	};

	enum class ETokenType : uint8
	{
		None,

		Comment,

		Name,
		Number,
		String,
		Punctuation
	};

	enum class ECommentTokenType : uint8
	{
		Normal,
		Document
	};

	enum class ENameTokenType : uint8
	{
		User,
		Keyword
	};
	enum class ENumberTokenType : uint8
	{
		Integer,
		Real
	};
	enum class EStringTokenType : uint8
	{
		Unqualified,
		Multibyte,
		Wide,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE
	};

	enum ECommentTokenFlags : uint8
	{
	};
	enum ENameTokenFlags : uint8
	{
	};
	enum ENumberTokenFlags : uint8
	{
		kNumTokF_Decimal			= 1<<0,
		kNumTokF_Hexadecimal		= 1<<1,
		kNumTokF_Octal				= 1<<2,
		kNumTokF_Binary				= 1<<3,
		kNumTokF_HasDot				= 1<<4,
		kNumTokF_SciNot				= 1<<5,
		kNumTokF_ArbitraryBase		= 1<<6
	};
	enum class ENumberTokenQualifier : uint8
	{
		Unqualified,
		Long,
		LongLong,
		Unsigned,
		UnsignedLong,
		UnsignedLongLong,
		Float,
		SignedInt8,
		SignedInt16,
		SignedInt32,
		SignedInt64,
		UnsignedInt8,
		UnsignedInt16,
		UnsignedInt32,
		UnsignedInt64
	};
	enum EStringTokenFlags : uint8
	{
		kStrTokF_Raw				= 1<<0,
		kStrTokF_HasEscapes			= 1<<1,
		kStrTokF_HasExpressions		= 1<<2
	};

	enum EGeneralTokenFlags : uint8
	{
		// Token is at start of line
		kTokF_StartsLine			= 1<<7
	};

	inline const char *TokenTypeToString( ETokenType Type )
	{
		switch( Type )
		{
		case ETokenType::None:
			return "None";

		case ETokenType::Comment:
			return "Comment";

		case ETokenType::Name:
			return "Name";

		case ETokenType::Number:
			return "Number";

		case ETokenType::String:
			return "String";

		case ETokenType::Punctuation:
			return "Punctuation";
		}

		return "(unknown)";
	}

	struct SToken
	{
		// How the token is classified
		ETokenType					Type;
		union {
			uint8					Subtype;
			ECommentTokenType		CommentType;
			ENameTokenType			NameType;
			ENumberTokenType		NumberType;
			EStringTokenType		StringType;
		};
		// Flags depending on the token type
		uint8						Flags;
		// Additional qualifier information (for the number)
		ENumberTokenQualifier		Qualifier;
		// Pointer to the keyword if this has a keyword
		const SKeyword *			pKeyword;
		// Offset into the stream where the token was found
		uintptr						uOffset;
		// Length of the token
		uintptr						cLength;
		// Where the token came from
		CSource *					pSource;
		union
		{
			// Processed data (for strings)
			struct
			{
				// Processed data start
				uintptr				uOffset;
				// Processed data length
				uintptr				cBytes;
			}						Data;

			// Encoded literals (for numbers)
			uint64					uLiteral;
		};

		inline SToken()
		: Type( ETokenType::None )
		, Subtype( 0 )
		, Flags( 0 )
		, Qualifier( ENumberTokenQualifier::Unqualified )
		, pKeyword( nullptr )
		, uOffset( 0 )
		, cLength( 0 )
		, pSource( nullptr )
		{
			Data.uOffset = 0;
			Data.cBytes = 0;
		}
		inline SToken( const SToken &Other )
		: Type( Other.Type )
		, Subtype( Other.Subtype )
		, Flags( Other.Flags )
		, Qualifier( Other.Qualifier )
		, pKeyword( Other.pKeyword )
		, uOffset( Other.uOffset )
		, cLength( Other.cLength )
		, pSource( Other.pSource )
		{
			Data.uOffset = Other.Data.uOffset;
			Data.cBytes = Other.Data.cBytes;
		}

		inline SToken &operator=( const SToken &Other )
		{
			Type = Other.Type;
			Subtype = Other.Subtype;
			Flags = Other.Flags;
			Qualifier = Other.Qualifier;
			pKeyword = Other.pKeyword;
			uOffset = Other.uOffset;
			cLength = Other.cLength;
			pSource = Other.pSource;
			Data.uOffset = Other.Data.uOffset;
			Data.cBytes = Other.Data.cBytes;

			return *this;
		}

		inline bool Is( ETokenType TokenType ) const
		{
			return Type == TokenType;
		}
		inline bool IsComment() const
		{
			return Type == ETokenType::Comment;
		}
		inline bool IsComment( ECommentTokenType commentSubtype ) const
		{
			return Type == ETokenType::Comment && CommentType == commentSubtype;
		}
		inline bool IsDocComment() const
		{
			return Type == ETokenType::Comment && CommentType == ECommentTokenType::Document;
		}
		inline bool IsName() const
		{
			return Type == ETokenType::Name;
		}
		inline bool IsName( ENameTokenType nameSubtype )
		{
			return Type == ETokenType::Name && NameType == nameSubtype;
		}
		inline bool IsKeyword() const
		{
			return Type == ETokenType::Name && NameType == ENameTokenType::Keyword;
		}
		inline bool IsKeyword( const char *pszName ) const
		{
			return Type == ETokenType::Name && NameType == ENameTokenType::Keyword && Compare( pszName );
		}
		inline bool IsKeyword( const uint32 Ident ) const
		{
			return Type == ETokenType::Name && NameType == ENameTokenType::Keyword && pKeyword != nullptr && pKeyword->Identifier == Ident;
		}
		inline bool IsNumber() const
		{
			return Type == ETokenType::Number;
		}
		inline bool IsString() const
		{
			return Type == ETokenType::String;
		}
		inline bool IsPunctuation() const
		{
			return Type == ETokenType::Punctuation;
		}
		inline bool IsPunctuation( const char *pszPunctuation ) const
		{
			return Type == ETokenType::Punctuation && Compare( pszPunctuation );
		}

		inline bool IsLiteral() const
		{
			return Type == ETokenType::Number || Type == ETokenType::String;
		}

		inline bool IsPunctuation( const char *pszFirst, const char *pszSecond ) const
		{
			return Type == ETokenType::Punctuation && ( Compare( pszFirst ) || Compare( pszSecond ) );
		}
		inline bool IsPunctuation( const char *pszFirst, const char *pszSecond, const char *pszThird ) const
		{
			return Type == ETokenType::Punctuation && ( Compare( pszFirst ) || Compare( pszSecond ) || Compare( pszThird ) );
		}

		template< uintptr tNumTests >
		inline bool IsPunctuation( const char *const( &pszTests )[ tNumTests ] ) const
		{
			if( Type != ETokenType::Punctuation ) {
				return false;
			}

			for( uintptr i = 0; i < tNumTests; ++i ) {
				if( Compare( pszTests[ i ] ) ) {
					return true;
				}
			}

			return false;
		}

		inline bool Any( uint8 TestFlags ) const
		{
			return ( Flags & TestFlags ) != 0;
		}
		inline bool All( uint8 TestFlags ) const
		{
			return ( Flags & TestFlags ) == TestFlags;
		}
		inline bool None( uint8 TestFlags ) const
		{
			return ( Flags & TestFlags ) == 0;
		}

		inline bool StartsLine() const
		{
			return ( Flags & kTokF_StartsLine ) == kTokF_StartsLine;
		}

		inline EEncoding GetStringEncoding() const
		{
			if( Type != ETokenType::String ) {
				return EEncoding::Unknown;
			}

			switch( StringType ) {
			case EStringTokenType::Multibyte:
				return EEncoding::Unknown;

			case EStringTokenType::Unqualified:
			case EStringTokenType::UTF8:
				return EEncoding::UTF8;

			case EStringTokenType::Wide:
			case EStringTokenType::UTF16LE:
				return EEncoding::UTF16_LE;
			case EStringTokenType::UTF16BE:
				return EEncoding::UTF16_BE;

			case EStringTokenType::UTF32BE:
				return EEncoding::UTF32_BE;
			case EStringTokenType::UTF32LE:
				return EEncoding::UTF32_LE;
			}

			return EEncoding::Unknown;
		}
		TArray< uint8 > GetEncodedString() const;
		inline String GetUnencodedString() const
		{
			return String::FromEncoding( GetEncodedString(), GetStringEncoding() );
		}

		inline uint64 GetEncodedUnsigned() const
		{
			return uLiteral;
		}
		inline int64 GetEncodedSigned() const
		{
			return *( const int64 * )&uLiteral;
		}
		inline double GetEncodedFloat() const
		{
			return *( const double * )&uLiteral;
		}

		inline bool IsCPPDirective() const
		{
			return StartsLine() && IsPunctuation( "#" );
		}

		bool Compare( const String &Other ) const;
		bool CaseCompare( const String &Other ) const;

		bool Compare( const char *pszOther ) const;
		bool CaseCompare( const char *pszOther ) const;

		inline bool Cmp( const String &Other ) const
		{
			return Compare( Other );
		}
		inline bool CaseCmp( const String &Other ) const
		{
			return CaseCompare( Other );
		}

		inline bool Cmp( const char *pszOther ) const
		{
			return Compare( pszOther );
		}
		inline bool CaseCmp( const char *pszOther ) const
		{
			return CaseCompare( pszOther );
		}

		inline bool operator==( const String &Other ) const
		{
			return Compare( Other );
		}
		inline bool operator==( const char *pszOther ) const
		{
			return Compare( pszOther );
		}
		inline bool operator!=( const String &Other ) const
		{
			return !Compare( Other );
		}
		inline bool operator!=( const char *pszOther ) const
		{
			return !Compare( pszOther );
		}

		inline bool operator!() const
		{
			return cLength == 0;
		}
		inline operator bool() const
		{
			return cLength > 0 && Type != ETokenType::None;
		}

		// Get the string of this token
		String GetString() const;
		// Retrieve the direct text of this token from the source
		const char *GetPointer() const;

		// Submit a report to the logger
		void Report( ESeverity Sev, const String &Message ) const;
		// Submit an error
		void Error( const String &Message ) const;
		// Submit a warning
		void Warn( const String &Message ) const;
	};

	typedef TList< SToken >			TokenList;
	typedef TokenList::Iterator		TokenIter;

	inline void SToken::Error( const String &Message ) const
	{
		Report( ESeverity::Error, Message );
	}
	inline void SToken::Warn( const String &Message ) const
	{
		Report( ESeverity::Warning, Message );
	}

	inline String SToken::GetString() const
	{
		const char *const pszText = GetPointer();
		if( !pszText ) {
			return String();
		}

		return String( pszText, intptr( cLength ) );
	}

}}

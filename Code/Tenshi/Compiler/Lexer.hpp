#pragma once

#include <Parser/Source.hpp>
#include "ParserConfig.hpp"

namespace Tenshi { namespace Compiler {

	typedef Ax::Parser::SToken		SToken;

	enum class ETokenPlace
	{
		Irrelevant,
		SameLine,
		NewLine
	};

	//
	//	TODO: Add lexing modes
	//	-     normal: Same operation as currently
	//	-     line: Lex() will continue returning tokens on the current line
	//	...         until the end of the line is reached. Then AdvanceLine()
	//	...         will need to be called
	//	-     then: Same as "line" but allows virtual lines (caused by ":") to
	//	...         go through
	//

	class CLexer
	{
	public:
		CLexer();
		~CLexer();

		void SetDebugPrint( bool bDebugPrintTokens );
		bool IsDebugPrinting() const;

		bool LoadFile( const char *pszFilename, Ax::EEncoding Encoding = Ax::EEncoding::Unknown );
		bool LoadText( const char *pszFilename, const Ax::String &FileText );

		// Read a new token from the stream
		const SToken &Lex();
		// Retrieve the current token (the token last pointed to by Lex() or Unlex())
		const SToken &Token();
		// Unread the last token from the stream so the next call to Lex() will return it again
		void Unlex();

		// Read the next token on the current line
		//
		// If the next token is not on the current line then it will not be
		// returned (but will be made available for the next Lex() call)
		const SToken &LexLine();
		
		void Report( Ax::ESeverity Sev, const Ax::String &Message );
		void Error( const Ax::String &Message );
		void Warn( const Ax::String &Message );

		void Expected( const Ax::String &What );

		const SToken &ExpectToken();
		const SToken &Expect( ETokenPlace Placement, Ax::uintptr cTypes, const Ax::Parser::ETokenType *pTypes, const char *const *ppValues = nullptr );

		const SToken &Expect( Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &Expect( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &Expect( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );

		template< Ax::uintptr tNum >
		inline const SToken &Expect( const Ax::Parser::ETokenType( &Types )[ tNum ] )
		{
			return Expect( ETokenPlace::Irrelevant, tNum, Types );
		}
		template< Ax::uintptr tNum >
		inline const SToken &Expect( const Ax::Parser::ETokenType( &Types )[ tNum ], const char *const( &pszValues )[ tNum ] )
		{
			return Expect( ETokenPlace::Irrelevant, tNum, Types, pszValues );
		}

		const SToken &Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );

		const SToken &ExpectLine( Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &ExpectLine( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &ExpectLine( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );

		template< Ax::uintptr tNum >
		inline const SToken &Expect( ETokenPlace Placement, const Ax::Parser::ETokenType( &Types )[ tNum ] )
		{
			return Expect( Placement, tNum, Types );
		}
		template< Ax::uintptr tNum >
		inline const SToken &Expect( ETokenPlace Placement, const Ax::Parser::ETokenType( &Types )[ tNum ], const char *const( &pszValues )[ tNum ] )
		{
			return Expect( Placement, tNum, Types, pszValues );
		}

		const SToken &Check( ETokenPlace Placement, Ax::uintptr cTypes, const Ax::Parser::ETokenType *pTypes, const char *const *ppValues = nullptr );

		const SToken &Check( Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &Check( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &Check( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );
		const SToken &Check( Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 );

		template< Ax::uintptr tNum >
		inline const SToken &Check( const Ax::Parser::ETokenType( &Types )[ tNum ] )
		{
			return Check( ETokenPlace::Irrelevant, tNum, Types );
		}
		template< Ax::uintptr tNum >
		inline const SToken &Check( const Ax::Parser::ETokenType( &Types )[ tNum ], const char *const( &pszValues )[ tNum ] )
		{
			return Check( ETokenPlace::Irrelevant, tNum, Types, pszValues );
		}

		const SToken &Check( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &Check( ETokenPlace Placement, Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &Check( ETokenPlace Placement, Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );
		const SToken &Check( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 );

		const SToken &CheckLine( Ax::Parser::ETokenType Type, const char *pszValue = nullptr );
		const SToken &CheckLine( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 );
		const SToken &CheckLine( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 = nullptr );
		const SToken &CheckLine( Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 );

		template< Ax::uintptr tNum >
		inline const SToken &Check( ETokenPlace Placement, const Ax::Parser::ETokenType( &Types )[ tNum ] )
		{
			return Check( Placement, tNum, Types );
		}
		template< Ax::uintptr tNum >
		inline const SToken &Check( ETokenPlace Placement, const Ax::Parser::ETokenType( &Types )[ tNum ], const char *const( &pszValues )[ tNum ] )
		{
			return Check( Placement, tNum, Types, pszValues );
		}

		const SToken &CheckKeyword( ETokenPlace Placement, Ax::uintptr cValues, const Ax::uint32 *pValues );

		inline const SToken &CheckKeyword( Ax::uint32 KeywordIdentifier )
		{
			return CheckKeyword( ETokenPlace::Irrelevant, 1, &KeywordIdentifier );
		}
		template< Ax::uintptr tNum >
		inline const SToken &CheckKeyword( const Ax::uint32( &KeywordIds )[ tNum ] )
		{
			return CheckKeyword( ETokenPlace::Irrelevant, tNum, KeywordIds );
		}
		inline const SToken &CheckKeyword( ETokenPlace Placement, Ax::uint32 KeywordIdentifier )
		{
			return CheckKeyword( Placement, 1, &KeywordIdentifier );
		}
		inline const SToken &CheckLineKeyword( Ax::uint32 KeywordIdentifier )
		{
			return CheckKeyword( ETokenPlace::SameLine, 1, &KeywordIdentifier );
		}
		template< Ax::uintptr tNum >
		inline const SToken &CheckKeyword( ETokenPlace Placement, const Ax::uint32( &KeywordIds )[ tNum ] )
		{
			return CheckKeyword( Placement, tNum, KeywordIds );
		}
		template< Ax::uintptr tNum >
		inline const SToken &CheckLineKeyword( const Ax::uint32( &KeywordIds )[ tNum ] )
		{
			return CheckKeyword( ETokenPlace::SameLine, tNum, KeywordIds );
		}

	private:
		typedef Ax::TArray< const SToken * > TokenArray;

		Ax::Parser::CSource			m_Source;
		TokenArray					m_Tokens;
		Ax::uintptr					m_CurrentToken;

		Ax::Parser::CSource			m_VirtualSource;
		Ax::TList< SToken >			m_VirtualTokens;

		bool						m_bDebugPrintTokens;
		enum class EIfState
		{
			None,
			HaveIf,
			HaveElseIf,
			HaveThen
		}							m_IfState;
		unsigned					m_cEndifs;

		const SToken &ProcessNamedOperator( const SToken &OpTok );
	};

	inline void CLexer::Error( const Ax::String &Message )
	{
		Report( Ax::ESeverity::Error, Message );
	}
	inline void CLexer::Warn( const Ax::String &Message )
	{
		Report( Ax::ESeverity::Warning, Message );
	}

	inline const SToken &CLexer::Expect( Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Expect( ETokenPlace::Irrelevant, Type, pszValue );
	}
	inline const SToken &CLexer::Expect( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		return Expect( ETokenPlace::Irrelevant, Type1, Type2 );
	}
	inline const SToken &CLexer::Expect( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		return Expect( ETokenPlace::Irrelevant, Type1, pszValue1, Type2, pszValue2 );
	}

	inline const SToken &CLexer::Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Expect( Placement, 1, &Type, &pszValue );
	}
	inline const SToken &CLexer::Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		const Ax::Parser::ETokenType Types[] = {
			Type1,
			Type2
		};

		return Expect( Placement, 2, Types, nullptr );
	}
	inline const SToken &CLexer::Expect( ETokenPlace Placement, Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		const Ax::Parser::ETokenType Types[] = {
			Type1,
			Type2
		};
		const char *const pszValues[] = {
			pszValue1,
			pszValue2
		};

		return Expect( Placement, 2, Types, pszValues );
	}

	inline const SToken &CLexer::Check( Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Check( ETokenPlace::Irrelevant, Type, pszValue );
	}
	inline const SToken &CLexer::Check( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		return Check( ETokenPlace::Irrelevant, Type1, Type2 );
	}
	inline const SToken &CLexer::Check( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		return Check( ETokenPlace::Irrelevant, Type1, pszValue1, Type2, pszValue2 );
	}
	inline const SToken &CLexer::Check( Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 )
	{
		return Check( ETokenPlace::Irrelevant, Type, pszValue1, Type, pszValue2 );
	}

	inline const SToken &CLexer::Check( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Check( Placement, 1, &Type, &pszValue );
	}
	inline const SToken &CLexer::Check( ETokenPlace Placement, Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		const Ax::Parser::ETokenType Types[] = {
			Type1,
			Type2
		};

		return Check( Placement, 2, Types, nullptr );
	}
	inline const SToken &CLexer::Check( ETokenPlace Placement, Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		const Ax::Parser::ETokenType Types[] = {
			Type1,
			Type2
		};
		const char *const pszValues[] = {
			pszValue1,
			pszValue2
		};

		return Check( Placement, 2, Types, pszValues );
	}
	inline const SToken &CLexer::Check( ETokenPlace Placement, Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 )
	{
		const Ax::Parser::ETokenType Types[] = {
			Type,
			Type
		};
		const char *const pszValues[] = {
			pszValue1,
			pszValue2
		};

		return Check( Placement, 2, Types, pszValues );
	}

	inline const SToken &CLexer::ExpectLine( Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Expect( ETokenPlace::SameLine, Type, pszValue );
	}
	inline const SToken &CLexer::ExpectLine( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		return Expect( ETokenPlace::SameLine, Type1, Type2 );
	}
	inline const SToken &CLexer::ExpectLine( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		return Expect( ETokenPlace::SameLine, Type1, pszValue1, Type2, pszValue2 );
	}

	inline const SToken &CLexer::CheckLine( Ax::Parser::ETokenType Type, const char *pszValue )
	{
		return Check( ETokenPlace::SameLine, Type, pszValue );
	}
	inline const SToken &CLexer::CheckLine( Ax::Parser::ETokenType Type1, Ax::Parser::ETokenType Type2 )
	{
		return Check( ETokenPlace::SameLine, Type1, Type2 );
	}
	inline const SToken &CLexer::CheckLine( Ax::Parser::ETokenType Type1, const char *pszValue1, Ax::Parser::ETokenType Type2, const char *pszValue2 )
	{
		return Check( ETokenPlace::SameLine, Type1, pszValue1, Type2, pszValue2 );
	}
	inline const SToken &CLexer::CheckLine( Ax::Parser::ETokenType Type, const char *pszValue1, const char *pszValue2 )
	{
		return Check( ETokenPlace::SameLine, Type, pszValue1, Type, pszValue2 );
	}

}}

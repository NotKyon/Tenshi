#include "_PCH.hpp"
#include "Lexer.hpp"

using namespace Ax;

namespace Tenshi { namespace Compiler {

	CLexer::CLexer()
	: m_Source()
	, m_Tokens()
	, m_CurrentToken( 0 )
	, m_VirtualTokens()
	, m_bDebugPrintTokens( false )
	, m_IfState( EIfState::None )
	, m_cEndifs( 0 )
	{
	}
	CLexer::~CLexer()
	{
	}

	void CLexer::SetDebugPrint( bool bDebugPrintTokens )
	{
		m_bDebugPrintTokens = bDebugPrintTokens;
	}
	bool CLexer::IsDebugPrinting() const
	{
		return m_bDebugPrintTokens;
	}

	bool CLexer::LoadFile( const char *pszFilename, Ax::EEncoding Encoding )
	{
		String FileText;

		if( !System::ReadFile( FileText, pszFilename, Encoding ) ) {
			return false;
		}

		return LoadText( pszFilename, FileText );
	}
	bool CLexer::LoadText( const char *pszFilename, const String &FileText )
	{
		// Speed up token reads by allocating a sufficient amount of reserve
		// space
		//
		// If this operation fails it's not necessarily an error as we don't
		// know how many tokens we'll need yet
		if( !m_Tokens.Reserve( 1 + FileText.Len()/8 ) ) {
			Warnf( pszFilename, "Memory might be too low" );
		}

		m_Source.SetProcessor( GetDefaultSourceProcessor() );
		m_Source.SetText( pszFilename, FileText, Parser::ESourceType::File );

		// This isn't technically a digraph expansion, but it's suitable semantically
		m_VirtualSource.SetText( "$virtual", "!&&||~~%%<<>>~<<~>>&|~endif", Parser::ESourceType::DigraphExpansion );
		m_IfState = EIfState::None;
		m_cEndifs = 0;

		return true;
	}

	static bool NewlineFollows( const Parser::TokenIter Tok )
	{
		AX_ASSERT( !!Tok );

		const Parser::CSource *const pSource = Tok->pSource;
		const char *const pText = pSource->GetText();
		const char *p = pText + Tok->uOffset + Tok->cLength;

		while( *p != '\0' ) {
			if( *p == '\n' ) {
				return true;
			}

			if( *( const unsigned char * )p <= ' ' ) {
				++p;
				continue;
			}

			if( ( *p == 'r' || *p == 'R' ) && CaseCmp( p, "rem", 3 ) && *( const unsigned char * )( p + 3 ) <= ' ' ) {
				return true;
			}
			if( *p == '/' && *( p + 1 ) == '/' ) {
				return true;
			}
			if( *p == '`' ) {
				return true;
			}

			break;
		}

		return false;
	}
	static bool NewlinePrecedes( const Parser::TokenIter Tok )
	{
		AX_ASSERT( !!Tok );

		Parser::TokenIter Other = Tok;
		Other.Retreat();

		if( !Other ) {
			return true;
		}

		return NewlineFollows( Other );
	}
	static bool TokenDirectlyFollows( const Parser::TokenIter TokFirst, const Parser::TokenIter TokSecond )
	{
		AX_ASSERT( !!TokFirst );
		AX_ASSERT( !!TokSecond );

		return TokFirst->uOffset + TokFirst->cLength == TokSecond->uOffset;
	}
	const SToken &CLexer::Lex()
	{
		static const SToken NullTok;

		if( m_CurrentToken < m_Tokens.Num() ) {
			const SToken *pTok = m_Tokens[ m_CurrentToken ];

			AX_ASSERT_NOT_NULL( pTok );
			++m_CurrentToken;

			if( m_bDebugPrintTokens ) {
				Ax::BasicDebugf( "token<%s> from unlex", pTok->GetString().CString() );
			}
			return *pTok;
		}

		const SToken *pFoundTok = nullptr;

		bool bForceStartLine = false;
		while( !pFoundTok ) {
			Parser::TokenIter Tok = m_Source.ReadToken();
			if( !Tok ) {
				pFoundTok = &NullTok;
				break;
			}

			if( bForceStartLine ) {
				Tok->Flags |= Ax::Parser::kTokF_StartsLine;
				bForceStartLine = false;
			}

			const bool bStartsLine = Tok->StartsLine();

			if( bStartsLine && m_IfState == EIfState::HaveThen && NewlinePrecedes( Tok ) ) {
				m_Source.UnreadToken();

				AX_ASSERT( m_cEndifs > 0 );
				if( --m_cEndifs == 0 ) {
					m_IfState = EIfState::None;
				}

				Parser::TokenIter VTok = m_VirtualTokens.AddTail();
				AX_EXPECT_MEMORY( VTok != m_VirtualTokens.end() );

				VTok->Type = Parser::ETokenType::Name;
				VTok->NameType = Parser::ENameTokenType::Keyword;
				VTok->Flags = Ax::Parser::kTokF_StartsLine;
				VTok->Qualifier = Parser::ENumberTokenQualifier::Unqualified;
				VTok->pKeyword = GetEndifKeywordPtr();
				VTok->pSource = &m_VirtualSource;

				VTok->uOffset = 22;
				VTok->cLength = 5;

				pFoundTok = VTok.Get();
				break;
			}

			//
			//	TODO: Preprocessing
			//

			if( Tok->IsPunctuation( "#" ) && bStartsLine && NewlinePrecedes( Tok ) ) {
				Tok->Warn( "Preprocessor directives not yet supported" );
				m_Source.SkipLine();
				continue;
			}

			// Special handling for labels
			if( Tok->IsName() && !Tok->IsKeyword() && bStartsLine ) {
				Parser::TokenIter ColonTok = m_Source.ReadToken();

				// Check for a ':' on the same line that ends the line
				if( !!ColonTok ) {
					if( ColonTok->IsPunctuation( ":" ) ) {
						if( TokenDirectlyFollows( Tok, ColonTok ) && NewlineFollows( ColonTok ) && NewlinePrecedes( Tok ) ) {
							// Keep both tokens (for label declaration)
							AX_EXPECT_MEMORY( m_Tokens.Append( Tok.Get() ) );
							AX_EXPECT_MEMORY( m_Tokens.Append( ColonTok.Get() ) );

							++m_CurrentToken;
							return *Tok.Get();
						}
					} else {
						ColonTok->pSource->UnreadToken();
					}
				}
			}

			// If/Then requires special processing
			//
			// The special processing converts `if x then f(y)` to
			// `if x : f(y) : endif` to simplify the parser.
			//
			// Processing is also necessary to remain semantically equivalent to
			// the Basic idiom of `if x then f(y) : g(z)` in which both the `f`
			// and `g` statements would run only if the expression `x` is true.
			// That is, the "end-statement" operator does not terminate the if-
			// statement.
			if( Tok->IsKeyword( kKeyword_If ) ) {
				m_IfState = EIfState::HaveIf;
			} else if( Tok->IsKeyword( kKeyword_ElseIf ) ) {
				m_IfState = EIfState::HaveElseIf;
			} else if( Tok->IsKeyword( kKeyword_Then ) && ( m_IfState == EIfState::HaveIf || m_IfState == EIfState::HaveElseIf ) ) {
				if( m_IfState == EIfState::HaveIf ) {
					++m_cEndifs;
				}
				bForceStartLine = true;
				m_IfState = EIfState::HaveThen;
				continue;
			}

			// Remove the colon if it's not used as a label
			if( Tok->IsPunctuation( ":" ) ) {
				continue;
			}

			pFoundTok = &ProcessNamedOperator( *Tok.Get() );
		}

		AX_ASSERT_NOT_NULL( pFoundTok );
		AX_EXPECT_MEMORY( m_Tokens.Append( pFoundTok ) );

		++m_CurrentToken;
		if( m_bDebugPrintTokens ) {
			Ax::BasicDebugf( "token<%s>", pFoundTok->GetString().CString() );
		}
		return *pFoundTok;
	}
	const SToken &CLexer::Token()
	{
		if( m_CurrentToken > 0 ) {
			AX_ASSERT_NOT_NULL( m_Tokens[ m_CurrentToken - 1 ] );
			return *m_Tokens[ m_CurrentToken - 1 ];
		}

		static const SToken NullTok;
		return NullTok;
	}
	void CLexer::Unlex()
	{
		if( m_CurrentToken > 0 ) {
			if( m_bDebugPrintTokens ) {
				Ax::BasicDebugf( "unlex(%s)", m_Tokens[ m_CurrentToken - 1 ]->GetString().CString() );
			}
			--m_CurrentToken;
		}
	}
	const SToken &CLexer::LexLine()
	{
		static const SToken NullTok;

		const SToken &Tok = Lex();
		if( Tok.StartsLine() ) {
			Unlex();
			return NullTok;
		}

		return Tok;
	}

	void CLexer::Report( Ax::ESeverity Sev, const Ax::String &Message )
	{
		if( !m_CurrentToken ) {
			m_Source.Report( Sev, Message );
		}

		AX_ASSERT( m_CurrentToken <= m_Tokens.Num() );

		const SToken *const pTok = m_Tokens[ m_CurrentToken - 1 ];
		AX_ASSERT_NOT_NULL( m_Tokens[ m_CurrentToken - 1 ] );

		const_cast< SToken * >( pTok )->Report( Sev, Message );
	}

	void CLexer::Expected( const Ax::String &What )
	{
		String Message;
		
		const SToken &Tok = Token();

		AX_EXPECT_MEMORY( Message.Assign( What ) );
		AX_EXPECT_MEMORY( Message.Append( ", but got " ) );
		if( Tok.IsPunctuation() ) {
			AX_EXPECT_MEMORY( Message.Append( "'" + Tok.GetString() + "'" ) );
		} else if( Tok.IsName() ) {
			AX_EXPECT_MEMORY( Message.Append( "identifier '" + Tok.GetString() + "'" ) );
		} else if( !Tok ) {
			AX_EXPECT_MEMORY( Message.Append( "end-of-file" ) );
		} else {
			AX_EXPECT_MEMORY( Message.Append( Parser::TokenTypeToString( Tok.Type ) ) );
		}

		Error( Message );
	}

	const SToken &CLexer::ExpectToken()
	{
		const SToken &Tok = Lex();
		if( !Tok ) {
			Error( "Expected a token, but got end-of-file" );
		}

		return Tok;
	}
	static bool CheckPlacement( const SToken &Tok, ETokenPlace Placement )
	{
		switch( Placement ) {
		case ETokenPlace::Irrelevant:
			return true;

		case ETokenPlace::SameLine:
			return !Tok.StartsLine();

		case ETokenPlace::NewLine:
			return Tok.StartsLine();
		}

		AX_ASSERT_MSG( false, "Unreachable" );
		return false;
	}
	const SToken &CLexer::Expect( ETokenPlace Placement, Ax::uintptr cTypes, const Ax::Parser::ETokenType *pTypes, const char *const *ppValues )
	{
		static const SToken NullTok;

		AX_ASSERT( cTypes > 0 );
		AX_ASSERT_NOT_NULL( pTypes );

		const SToken &Tok = Lex();
		if( CheckPlacement( Tok, Placement ) ) {
			for( uintptr i = 0; i < cTypes; ++i ) {
				if( Tok.Type != pTypes[ i ] ) {
					continue;
				}

				if( !ppValues || !ppValues[ i ] || Tok.Cmp( ppValues[ i ] ) ) {
					return Tok;
				}
			}
		}

		String ExpectedMsg = "Expected ";
		for( uintptr i = 0; i < cTypes; ++i ) {
			if( i > 0 ) {
				AX_EXPECT_MEMORY( ExpectedMsg.Append( ", " ) );

				if( i + 1 == cTypes ) {
					AX_EXPECT_MEMORY( ExpectedMsg.Append( "or " ) );
				}
			}

			if( ppValues != nullptr && ppValues[ i ] != nullptr ) {
				AX_EXPECT_MEMORY( ExpectedMsg.Append( String( "'" ) + ppValues[ i ] + "'" ) );
			} else {
				AX_EXPECT_MEMORY( ExpectedMsg.Append( Parser::TokenTypeToString( pTypes[ i ] ) ) );
			}
		}
		if( Placement == ETokenPlace::SameLine ) {
			AX_EXPECT_MEMORY( ExpectedMsg.Append( " on same line" ) );
		} else if( Placement == ETokenPlace::NewLine ) {
			AX_EXPECT_MEMORY( ExpectedMsg.Append( " on different line" ) );
		}

		Expected( ExpectedMsg );
		return NullTok;
	}
	const SToken &CLexer::Check( ETokenPlace Placement, Ax::uintptr cTypes, const Ax::Parser::ETokenType *pTypes, const char *const *ppValues )
	{
		static const SToken NullTok;

		AX_ASSERT( cTypes > 0 );
		AX_ASSERT_NOT_NULL( pTypes );

		const SToken &Tok = Lex();
		if( CheckPlacement( Tok, Placement ) ) {
			for( uintptr i = 0; i < cTypes; ++i ) {
				if( Tok.Type != pTypes[ i ] ) {
					continue;
				}

				if( !ppValues || !ppValues[ i ] || Tok.Cmp( ppValues[ i ] ) ) {
					return Tok;
				}
			}
		}

		Unlex();
		return NullTok;
	}
	const SToken &CLexer::CheckKeyword( ETokenPlace Placement, Ax::uintptr cValues, const Ax::uint32 *pValues )
	{
		static const SToken NullTok;

		AX_ASSERT( cValues > 0 );
		AX_ASSERT_NOT_NULL( pValues );

		const SToken &Tok = Lex();
		if( CheckPlacement( Tok, Placement ) ) {
			for( uintptr i = 0; i < cValues; ++i ) {
				if( !Tok.IsKeyword( pValues[ i ] ) ) {
					continue;
				}

				return Tok;
			}
		}

		Unlex();
		return NullTok;
	}

	const SToken &CLexer::ProcessNamedOperator( const SToken &OpTok )
	{
		if( !OpTok.IsKeyword() || !OpTok.pKeyword ) {
			return OpTok;
		}

		const EKeyword Keyword = EKeyword( OpTok.pKeyword->Identifier );
		if( Keyword < kKeyword_Op_Start__ || Keyword >= kKeyword_Op_End__ ) {
			return OpTok;
		}

		Parser::TokenIter Tok = m_VirtualTokens.AddTail();
		AX_EXPECT_MEMORY( Tok != m_VirtualTokens.end() );

		Tok->Type = Parser::ETokenType::Punctuation;
		Tok->Subtype = 0;
		Tok->Flags = OpTok.Flags;
		Tok->Qualifier = Parser::ENumberTokenQualifier::Unqualified;
		Tok->pKeyword = OpTok.pKeyword; //for determining where this came from
		Tok->pSource = &m_VirtualSource;

		// "!&&||~~%%<<>>~<<~>>&|~"
		switch( Keyword )
		{
		case kKeyword_Op_Not:
			Tok->uOffset = 0;
			Tok->cLength = 1;
			break;

		case kKeyword_Op_And:
			Tok->uOffset = 1;
			Tok->cLength = 2;
			break;

		case kKeyword_Op_Or:
			Tok->uOffset = 3;
			Tok->cLength = 2;
			break;

		case kKeyword_Op_Xor:
			Tok->uOffset = 5;
			Tok->cLength = 2;
			break;

		case kKeyword_Op_Mod:
			Tok->uOffset = 7;
			Tok->cLength = 2;
			break;

		case kKeyword_Op_BitwiseLShift:
			Tok->uOffset = 9;
			Tok->cLength = 2;
			break;
		case kKeyword_Op_BitwiseRShift:
			Tok->uOffset = 11;
			Tok->cLength = 2;
			break;

		case kKeyword_Op_BitwiseLRotate:
			Tok->uOffset = 13;
			Tok->cLength = 3;
			break;
		case kKeyword_Op_BitwiseRRotate:
			Tok->uOffset = 16;
			Tok->cLength = 3;
			break;

		case kKeyword_Op_BitwiseAnd:
			Tok->uOffset = 19;
			Tok->cLength = 1;
			break;
		case kKeyword_Op_BitwiseOr:
			Tok->uOffset = 20;
			Tok->cLength = 1;
			break;
		case kKeyword_Op_BitwiseXor:
			Tok->uOffset = 21;
			Tok->cLength = 1;
			break;
		case kKeyword_Op_BitwiseNot:
			Tok->uOffset = 21;
			Tok->cLength = 1;
			break;

		default:
			AX_ASSERT_MSG( false, "Unreachable" );
		}

		return *Tok.Get();
	}

}}

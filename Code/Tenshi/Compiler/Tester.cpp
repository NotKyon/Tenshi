#include "_PCH.hpp"
#include "Tester.hpp"
#include "ParserConfig.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "UDTParser.hpp"
#include "FunctionParser.hpp"
#include "Program.hpp"
#include "CodeGen.hpp"

#include <Core/Logger.hpp>

namespace Tenshi { namespace Compiler {

	CTester &CTester::GetInstance()
	{
		static CTester Instance;
		return Instance;
	}

	CTester::CTester()
	: m_TestFiles()
	{
	}
	CTester::~CTester()
	{
	}

	bool CTester::CollectTests( const Ax::String &Path )
	{
		if( !Ax::System::EnumFileTree( m_TestFiles, Path ) ) {
			Ax::Errorf( Path, "Failed to enumerate file list" );
			return false;
		}

		m_TestFiles.FilterExtension( ".txt;.tenshi;.te" );

		return true;
	}
	bool CTester::RunTests( Ax::uint32 Tests )
	{
		bool bDidAllSucceed = true;
		bool bDidSomeSucceed = false;
		for( Ax::uintptr i = 0; i < m_TestFiles.Num(); ++i ) {
			//Ax::Debugf( m_TestFiles.GetFile( i ), "Running test..." );
			if( !RunTest( m_TestFiles.GetFile( i ), Tests ) ) {
				bDidAllSucceed = false;
				Ax::Errorf( m_TestFiles.GetFile( i ), "Test failed" );
			} else {
				bDidSomeSucceed = true;
				Ax::Statusf( m_TestFiles.GetFile( i ), "Test passed" );
			}
		}

		if( !bDidAllSucceed ) {
			if( bDidSomeSucceed ) {
				Ax::BasicWarnf( "Some tests have failed" );
			} else {
				Ax::BasicErrorf( "All tests have failed" );
			}

			return false;
		}

		Ax::BasicStatusf( "All tests have passed" );
		return true;
	}
	bool CTester::RunTest( const Ax::String &Filename, Ax::uint32 Tests )
	{
		Ax::String FileText;
		Ax::TList< Ax::String > TestLines;

		if( !Ax::System::ReadFile( FileText, Filename ) ) {
			Ax::Errorf( Filename, "Failed to read file into memory" );
			return false;
		}

		Ax::intptr s = 0, p = 0;
		for(;;) {
			if( !FileText.Cmp( "#__TEST:", s, 8 ) ) {
				break;
			}

			s = s + 8;
			p = FileText.Find( '\n', s );
			if( p == -1 ) {
				p = FileText.Len();
			}

			if( !AX_VERIFY( TestLines.AddTail( FileText.Substring( s, p ).Trimmed() ) != TestLines.end() ) ) {
				FileText.Clear();
				TestLines.Clear();

				Ax::Errorf( Filename, "Failed to append test line" );
				return false;
			}

			FileText.Replace( s - 8, p, ' ' );

			if( p == FileText.Len() ) {
				break;
			}

			s = p + 1;
		}

		if( TestLines.IsEmpty() ) {
			Ax::Warnf( Filename, "This is not a test (does not begin with \"#__TEST:\")" );
			return true;
		}

		auto Iter = TestLines.begin();
		AX_ASSERT( Iter != TestLines.end() );

		ETestType TestType = ETestType::Unknown;

		// Check the type of test
		if( Iter->Cmp( "LEXER" ) ) {
			//Ax::Debugf( Filename, "Found lexer test" );
			TestType = ETestType::Lexer;
		} else if( Iter->Cmp( "PREPROCESSOR" ) ) {
			//Ax::Debugf( Filename, "Found preprocessor test" );
			TestType = ETestType::Preprocessor;
		} else if( Iter->Cmp( "PARSER" ) ) {
			//Ax::Debugf( Filename, "Found parser test" );
			TestType = ETestType::Parser;
		} else if( Iter->Cmp( "CODEGEN" ) ) {
			//Ax::Debugf( Filename, "Found code generator test" );
			TestType = ETestType::CodeGenerator;
		} else {
			Ax::Errorf( Filename, "Unknown test type <%s>", Iter->CString() );
			return false;
		}

		TestLines.Remove( Iter );
		switch( TestType )
		{
		case ETestType::Lexer:
			return RunTest_Lexer( Filename, TestLines, FileText );

		case ETestType::Preprocessor:
			return RunTest_Preprocessor( Filename, TestLines, FileText );

		case ETestType::Parser:
			return RunTest_Parser( Filename, TestLines, FileText );

		case ETestType::CodeGenerator:
			return RunTest_CodeGenerator( Filename, TestLines, FileText );

		case ETestType::Unknown:
			break;
		}

		AX_ASSERT_MSG( false, "Unreachable code" );
		return false;
	}
	bool CTester::RunTest_Lexer( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile )
	{
		Ax::Parser::CSource Src;

		Src.SetProcessor( GetDefaultSourceProcessor() );
		Src.SetText( Filename, TestFile );
		
		for( const Ax::String &TestLine : TestLines ) {
			//printf( "::<%s>\n", TestLine.CString() );
			const Ax::TArray< Ax::String > Tokens = TestLine.SplitUnquoted( " ", '\\', Ax::EKeepQuotes::No );
			if( Tokens[ 0 ] == "expect-token" ) {
				auto tok = Src.ReadToken();

				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "expect-token expects a token type" );
				}

				if( Tokens[ 1 ] == "none" ) {
					if( !!tok ) {
						tok->Error( "Did not expect token" );
						return false;
					}

					continue;
				}

				if( !tok ) {
					Src.Error( "Expected a token" );
					return false;
				}

				//tok->Report( Ax::ESeverity::Debug, "GOT: [" + tok->GetString() + "]" );

				if( Tokens[ 1 ] == "name" ) {
					if( !tok->IsName() ) {
						tok->Error( "Expected a name token" );
						return false;
					}
					if( tok->cLength > 128 ) {
						tok->Error( "This token is probably too long" );
						return false;
					}
					//tok->Report( Ax::ESeverity::Debug, "Found token: <" + tok->GetString() + ">" );

					if (Tokens.Num() > 2 ) {
						if( !tok->Cmp( Tokens[ 2 ] ) ) {
							tok->Error( Ax::String( "Expected name: " ) + Tokens[ 2 ] );
							return false;
						}

						for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
							if( Tokens[ i ] == "+keyword" ) {
								if( !tok->IsKeyword() ) {
									tok->Error( "Expected keyword" );
									return false;
								}

								continue;
							}
						}
					}
				} else if( Tokens[ 1 ] == "number" ) {
					if( !tok->IsNumber() ) {
						tok->Error( "Expected a number" );
						return false;
					}
					if( tok->cLength > 64 ) {
						tok->Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
							if( Tokens[ i ] == "+integer" ) {
								if( tok->NumberType != Ax::Parser::ENumberTokenType::Integer ) {
									tok->Error( "Expected integer" );
									return false;
								}

								const Ax::int64 tokint = tok->GetEncodedSigned();
								const Ax::int64 expint = Tokens[ 2 ].ToInteger( Ax::kRadix_BasicStyle, '_' );
								if( tokint != expint ) {
									tok->Error( Ax::String::Formatted( "Expected integer %i (got %i)", ( int )expint, ( int )tokint ) );
									return false;
								}

								continue;
							}

							if( Tokens[ i ] == "+float" ) {
								if( tok->NumberType != Ax::Parser::ENumberTokenType::Real ) {
									tok->Error( "Expected float" );
									return false;
								}

								const double tokflt = tok->GetEncodedFloat();
								const double expflt = Tokens[ 2 ].ToFloat();
								const double dif = tokflt - expflt >= 0.0 ? tokflt - expflt : expflt - tokflt;
								if( dif > 0.00001 ) {
									tok->Error( Ax::String::Formatted( "Expected float %g (got %g)", expflt, tokflt ) );
									return false;
								}

								continue;
							}
						}
					}
				} else if( Tokens[ 1 ] == "string" ) {
					if( !tok->IsString() ) {
						tok->Error( "Expected a string" );
						return false;
					}
					if( tok->cLength > 512 ) {
						tok->Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						const Ax::String tokstr = tok->GetUnencodedString();
						const Ax::String expstr = Tokens[ 2 ].Unescape();
						if( tokstr != expstr ) {
							tok->Error( "Expected string \"" + expstr + "\", but got \"" + tokstr + "\"" );
							return false;
						}
					}
				} else if( Tokens[ 1 ] == "punctuation" ) {
					if( !tok->IsPunctuation() ) {
						tok->Error( "Expected punctuation" );
					}
					if( tok->cLength > 8 ) {
						tok->Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						if( !tok->Cmp( Tokens[ 2 ] ) ) {
							tok->Error( "Expected punctuation \"" + Tokens[ 2 ] + "\", but got \"" + tok->GetString() + "\"" );
							return false;
						}
					}
				} else {
					Ax::Errorf( Filename, "Unknown expect-token type: %s", Tokens[ 1 ].CString() );
				}

				for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
					if( Tokens[ i ].StartsWith( "line:" ) ) {
						Ax::uint32 line = ( Ax::uint32 )Tokens[ i ].Substring( 5, -1 ).ToInteger();
						Ax::Parser::SLineInfo LineInfo = Src.CalculateLineInfo( tok->uOffset );
						if( LineInfo.Line != line ) {
							tok->Error( Ax::String::Formatted( "Expected token to be on line %u, but was found on line %u", line, LineInfo.Line ) );
							return false;
						}

						continue;
					}
					
					if( Tokens[ i ] == "+startline" ) {
						if( !tok->All( Ax::Parser::kTokF_StartsLine ) ) {
							tok->Error( "Expected token to start the line" );
							return false;
						}

						continue;
					}
					if( Tokens[ i ] == "-startline" ) {
						if( tok->All( Ax::Parser::kTokF_StartsLine ) ) {
							tok->Error( "Expected token to continue the line" );
							return false;
						}

						continue;
					}
				}
			} else if( Tokens[ 0 ] == "add-keyword" ) {
				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "add-keyword expects a keyword" );
					return false;
				}

				InstallKeyword( Tokens[ 1 ], nullptr, 0, Ax::Parser::EKeywordExists::Ignore );
			} else {
				Ax::Errorf( Filename, "Unknown directive <%s>", Tokens[ 0 ].CString() );
				return false;
			}
		}

		return true;
	}
	bool CTester::RunTest_Preprocessor( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile )
	{
		CLexer Lexer;

		if( !Lexer.LoadText( Filename, TestFile ) ) {
			Ax::Errorf( Filename, "Failed to load file text" );
			return false;
		}
		
		for( const Ax::String &TestLine : TestLines ) {
			//printf( "::<%s>\n", TestLine.CString() );
			const Ax::TArray< Ax::String > Tokens = TestLine.SplitUnquoted( " ", '\\', Ax::EKeepQuotes::No );
			if( Tokens[ 0 ] == "expect-token" ) {
				SToken &tok = const_cast< SToken & >( Lexer.Lex() );

				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "expect-token expects a token type" );
				}

				if( Tokens[ 1 ] == "none" ) {
					if( !!tok ) {
						tok.Error( "Did not expect token" );
						return false;
					}

					continue;
				}

				if( !tok ) {
					Lexer.Error( "Expected a token" );
					return false;
				}

				//Ax::BasicDebugf( "GOT: [" + tok.GetString() + "]" );

				if( Tokens[ 1 ] == "name" ) {
					if( !tok.IsName() ) {
						tok.Error( "Expected a name token" );
						return false;
					}
					if( tok.cLength > 128 ) {
						tok.Error( "This token is probably too long" );
						return false;
					}
					//tok->Report( Ax::ESeverity::Debug, "Found token: <" + tok->GetString() + ">" );

					if (Tokens.Num() > 2 ) {
						if( !tok.Cmp( Tokens[ 2 ] ) ) {
							tok.Error( Ax::String( "Expected name: " ) + Tokens[ 2 ] );
							return false;
						}

						for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
							if( Tokens[ i ] == "+keyword" ) {
								if( !tok.IsKeyword() ) {
									tok.Error( "Expected keyword" );
									return false;
								}

								continue;
							}
						}
					}
				} else if( Tokens[ 1 ] == "number" ) {
					if( !tok.IsNumber() ) {
						tok.Error( "Expected a number" );
						return false;
					}
					if( tok.cLength > 64 ) {
						tok.Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
							if( Tokens[ i ] == "+integer" ) {
								if( tok.NumberType != Ax::Parser::ENumberTokenType::Integer ) {
									tok.Error( "Expected integer" );
									return false;
								}

								const Ax::int64 tokint = tok.GetEncodedSigned();
								const Ax::int64 expint = Tokens[ 2 ].ToInteger( Ax::kRadix_BasicStyle, '_' );
								if( tokint != expint ) {
									tok.Error( Ax::String::Formatted( "Expected integer %i (got %i)", ( int )expint, ( int )tokint ) );
									return false;
								}

								continue;
							}

							if( Tokens[ i ] == "+float" ) {
								if( tok.NumberType != Ax::Parser::ENumberTokenType::Real ) {
									tok.Error( "Expected float" );
									return false;
								}

								const double tokflt = tok.GetEncodedFloat();
								const double expflt = Tokens[ 2 ].ToFloat();
								const double dif = tokflt - expflt >= 0.0 ? tokflt - expflt : expflt - tokflt;
								if( dif > 0.00001 ) {
									tok.Error( Ax::String::Formatted( "Expected float %g (got %g)", expflt, tokflt ) );
									return false;
								}

								continue;
							}
						}
					}
				} else if( Tokens[ 1 ] == "string" ) {
					if( !tok.IsString() ) {
						tok.Error( "Expected a string" );
						return false;
					}
					if( tok.cLength > 512 ) {
						tok.Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						const Ax::String tokstr = tok.GetUnencodedString();
						const Ax::String expstr = Tokens[ 2 ].Unescape();
						if( tokstr != expstr ) {
							tok.Error( "Expected string \"" + expstr + "\", but got \"" + tokstr + "\"" );
							return false;
						}
					}
				} else if( Tokens[ 1 ] == "punctuation" ) {
					if( !tok.IsPunctuation() ) {
						tok.Error( "Expected punctuation" );
					}
					if( tok.cLength > 8 ) {
						tok.Error( "This token is probably too long" );
						return false;
					}

					if( Tokens.Num() > 2 ) {
						if( !tok.Cmp( Tokens[ 2 ] ) ) {
							tok.Error( "Expected punctuation \"" + Tokens[ 2 ] + "\", but got \"" + tok.GetString() + "\"" );
							return false;
						}
					}
				} else {
					Ax::Errorf( Filename, "Unknown expect-token type: %s", Tokens[ 1 ].CString() );
				}

				for( Ax::uintptr i = 3; i < Tokens.Num(); ++i ) {
					if( Tokens[ i ].StartsWith( "line:" ) ) {
						Ax::uint32 line = ( Ax::uint32 )Tokens[ i ].Substring( 5, -1 ).ToInteger();
						Ax::Parser::SLineInfo LineInfo = tok.pSource->CalculateLineInfo( tok.uOffset );
						if( LineInfo.Line != line ) {
							tok.Error( Ax::String::Formatted( "Expected token to be on line %u, but was found on line %u", line, LineInfo.Line ) );
							return false;
						}

						continue;
					}
					
					if( Tokens[ i ] == "+startline" ) {
						if( !tok.All( Ax::Parser::kTokF_StartsLine ) ) {
							tok.Error( "Expected token to start the line" );
							return false;
						}

						continue;
					}
					if( Tokens[ i ] == "-startline" ) {
						if( tok.All( Ax::Parser::kTokF_StartsLine ) ) {
							tok.Error( "Expected token to continue the line" );
							return false;
						}

						continue;
					}
				}
			} else if( Tokens[ 0 ] == "add-keyword" ) {
				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "add-keyword expects a keyword" );
					return false;
				}

				InstallKeyword( Tokens[ 1 ], nullptr, 0, Ax::Parser::EKeywordExists::Ignore );
			} else {
				Ax::Errorf( Filename, "Unknown directive <%s>", Tokens[ 0 ].CString() );
				return false;
			}
		}

		return true;
	}
	bool CTester::RunTest_Parser( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile )
	{
		CParser Parser;

		if( !Parser.LoadText( Filename, TestFile ) ) {
			Ax::Errorf( Filename, "Failed to load file text" );
			return false;
		}
		
		for( const Ax::String &TestLine : TestLines ) {
			const Ax::TArray< Ax::String > Tokens = TestLine.SplitUnquoted( " ", '\\', Ax::EKeepQuotes::No );

			if( Tokens[ 0 ] == "parse-program" ) {
				if( !Parser.ParseProgram() ) {
					Ax::Errorf( Filename, "parse-program failed" );
					return false;
				}

				continue;
			} 

			if( Tokens[ 0 ] == "print-ast" ) {
				Ax::Statusf( Filename, "AST listing (in stdout)" );
				Parser.PrintAST();
				continue;
			}

			if( Tokens[ 0 ] == "semant" ) {
				if( !Parser.Semant() ) {
					Ax::Errorf( Filename, "semant failed" );
					return false;
				}

				continue;
			}

			if( Tokens[ 0 ] == "print-syms" ) {
				Ax::Statusf( Filename, "Symbols listing (in stdout)" );

				Ax::String SymsStr;

				for( Ax::uintptr i = 0; i < g_Prog->GlobalScope().NumSymbols(); ++i ) {
					const SSymbol *const pSym = g_Prog->GlobalScope().Symbol( i );

					AX_ASSERT_NOT_NULL( pSym );
					AX_ASSERT_NOT_NULL( pSym->pDeclToken );
					const auto *const pTok = pSym->pNameToken != nullptr ? pSym->pNameToken : pSym->pDeclToken;
					AX_EXPECT_MSG( SymsStr.Append( pTok->GetString() + "\n" ), "Out of memory" );
				}

				printf( "%s", SymsStr.CString() );
				continue;
			}

			if( Tokens[ 0 ] == "print-types" ) {
				Ax::Statusf( Filename, "Types listing (in stdout)" );

				Ax::String TypeStr;
				
				for( Ax::uintptr i = 0; i < g_Prog->GlobalScope().NumSymbols(); ++i ) {
					const SSymbol *const pSym = g_Prog->GlobalScope().Symbol( i );

#if 0
					AX_ASSERT_NOT_NULL( pSym );
					AX_EXPECT_MSG( TypeStr.Append( pSym->DeclToken().GetString() + "\n" ), "Out of memory" );
#endif

					if( !pSym || !pSym->pUDT ) {
						continue;
					}

					AX_EXPECT_MSG( TypeStr.Append( pSym->pUDT->ToString() ), "Out of memory" );
					AX_EXPECT_MSG( TypeStr.Append( "\n" ), "Out of memory" );
				}

				printf( "%s", TypeStr.CString() );
				continue;
			}

			if( Tokens[ 0 ] == "expect-type-size" ) {
				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "Expected type name for expect-type-size" );
					return false;
				}
				if( Tokens.Num() < 3 ) {
					Ax::Errorf( Filename, "Expected size of type (in bytes) for expect-type-size" );
					return false;
				}

				const SSymbol *const pSym = g_Prog->GlobalScope().FindSymbol( Tokens[ 1 ] );
				if( !pSym ) {
					Ax::Errorf( Filename, "Type \"%s\" not found", Tokens[ 1 ].CString() );
					return false;
				}

				if( !pSym->pUDT ) {
					Ax::Errorf( Filename, "Symbol \"%s\" is not a (user defined) type", Tokens[ 1 ].CString() );
					return false;
				}

				if( pSym->pUDT->cBytes != ( Ax::uint32 )Tokens[ 2 ].ToUnsignedInteger() ) {
					Ax::Errorf( Filename, "Type \"%s\" is %u bytes, not %s", Tokens[ 1 ].CString(), pSym->pUDT->cBytes, Tokens[ 2 ].CString() );
					return false;
				}

				continue;
			}

			if( Tokens[ 0 ] == "load-mod" ) {
				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "Expected module name for load-mod" );
					return false;
				}

				if( Tokens[ 1 ] == "$core" ) {
					Mods->LoadCoreInternal();
				} else if( Tokens[ 1 ] == "$plugins-core" ) {
					Mods->LoadCorePlugins();
				} else if( Tokens[ 1 ] == "$from" ) {
					if( Tokens.Num() > 2 ) {
						Mods->LoadDirectory( Tokens[ 2 ] );
					} else {
						Ax::Errorf( Filename, "Expected directory name for load-mod $from" );
						return false;
					}
				} else {
					if( !Mods->LoadFromFile( Tokens[ 1 ] ) ) {
						Ax::Errorf( Filename, "Call to load-mod failed" );
						return false;
					}
				}

				continue;
			}

			Ax::Errorf( Filename, "Unknown directive <%s>", Tokens[ 0 ].CString() );
			return false;
		}

		return true;
	}
	bool CTester::RunTest_CodeGenerator( const Ax::String &Filename, Ax::TList< Ax::String > &TestLines, const Ax::String &TestFile )
	{
		CParser Parser;

		if( CG->IsInitialized() ) {
			CG->Fini();
		}

		CG->Init();

		if( !Parser.LoadText( Filename, TestFile ) ) {
			Ax::Errorf( Filename, "Failed to load file text" );
			return false;
		}
		
		for( const Ax::String &TestLine : TestLines ) {
			const Ax::TArray< Ax::String > Tokens = TestLine.SplitUnquoted( " ", '\\', Ax::EKeepQuotes::No );

			if( Tokens[ 0 ] == "compile" ) {
				if( !Parser.ParseProgram() ) {
					Ax::Errorf( Filename, "compile failed (Syntax)" );
					return false;
				}

				if( !Parser.Semant() ) {
					Ax::Errorf( Filename, "compile failed (Semantics)" );
					return false;
				}

				if( !Parser.CodeGen() ) {
					CG->Dump();
					Ax::Errorf( Filename, "compile failed (CodeGen)" );
					return false;
				}

				continue;
			}

			if( Tokens[ 0 ] == "dump" ) {
				CG->Dump();
				continue;
			}

			if( Tokens[ 0 ] == "load-mod" ) {
				if( Tokens.Num() < 2 ) {
					Ax::Errorf( Filename, "Expected module name for load-mod" );
					return false;
				}

				if( Tokens[ 1 ] == "$core" ) {
					Mods->LoadCoreInternal();
				} else if( Tokens[ 1 ] == "$plugins-core" ) {
					Mods->LoadCorePlugins();
				} else if( Tokens[ 1 ] == "$from" ) {
					if( Tokens.Num() > 2 ) {
						Mods->LoadDirectory( Tokens[ 2 ] );
					} else {
						Ax::Errorf( Filename, "Expected directory name for load-mod $from" );
						return false;
					}
				} else {
					if( !Mods->LoadFromFile( Tokens[ 1 ] ) ) {
						Ax::Errorf( Filename, "Call to load-mod failed" );
						return false;
					}
				}

				continue;
			}

			if( Tokens[ 0 ] == "+optimize" ) {
				CG->SetOptimize( true );
				continue;
			}
			if( Tokens[ 0 ] == "-optimize" ) {
				CG->SetOptimize( false );
				continue;
			}
			if( Tokens[ 0 ] == "+labeldebug" ) {
				CG->SetLabelDebugLogging( true );
				continue;
			}
			if( Tokens[ 0 ] == "-labeldebug" ) {
				CG->SetLabelDebugLogging( false );
				continue;
			}

			Ax::Errorf( Filename, "Unknown directive <%s>", Tokens[ 0 ].CString() );
			return false;
		}

		return true;
	}

}}

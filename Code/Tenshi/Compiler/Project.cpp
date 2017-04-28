#include "_PCH.hpp"
#include "Project.hpp"

#include "Platform.hpp"
#include "Environment.hpp"

#include "Parser.hpp"
#include "CodeGen.hpp"

#include "Binutils.hpp"

#ifndef _WIN32
# include <unistd.h>
#endif

namespace Tenshi { namespace Compiler {

	/*
	===========================================================================

		PROJECT MANAGER

	===========================================================================
	*/

	MProjects &MProjects::GetInstance()
	{
		static MProjects instance;
		return instance;
	}

	MProjects::MProjects()
	{
	}
	MProjects::~MProjects()
	{
	}

	bool MProjects::AddProjectDirectory( const char *pszProjDir )
	{
		AX_ASSERT_NOT_NULL( pszProjDir );

		char szPath[ MAX_PATH + 1 ];
		if( !Ax::GetAbsolutePath( szPath, pszProjDir ) ) {
			return false;
		}
		if( !Ax::System::IsDir( szPath ) ) {
			return false;
		}

		auto R = m_ProjectDirs.AddTail();
		AX_EXPECT_MEMORY( R != m_ProjectDirs.end() );
		AX_EXPECT_MEMORY( R->Assign( szPath ) );

		return true;
	}

	CProject &MProjects::Current()
	{
		if( m_Projects.IsEmpty() ) {
			return *Add();
		}

		return *m_Projects.Last();
	}

	CProject *MProjects::Add()
	{
		auto R = m_Projects.AddTail();
		AX_EXPECT_MEMORY( R != m_Projects.end() );

		return R.Get();
	}
	CProject *MProjects::Load( const char *pszProjFile )
	{
		AX_ASSERT_NOT_NULL( pszProjFile );

		auto R = m_Projects.AddTail();
		AX_EXPECT_MEMORY( R != m_Projects.end() );

		if( !R->LoadFromFile( pszProjFile ) ) {
			m_Projects.Remove( R );
			return nullptr;
		}

		return R.Get();
	}

	CProject *MProjects::FindExisting( const char *pszName, const char *pszNameEnd ) const
	{
		AX_ASSERT_NOT_NULL( pszName );

		const char *const s = pszName;
		const char *const e = pszNameEnd != nullptr ? pszNameEnd : strchr( pszName, '\0' );

		const Ax::uintptr n = e - s;

		for( CProject &Proj : m_Projects ) {
			if( Proj.m_Name.Len() != n ) {
				continue;
			}

			if( !Proj.m_Name.CaseCmp( s, 0, ( Ax::intptr )n ) ) {
				continue;
			}

			return &Proj;
		}

		return nullptr;
	}
	
	CProject *MProjects::Remove( CProject *pProj )
	{
		for( auto R = m_Projects.Last(); !!R; --R ) {
			if( R.Get() != pProj ) {
				continue;
			}

			m_Projects.Remove( R );
			break;
		}

		return nullptr;
	}
	void MProjects::Clear()
	{
		m_Projects.Clear();
	}

	static Ax::String Plural( unsigned x, const char *name, const char *plural = "s", const char *nonplural = "" )
	{
		return Ax::String::Formatted( "%u %s%s", x, name, x == 1 ? nonplural : plural );
	}
	bool MProjects::Build()
	{
		unsigned cSuccesses = 0;
		unsigned cFailures = 0;
		for( CProject &Proj : m_Projects ) {
			if( Proj.Build() ) {
				++cSuccesses;
			} else {
				++cFailures;
			}
		}

		if( !cSuccesses && !cFailures ) {
			Ax::g_VerboseLog += "Nothing to build";
		} else {
			Ax::g_VerboseLog +=
				"Built projects [" +
				Plural( cSuccesses, "success", "es" ) +
				", " +
				Plural( cFailures, "failure" ) +
				"]";
		}

		return cFailures == 0;
	}


	/*
	===========================================================================

		PROJECT TOKEN

	===========================================================================
	*/

	bool SProjToken::Unquote( Ax::String &Dst, bool bAppend ) const
	{
		AX_ASSERT_NOT_NULL( s );
		AX_ASSERT_NOT_NULL( e );

		if( !bAppend ) {
			Dst.Clear();
		}

		if( !Dst.Reserve( Dst.Num() + ( Ax::uintptr )( e - s ) ) ) {
			return false;
		}

		if( *s != '\"' ) {
			return Dst.Append( s, e );
		}

		const char *p = s + 1;
		while( p < e && *p != '\"' ) {
			const char *q = p;
			while( q < e && *q != '\"' ) {
				if( *q == '\\' ) {
					break;
				}

				++q;
			}

			if( q != p ) {
				if( !Dst.Append( p, q ) ) {
					return false;
				}

				if( q == e ) {
					break;
				}
			}

			p = q + 1;

			bool bSucceeded = true;
			if( *p == 'n' ) {
				++p;
				bSucceeded = Dst.Append( '\n' );
			} else if( *p == 'r' ) {
				++p;
				bSucceeded = Dst.Append( '\r' );
			} else if( *p == 't' ) {
				++p;
				bSucceeded = Dst.Append( '\t' );
			} else if( *p == '\?' ) {
				++p;
				bSucceeded = Dst.Append( '\?' );
			} else if( *p == '\'' ) {
				++p;
				bSucceeded = Dst.Append( '\'' );
			} else if( *p == '\"' ) {
				++p;
				bSucceeded = Dst.Append( '\"' );
			} else if( *p == '\\' ) {
				++p;
				bSucceeded = Dst.Append( '\\' );
			} else if( *p == 'u' ) {
				bool curly = false;
				if( p[1] == '{' ) {
					curly = true;
					p += 2;
				} else {
					++p;
				}

				char szDigit[ 9 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				unsigned cDigits = 0;

				if( curly ) {
					while( p < e && *p != '}' ) {
						if( cDigits < 9 ) {
							szDigit[ cDigits++ ] = *p;
						}

						++p;
					}
				} else {
					while( p < e && cDigits < 4 ) {
						szDigit[ cDigits++ ] = *p++;
					}
				}

				const Ax::uint32 UTF32Char = ( Ax::uint32 )Ax::String::ToUnsignedInteger( szDigit, 16 );
				bSucceeded = Dst.AppendUTF32Char( UTF32Char );
			} else if( *p == 'U' ) {
				char szDigit[ 9 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				unsigned cDigits = 0;

				++p;
				while( p < e && cDigits < 8 ) {
					szDigit[ cDigits++ ] = *p++;
				}

				const Ax::uint32 UTF32Char = ( Ax::uint32 )Ax::String::ToUnsignedInteger( szDigit, 16 );
				bSucceeded = Dst.AppendUTF32Char( UTF32Char );
			}

			if( !bSucceeded ) {
				return false;
			}
		}

		return true;
	}


	/*
	===========================================================================

		PROJECT

	===========================================================================
	*/

	CProject::CProject()
	: m_Name( "a" )
	, m_TargetPath()
	, m_bTargetPrefix( true )
	, m_bTargetSuffix( true )
	, m_bASMList( false )
	, m_bIRList( false )
	, m_TargetType( ELinkTarget::Executable )
	, m_TargetEnv( ETargetEnv::Terminal )
	, m_LTO( ELTOConfig::Disabled )
	, m_BuildInfo()
	, m_Settings()
	, m_Compilations()
	, m_pCurrentCompilation( nullptr )
	, m_Modules()
	{
	}
	CProject::~CProject()
	{
	}

	bool CProject::LoadFromFile( const char *pszFilename )
	{
		AX_ASSERT_NOT_NULL( pszFilename );

		Ax::String Text;

		char szPath[ PATH_MAX ];
		bool bDidFSEnter = false;

#ifdef _WIN32
		GetCurrentDirectoryA( sizeof( szPath ), szPath );
#else
		getcwd( szPath, sizeof( szPath ) );
#endif
		AX_DEBUG_LOG += "Loading project \"" + Ax::String( pszFilename ) + "\" -- from directory=\"" + Ax::String( szPath ) + "\"";

		if( Ax::GetAbsolutePath( szPath, pszFilename ) != nullptr ) {
			Ax::Debugf( szPath, "Absolute path to project file" );
			bDidFSEnter = Ax::System::PushDir( Ax::String( szPath ).ExtractDirectory() );
			if( !bDidFSEnter ) {
				Ax::Errorf( pszFilename, "Failed to enter directory: \"" + Ax::String( szPath ).ExtractDirectory() + "\"" );
				return false;
			}
		}
		AX_SCOPE_GUARD({if( bDidFSEnter ){ Ax::System::PopDir(); }});

		if( !Ax::System::ReadFile( Text, szPath ) ) {
			return false;
		}

		m_BuildInfo = g_Env->BuildInfo();

		const char *text_p = Text.CString();
		const char *text_e = text_p + Text.Num();

		const char *line_s = text_p;
		Ax::uint32 uLine = 1;

		while( text_p <= text_e ) {
			if( *text_p != '\n' && text_p != text_e ) {
				++text_p;
				continue;
			}

			const char *line_e = text_p;
			if( !ApplyLine( pszFilename, uLine, line_s, line_e ) ) {
				return false;
			}

			text_p = line_e + 1;
			line_s = text_p;
			++uLine;
		}

		return true;
	}
	bool CProject::ApplyLine( const char *pszFilename, Ax::uint32 uLine, const char *pszLineStart, const char *pszLineEnd )
	{
		SProjToken Tokens[ 128 ];
		Ax::uintptr cTokens = 0;

		AX_ASSERT_NOT_NULL( pszFilename );
		AX_ASSERT_NOT_NULL( pszLineStart );

		SProjDiagState Diag;

		Diag.pszFilename = pszFilename;
		Diag.uLine = uLine;
		Diag.pszLineStart = pszLineStart;

		// the start of the line
		const char *s = pszLineStart;
		// skip white-space
		while( *( const unsigned char * )s <= ' ' && *s != '\0' ) {
			++s;
		}

		// early exit if the line starts with a '#'
		if( *s == '#' ) {
			return true;
		}

		// end of the line
		const char *e = strchr( s, '\n' );
		if( !e ) {
			e = strchr( s, '\r' );
			if( !e ) {
				e = strchr( s, '\0' );
			}
		}

		// ignore trailing white-space
		while( e > s && *( const unsigned char * )( e - 1 ) <= ' ' ) {
			--e;
		}

		// current pointer to the line
		const char *p = s;

		// tokenize the line
		while( p < e ) {
			// start of the token
			const char *tok_s = p;
			// end of the token
			const char *tok_e = nullptr;

			// parse token
			if( *p == '\"' ) {
				++p;

				while( p < e && *p != '\"' ) {
					if( *p == '\\' ) {
						++p;
						if( p == e ) {
							break;
						}
					}

					++p;
				}

				if( *p == '\"' ) {
					++p;
				}
			} else {
				while( *( const unsigned char * )p > ' ' && p < e ) {
					++p;
				}
			}
			tok_e = p;

			// push the token back
			if( !PushToken( Tokens, cTokens, tok_s, tok_e ) ) {
				Ax::g_ErrorLog( pszFilename, uLine, ( Ax::uint32 )( tok_s - pszLineStart ) ) += "Too many tokens on line";
				return false;
			}

			// skip trailing white-space
			while( *( const unsigned char * )p <= ' ' && p < e ) {
				++p;
			}

			// done if have a single-line comment
			if( *p == '#' ) {
				break;
			}
		}

		// current token index
		Ax::uintptr uToken = 0;

		// handle conditionals
		while( uToken < cTokens ) {
			// token to check
			const SProjToken &Tok = Tokens[ uToken ];
			// break on a non-conditional (without incrementing uToken)
			if( *Tok.s != '\?' ) {
				break;
			}

			// increment uToken now (for 'continue' later)
			++uToken;

			// '?0' always evaluates to false
			if( Tok == "?0" ) {
				return true;
			}

			// '?1' always evaluates to true
			if( Tok == "?1" ) {
				continue;
			}

			// '?Debug' is true when building in debug mode
			if( Tok == "?Debug" ) {
				if( !g_Env->IsDebugEnabled() ) {
					return true;
				}

				continue;
			}
			// '?Release' is true when building in release mode (not debug mode)
			if( Tok == "?Release" ) {
				if( g_Env->IsDebugEnabled() ) {
					return true;
				}

				continue;
			}

			//
			//	TODO: Add more conditionals
			//
			//	x	?Debug          True if building in a debug mode.
			//	x	?Release        True if building in a release mode.
			//		?Test           True if building in a unit-testing mode.
			//		?Profile        True if building in a profiling mode.
			//
			//		?Win32          True if targeting Windows, but not the Windows Store.
			//		?Linux          True if targeting desktop GNU/Linux distribution.
			//		?MacOSX         True if targeting Mac OS X.
			//		?Android        True if targeting Android. (?Linux is not true for this.)
			//		?iOS            True if targeting iOS. (?MacOSX is not true for this.)
			//
			//		?Microsoft      True if targeting any Microsoft OS.
			//		?UNIX           True if targeting any UNIX-like OS.
			//
			//		?HadDependency  True if the last OptionalDependency found the project.
			//

			// unrecognized conditional evaluates to false
			return true;
		}

		// blank line? (or no command after conditionals)
		if( uToken == cTokens ) {
			return true;
		}

		// command token
		const SProjToken &Cmd = Tokens[ uToken ];
		// index to the first argument token
		Ax::uintptr uArg = uToken + 1;

		// [[ Name <Project Name> ]] :: Set the project's name
		if( Cmd == "Name" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}

			AX_EXPECT_MEMORY( Tokens[ uArg + 0 ].Unquote( m_Name ) );
			return true;
		}

		// [[ Type <Project Type> ]] :: Set the project's type
		if( Cmd == "Type" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}

			const SProjToken &Arg = Tokens[ uArg + 0 ];

			if( Arg == "Executable" ) {
				m_TargetType = ELinkTarget::Executable;
				m_TargetEnv = ETargetEnv::Terminal;
				m_BuildInfo.Platform.Subsystem = ESubsystem::Text;
			} else if( Arg == "Application" ) {
				m_TargetType = ELinkTarget::Executable;
				m_TargetEnv = ETargetEnv::Windowed;
				m_BuildInfo.Platform.Subsystem = ESubsystem::Window;
			} else if( Arg == "StaticLibrary" ) {
				m_TargetType = ELinkTarget::StaticLibrary;
			} else if( Arg == "DynamicLibrary" ) {
				m_TargetType = ELinkTarget::DynamicLibrary;
			} else if( Arg == "Pipeline" ) {
				m_TargetType = ELinkTarget::Executable;
				m_TargetEnv = ETargetEnv::Terminal;
				m_BuildInfo.Platform.Subsystem = ESubsystem::Text;
			} else if( Arg == "Editor" ) {
				m_TargetType = ELinkTarget::Executable;
				m_TargetEnv = ETargetEnv::Windowed;
				m_BuildInfo.Platform.Subsystem = ESubsystem::Window;
			} else if( Arg == "Game" ) {
				m_TargetType = ELinkTarget::Executable;
				m_TargetEnv = ETargetEnv::Platform;
				m_BuildInfo.Platform.Subsystem = ESubsystem::Window;
			} else {
				Diag.Error( Arg, "Unknown project type" );
				return false;
			}

			return true;
		}

		// [[ Target <Output Filename> ]] :: Set the target output file
		if( Cmd == "Target" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}

			const SProjToken &Arg = Tokens[ uArg + 0 ];

			Ax::String Target;
			AX_EXPECT_MEMORY( Arg.Unquote( Target ) );

			char szAbsPath[ MAX_PATH + 1 ];
			if( Ax::GetAbsolutePath( szAbsPath, Target ) != nullptr ) {
				AX_EXPECT_MEMORY( Target.Assign( szAbsPath ) );
			}

			AX_EXPECT_MEMORY( m_TargetPath.Assign( Target.ExtractDirectory() ) );

			const char *pszPrefix = "";
			if( m_bTargetPrefix ) {
				static const char *const pszLibPrefix = "lib";
				if( m_TargetType == ELinkTarget::StaticLibrary ) {
					pszPrefix = pszLibPrefix;
				} else if( m_TargetType == ELinkTarget::DynamicLibrary && g_Env->BuildInfo().Platform.OSName != "Windows" ) {
					pszPrefix = pszLibPrefix;
				}
			}

			const char *pszSuffix = "";
			if( m_bTargetSuffix ) {
				if( m_TargetType == ELinkTarget::Executable ) {
					pszSuffix = g_Env->BuildInfo().Platform.ExeExt;
				} else if( m_TargetType == ELinkTarget::DynamicLibrary ) {
					pszSuffix = g_Env->BuildInfo().Platform.DLLExt;
				} else if( m_TargetType == ELinkTarget::StaticLibrary ) {
					pszSuffix = ".a";
				}
			}

			AX_EXPECT_MEMORY( m_TargetPath.AppendPath( pszPrefix ) );
			AX_EXPECT_MEMORY( m_TargetPath.Append( Target.ExtractFilename() ) );
			AX_EXPECT_MEMORY( m_TargetPath.Append( pszSuffix ) );

			Ax::g_DebugLog( Diag.pszFilename, Diag.uLine ) += "Target path set: " + m_TargetPath;

			return true;
		}

		// [[ -TargetPrefix ]] :: Disable prefix generation for the target filename
		if( Cmd == "-TargetPrefix" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bTargetPrefix = false;
			return true;
		}
		// [[ +TargetPrefix ]] :: Enable prefix generation for the target filename
		if( Cmd == "+TargetPrefix" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bTargetPrefix = true;
			return true;
		}
		// [[ -TargetSuffix ]] :: Disable suffix generation for the target filename
		if( Cmd == "-TargetSuffix" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bTargetSuffix = false;
			return true;
		}
		// [[ +TargetSuffix ]] :: Enable suffix generation for the target filename
		if( Cmd == "+TargetSuffix" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bTargetSuffix = true;
			return true;
		}

		// [[ EmitLLVMText <Output Filename> ]] :: List the LLVM IR
		if( Cmd == "EmitLLVMText" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}

			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'EmitLLVMText' not implemented yet" );
			return false;
		}

		// [[ EmitModules <Output Filename> ]] :: Lists the modules (plugins) being used
		if( Cmd == "EmitModules" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}

			Diag.Error( Cmd, "'EmitModules' not implemented yet" );
			return false;
		}

		// [[ +ASMList ]] :: Enable assembly file listing to <Obj>-.o+.s
		if( Cmd == "+ASMList" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bASMList = true;
			return true;
		}
		// [[ -ASMList ]] :: Disable assembly file listing
		if( Cmd == "-ASMList" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bASMList = false;
			return true;
		}

		// [[ +IRList ]] :: Enable LLVM IR file listing to <Obj>-.o+.ll
		if( Cmd == "+IRList" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bIRList = true;
			return true;
		}
		// [[ -IRList ]] :: Disable LLVM IR file listing
		if( Cmd == "-IRList" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}

			m_bIRList = false;
			return true;
		}

		// [[ Compile <Source Filename> [ <Object Filename> ] ]] :: Create a compilation unit
		if( Cmd == "Compile" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes, 2 ) ) {
				return false;
			}

			SCompilation::Iter iter = m_Compilations.AddTail();
			AX_EXPECT_MEMORY( iter != m_Compilations.end() );

			iter->Settings = m_Settings;

			char szTemp[ PATH_MAX + 1 ];

			const SProjToken &SrcArg = Tokens[ uArg + 0 ];
			AX_EXPECT_MEMORY( SrcArg.Unquote( iter->SourceFilename ) );

			if( Ax::GetAbsolutePath( szTemp, iter->SourceFilename ) != nullptr ) {
				AX_EXPECT_MEMORY( iter->SourceFilename.Assign( szTemp ) );
			}

			if( uArg + 1 < cTokens ) {
				const SProjToken &ObjArg = Tokens[ uArg + 1 ];
				AX_EXPECT_MEMORY( ObjArg.Unquote( iter->ObjectFilename ) );

				if( Ax::GetAbsolutePath( szTemp, iter->ObjectFilename ) != nullptr ) {
					AX_EXPECT_MEMORY( iter->ObjectFilename.Assign( szTemp ) );
				}
			} else {
				Ax::String &Obj = iter->ObjectFilename;

				AX_EXPECT_MEMORY( Obj.Assign( iter->SourceFilename ) );
				AX_EXPECT_MEMORY( Obj.ReplaceExtension( ".o" ) );
			}

			if( m_bASMList ) {
				AX_EXPECT_MEMORY( iter->ASListFilename.Assign( iter->ObjectFilename ) );
				AX_EXPECT_MEMORY( iter->ASListFilename.ReplaceExtension( ".s" ) );
			}

			if( m_bIRList ) {
				AX_EXPECT_MEMORY( iter->IRListFilename.Assign( iter->ObjectFilename ) );
				AX_EXPECT_MEMORY( iter->IRListFilename.ReplaceExtension( ".ll" ) );
			}
			
			return true;
		}

		// [[ Dependency <Project Name> ]] :: Add a dependency on another project (required to exist)
		if( Cmd == "Dependency" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Dependency' not implemented yet" );
			return false;
		}
		// [[ OptionalDependency <Project Name> ]] :: Add a dependency on another project (optional)
		if( Cmd == "OptionalDependency" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'OptionalDependency' not implemented yet" );
			return false;
		}

		// [[ +Module <Module Name> ]] :: Require and import the given module
		if( Cmd == "+Module" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'+Module' not implemented yet" );
			return false;
		}
		// [[ -Module <Module Name> ]] :: Exclude the given module
		if( Cmd == "-Module" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'-Module' not implemented yet" );
			return false;
		}
		// [[ -Modules ]] :: Exclude all modules not explicitly added
		if( Cmd == "-Modules" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 0 ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'-Modules' not implemented yet" );
			return false;
		}

		// [[ Shell.PreDependencies <ShellCommand...> ]]
		if( Cmd == "Shell.PreDependencies" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PreDependencies' not implemented yet" );
			return false;
		}
		// [[ Shell.PostDependencies <ShellCommand...> ]]
		if( Cmd == "Shell.PostDependencies" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PostDependencies' not implemented yet" );
			return false;
		}
		// [[ Shell.PreCompile [ "<" <InputFile> | ">" <OutputFile> ] <ShellCommand...>
		if( Cmd == "Shell.PreCompile" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PreCompile' not implemented yet" );
			return false;
		}
		// [[ Shell.PostCompile [ "<" <InputFile> | ">" <OutputFile> ] <ShellCommand...>
		if( Cmd == "Shell.PostCompile" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PostCompile' not implemented yet" );
			return false;
		}
		// [[ Shell.PreBuild <ShellCommands...> ]]
		if( Cmd == "Shell.PreBuild" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PreBuild' not implemented yet" );
			return false;
		}
		// [[ Shell.PostBuild <ShellCommands...> ]]
		if( Cmd == "Shell.PostBuild" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.PostBuild' not implemented yet" );
			return false;
		}
		// [[ Shell.FailedDependencies <ShellCommand...> ]]
		if( Cmd == "Shell.FailedDependencies" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.FailedDependencies' not implemented yet" );
			return false;
		}
		// [[ Shell.FailedCompile [ "<" <InputFile> | ">" <OutputFile> ] <ShellCommand...>
		if( Cmd == "Shell.FailedCompile" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.FailedCompile' not implemented yet" );
			return false;
		}
		// [[ Shell.FailedBuild <ShellCommand...> ]]
		if( Cmd == "Shell.FailedCommand" ) {
			if( !HasParm( Tokens, cTokens, Diag, Cmd, 1, EVarArgs::Yes ) ) {
				return false;
			}
			
			AX_DEBUG_LOG += "Unimplemented";
			Diag.Error( Cmd, "'Shell.FailedCommand' not implemented yet" );
			return false;
		}

		// unknown command
		Diag.Error( Cmd, "Unknown command" );
		return false;
	}

	bool CProject::Build()
	{
		Ax::TArray< Ax::String > Objects;
		Ax::g_VerboseLog += "Building project \"" + m_Name + "\"...";

		g_Env->SetBuildInfo( m_BuildInfo );

		unsigned cSuccesses = 0;
		unsigned cFailures = 0;
		for( SCompilation &Unit : m_Compilations ) {
			if( !Unit.ObjectFilename.IsEmpty() ) {
				AX_EXPECT_MEMORY( Objects.Append( Unit.ObjectFilename ) );
			}

			if( Unit.Build() ) {
				++cSuccesses;
			} else {
				++cFailures;
			}
		}

		if( !cFailures && !Objects.IsEmpty() ) {
			cFailures += +( Binutils->Link( m_TargetPath, Objects, m_Modules ) != EXIT_SUCCESS );
		}

		return !cFailures;
	}

	void CProject::TouchModule( SModule &Mod )
	{
		if( m_pCurrentCompilation != nullptr && Mod.SourceLink.List() != &m_pCurrentCompilation->Modules ) {
			m_pCurrentCompilation->Modules.AddTail( Mod.SourceLink );
		}

		if( Mod.ProjectLink.List() != &m_Modules ) {
			m_Modules.AddTail( Mod.ProjectLink );
		}
	}

	bool SCompilation::Build()
	{
		// Skip dummy source
		if( SourceFilename.IsEmpty() ) {
			return true;
		}

		// Parse the source
		CParser Parser;

		if( CG->IsInitialized() ) {
			CG->Fini();
		}

		CG->Init();

		if( !Parser.LoadFile( SourceFilename ) ) {
			Ax::Errorf( SourceFilename, "Failed to load file text" );
			return false;
		}

		unsigned cSuccesses = 0;

		if( !ASListFilename.IsEmpty() ) {
			Ax::g_VerboseLog( SourceFilename ) += "Assembly listing: " + ASListFilename;

			if( !CG->AddAsmOut( LLVMStr( ASListFilename ) ) ) {
				Ax::Errorf( ASListFilename, "Failed to add assembly listing" );
				return false;
			}

			++cSuccesses;
		}
		if( !ObjectFilename.IsEmpty() ) {
			Ax::g_VerboseLog( SourceFilename ) += "Object file: " + ObjectFilename;

			if( !CG->AddObjOut( LLVMStr( ObjectFilename ) ) ) {
				Ax::Errorf( ObjectFilename, "Failed to add object file output" );
				return false;
			}

			++cSuccesses;
		}

		if( !Parser.ParseProgram() ) {
			Ax::Errorf( SourceFilename, "Compilation failed (Syntax)" );
			return false;
		}

		if( !Parser.Semant() ) {
			Ax::Errorf( SourceFilename, "Compilation failed (Semantics)" );
			return false;
		}

		if( !Parser.CodeGen() ) {
			Ax::Errorf( SourceFilename, "Compilation failed (Translation)" );
			return false;
		}

		if( !IRListFilename.IsEmpty() ) {
			Ax::g_VerboseLog( SourceFilename ) += "IR listing: " + IRListFilename;

			if( !CG->WriteIR( LLVMStr( IRListFilename ) ) ) {
				Ax::Warnf( IRListFilename, "Failed to generate IR listing" );
			} else {
				++cSuccesses;
			}
		}

		if( !LLVMBCFilename.IsEmpty() ) {
			Ax::g_VerboseLog( SourceFilename ) += "Bitcode dump: " + LLVMBCFilename;

			if( !CG->WriteBC( LLVMStr( LLVMBCFilename ) ) ) {
				Ax::Warnf( LLVMBCFilename, "Failed to generate bitcode listing" );
			} else {
				++cSuccesses;
			}
		}

		CG->WriteOutputs();

		return cSuccesses > 0;
	}

}}

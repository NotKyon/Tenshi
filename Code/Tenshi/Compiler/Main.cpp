#include "_PCH.hpp"

#include "Tester.hpp"
#include "Options.hpp"
#include "Project.hpp"
#include "Module.hpp"

#include "Shell.hpp"
#include "Binutils.hpp"

using namespace Tenshi;
using namespace Tenshi::Compiler;

//
//	TODO: Remove the options system and use LLVM's. (Theirs is better
//	-     implemented.)
//

//
//	TODO: Need to work on project system. Settings and variables should be
//	-     pushed to there and reasoned about there. That way we don't need to
//	-     do things differently based on whether files came from the command
//	-     line or through a project file. This additionally makes interaction
//	-     with subsystems easier. ("Environment" and per-module build settings.)
//

namespace Tenshi { namespace Compiler {

	class CHelpOption: public IOption
	{
	public:
		CHelpOption(): IOption()
		{
		}
		virtual ~CHelpOption()
		{
		}

		char GetShortName() const AX_OVERRIDE			{ return 'h'; }
		const char *GetLongName() const AX_OVERRIDE		{ return "help"; }

		const char *GetBriefHelp() const AX_OVERRIDE	{ return "Displays this help message."; }

		EOptionArg GetArgumentType() const AX_OVERRIDE	{ return EOptionArg::String; }
		bool IsArgumentRequired() const AX_OVERRIDE		{ return false; }

		bool ShouldShowInHelp() const AX_OVERRIDE		{ return true; }

		bool OnCall( UOptionArg Arg ) AX_OVERRIDE
		{
			AX_ASSERT_MSG( !Arg.pszValue || Arg.EnumValue > 0x1000, "Invalid string pointer" );

			if( !Arg.pszValue ) {
				ShowUsage();
				Ax::BasicStatusf( "Options:" );
			}

			for( const IOption *pOpt : Opts->GetOptions() ) {
				AX_ASSERT_NOT_NULL( pOpt );

				if( !pOpt->ShouldShowInHelp() ) {
					continue;
				}

				if( Arg.pszValue != nullptr ) {
					if( !Ax::Cmp( Arg.pszValue, pOpt->GetLongName() ) ) {
						continue;
					}

					ShowDetailedHelp( *pOpt );
					return true;
				}

				ShowBriefHelp( *pOpt );
			}

			if( Arg.pszValue != nullptr ) {
				Ax::BasicErrorf( "Unknown option for help '%s'", Arg.pszValue );
				return false;
			}

			return true;
		}

	private:
		static Ax::uintptr NumColumns()
		{
#ifdef _WIN32
			static const DWORD TryHandles[] = { STD_ERROR_HANDLE, STD_OUTPUT_HANDLE };

			CONSOLE_SCREEN_BUFFER_INFO csbi;

			for( DWORD dwHandle : TryHandles ) {
				if( !GetConsoleScreenBufferInfo( GetStdHandle( dwHandle ), &csbi ) ) {
					continue;
				}

				if( !csbi.dwSize.X ) {
					continue;
				}

				return ( Ax::uintptr )csbi.dwSize.X;
			}
#endif

			return 80;
		}

		void ShowUsage()
		{
#if AX_DEBUG_ENABLED
# define ARGV0 "TenshiCompilerDbg"
#else
# define ARGV0 "TenshiCompiler"
#endif
			Ax::BasicStatusf( "USAGE: " ARGV0 " [options] [input files...]" );
#undef ARGV0
		}
		void ShowBriefHelp( const IOption &Opt )
		{
			static const char pszSpaces[] =
				"                                                            "
				"                                                            ";
			const Ax::uintptr cColumns = NumColumns();
			const Ax::uintptr kColumn = 28;

			static_assert( kColumn + 1 < sizeof( pszSpaces ), "Column is too far" );

			const char chShortOpt = Opt.GetShortName();
			const char *const pszLongOpt = Opt.GetLongName();
			const EOptionArg ArgTy = Opt.GetArgumentType();

			AX_ASSERT_NOT_NULL( pszLongOpt );

			char szBuf[ 2048 ];

			szBuf[ 0 ] = '\0';
			Ax::uintptr index = 0;

			szBuf[ index++ ] = ' ';
			szBuf[ index++ ] = ' ';
			if( chShortOpt != '\0' ) {
				szBuf[ index++ ] = '-';
				szBuf[ index++ ] = chShortOpt;
				szBuf[ index++ ] = ',';

				szBuf[ index ] = '\0';
			}
			Ax::AppendString( szBuf, index, "--" );
			if( ArgTy == EOptionArg::Boolean ) {
				Ax::AppendString( szBuf, index, "[no-]" );
			}
			Ax::AppendString( szBuf, index, pszLongOpt );
			switch( ArgTy )
			{
			case EOptionArg::Directory:
				Ax::AppendString( szBuf, index, "=DIR" );
				break;

			case EOptionArg::File:
			case EOptionArg::OutputFile:
				Ax::AppendString( szBuf, index, "=FILE" );
				break;

			case EOptionArg::Integer:
			case EOptionArg::RangedInteger:
				Ax::AppendString( szBuf, index, "=N" );
				break;

			case EOptionArg::String:
				Ax::AppendString( szBuf, index, "=TEXT" );
				break;

			default:
				break;
			}

			const Ax::uintptr cItems = ArgTy == EOptionArg::Enum ? Opt.NumEnumItems() : 0;
			const char *const *ppszItems = ArgTy == EOptionArg::Enum ? Opt.GetEnumItems() : nullptr;
			Ax::uintptr uItem = 0;

			// Want brief text to be aligned, so skip to next column if necessary
			if( index >= kColumn ) {
				Ax::AppendString( szBuf, index, "\n" );
				Ax::AppendString( szBuf, index, pszSpaces, kColumn );
			} else {
				Ax::AppendString( szBuf, index, pszSpaces, kColumn - index );
			}

			// The number of columns for the brief text
			const Ax::uintptr cBriefColumns = cColumns - kColumn;

			// The brief text for the command
			const char *const pszBriefHelp = Opt.GetBriefHelp();
			AX_ASSERT_NOT_NULL( pszBriefHelp );

			// Word break
			const char *p = pszBriefHelp;
			while( *p != '\0' ) {
				// Find the last space within the current column
				const char *const s = p;
				const char *q = p;
				while( Ax::uintptr( q - s ) < cBriefColumns ) {
					if( *( const unsigned char * )q <= ' ' ) {
						p = q;
						if( *p == '\0' ) {
							break;
						}
					}

					++q;
				}

				// If we didn't increment at all then the word is too long, so
				// we don't care. (TODO: Break the word with a "-")
				if( s == p && *p != '\0' ) {
					// There cannot be a NUL character within the space we're incrementing past
					AX_ASSERT( ( Ax::uintptr )( strchr( p, '\0' ) - s ) < cBriefColumns );
					p += cBriefColumns;
				}

				// Append the words that did fit in the column
				Ax::AppendString( szBuf, index, s, p - s );

				// Skip trailing spaces
				while( *( const unsigned char * )p <= ' ' && *p != '\0' ) {
					++p;
				}

				// If we haven't reached the end then there's more to break up
				if( *p != '\0' ) {
					// If we are at the very edge of the column then there's no need for a line break
					if( ( Ax::uintptr )( p - s ) != cBriefColumns ) {
						Ax::AppendString( szBuf, index, "\n", 1 );
					}

					// Space out to the next column
					Ax::AppendString( szBuf, index, pszSpaces, kColumn );
				}
			}

			// == TODO :: Display the possible enumerants == //
			( void )cItems;
			( void )ppszItems;
			( void )uItem;

			// Output the text
			Ax::g_InfoLog += szBuf;
		}
		void ShowDetailedHelp( const IOption &Opt )
		{
			Ax::Errorf( __FILE__, __LINE__, "TODO: Show detailed help for \"%s\"", Opt.GetLongName() );
		}
	};

	class CCompileOnlyOption: public IOption
	{
	public:
		CCompileOnlyOption()
		: IOption()
		, m_bCompileOnly( false )
		{
		}
		virtual ~CCompileOnlyOption()
		{
		}
		
		char GetShortName() const AX_OVERRIDE			{ return 'c'; }
		const char *GetLongName() const AX_OVERRIDE		{ return "compile"; }

		const char *GetBriefHelp() const AX_OVERRIDE	{ return "Compile only; do not link."; }

		EOptionArg GetArgumentType() const AX_OVERRIDE	{ return EOptionArg::None; }
		bool ShouldShowInHelp() const AX_OVERRIDE		{ return true; }

		bool OnCall( UOptionArg ) AX_OVERRIDE
		{
			m_bCompileOnly = true;
			return true;
		}

		bool IsSet() const
		{
			return m_bCompileOnly;
		}

	private:
		bool						m_bCompileOnly;
	};

	class COutputOption: public virtual IOption
	{
	public:
		COutputOption()
		: IOption()
		, m_bSpecified( false )
		, m_OutputFile()
		{
		}
		virtual ~COutputOption()
		{
		}

		char GetShortName() const AX_OVERRIDE			{ return 'o'; }
		const char *GetLongName() const AX_OVERRIDE		{ return "output-file"; }

		const char *GetBriefHelp() const AX_OVERRIDE	{ return "Set the name of the output file."; }

		EOptionArg GetArgumentType() const AX_OVERRIDE	{ return EOptionArg::OutputFile; }
		bool ShouldShowInHelp() const AX_OVERRIDE		{ return true; }

		bool OnCall( UOptionArg Arg ) AX_OVERRIDE
		{
			AX_ASSERT_NOT_NULL( Arg.pszValue );
			AX_ASSERT_MSG( Arg.EnumValue > 0x1000, "Invalid string pointer" );

			if( m_bSpecified ) {
				Ax::g_WarningLog += "Changing output file from \"" + m_OutputFile + "\"";
			}

			m_bSpecified = true;
			AX_EXPECT_MSG( m_OutputFile.Assign( Arg.pszValue ), "Out of memory" );

			return true;
		}

		bool HasOutputFile() const
		{
			return m_bSpecified;
		}
		const Ax::String &GetOutputFile() const
		{
			return m_OutputFile;
		}

	private:
		bool						m_bSpecified;
		Ax::String					m_OutputFile;
	};

}}

bool ProcessFile( const char *pszInputFile )
{
	AX_ASSERT_NOT_NULL( pszInputFile );

	const char *const pszExt = strrchr( pszInputFile, '.' );
	if( pszExt != nullptr ) {
		if( Ax::CaseCmp( pszExt, ".teproj" ) || Ax::CaseCmp( pszExt, ".tenshiproject" ) ) {
			if( !Projects->Load( pszInputFile ) ) {
				return false;
			}
		}
	} else {
		if( !Projects->Current().ApplyLine( "(command-line)", 1, "Compile " + Ax::String( pszInputFile ).Escape().Quote() ) ) {
			return false;
		}
	}

	return true;
}

int main( int argc, char **argv )
{
#ifdef _WIN32
	SendMessageW( GetConsoleWindow(), WM_SETICON, 0, ( LPARAM )LoadIconW( GetModuleHandleW(nullptr), ( LPCWSTR )1 ) );
#endif
	Ax::InstallConsoleReporter();

	CHelpOption HelpOpt;
	CCompileOnlyOption CompOnlyOpt;
	COutputOption OutputOpt;

	Opts->Register( HelpOpt );
	Opts->Register( CompOnlyOpt );
	Opts->Register( OutputOpt );

	int ExitStatus = EXIT_SUCCESS;
	bool bProcessArgs = true;

	if( !Binutils->Init() ) {
		Ax::BasicErrorf( "Could not initialize binutils" );
		ExitStatus = EXIT_FAILURE;
	}

#if AX_DEBUG_ENABLED
	Ax::BasicDebugf( "TenshiCompiler (DEBUG) - Built on %s at %s", __DATE__, __TIME__ );

	if( argc <= 1 ) {
		char szAppDir[ 512 ];
		if( !Ax::System::GetAppDir( szAppDir ) ) {
			Ax::BasicErrorf( "Failed to retrieve app-path" );
			ExitStatus = EXIT_FAILURE;
		} else {
			const Ax::String TestsPath = Ax::String( szAppDir ) / "../../Code/Tenshi/Compiler/Tests";

			Tenshi::Compiler::Tester->CollectTests( TestsPath );
			if( !Tenshi::Compiler::Tester->RunTests() ) {
				ExitStatus = EXIT_FAILURE;
			}
		}

		bProcessArgs = false;
	}
#endif

	if( bProcessArgs ) {
		if( !Opts->Process( argc, argv ) ) {
			return EXIT_FAILURE;
		}
	}

	if( ExitStatus != EXIT_SUCCESS ) {
		return ExitStatus;
	}

	bool bDidAllSucceed = false;
	for( const char *pszInputFile : Opts->GetInputs() ) {
		if( !ProcessFile( pszInputFile ) ) {
			Ax::Errorf( pszInputFile, "Failed to process file; ignoring all other inputs" );
			ExitStatus = EXIT_FAILURE;
			break;
		}

		bDidAllSucceed = true;
	}

	if( bDidAllSucceed ) {
		if( OutputOpt.HasOutputFile() ) {
			Projects->Current().ApplyLine( "(command-line)", 1, "-TargetPrefix" );
			Projects->Current().ApplyLine( "(command-line)", 1, "-TargetSuffix" );
			Projects->Current().ApplyLine( "(command-line)", 1, "Target " + OutputOpt.GetOutputFile().Escape().Quote() );
		}

		if( CompOnlyOpt.IsSet() ) {
			AX_DEBUG_LOG += "Compile-only not yet implemented";
		}

		Mods->LoadCoreInternal();
		Mods->LoadCorePlugins();

		if( !Projects->Build() ) {
			ExitStatus = EXIT_FAILURE;
		}
	}

	return ExitStatus;
}

#if defined( _WIN32 )
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "Logger.hpp"

#include "../Core/String.hpp"
#include "../Collections/List.hpp"
#include "../System/AppPath.hpp"

namespace Ax
{

	// Get the reporters list
	static TList< IReporter * > &ReportersList()
	{
		static TList< IReporter * > instance;
		return instance;
	}

	// Submit a report to all listening reporters
	void Report( const SReportDetails &details, const char *message )
	{
		AX_ASSERT_NOT_NULL( message );

		auto &reporters = ReportersList();

		bool found = false;
		for( IReporter *&r : reporters ) {
			if( !r ) {
				continue;
			}
			
			found = true;
			r->Report( details, message );
		}
		
		if( !found ) {
			const ESeverity sev = details.Severity;
			const int From = details.From;
			const char *const pszFile = details.pszFile;
			const uint32 uLine = pszFile != nullptr ? details.uLine : 0;
			const uint32 uColumn = uLine > 0 ? details.uColumn : 0;
			const char *const pszFunction = pszFile != nullptr ? details.pszFunction : nullptr;

			const char *const sevMsg =
				sev == ESeverity::Error   ? "Error"   :
				sev == ESeverity::Warning ? "Warning" :
				sev == ESeverity::Hint    ? "Hint"    :
				sev == ESeverity::Debug   ? "Debug"   :
				sev == ESeverity::Normal  ? "Normal"  :
				"(unknown-Severity)";
			fprintf
			(
				  stderr
				, "%s from:%i file<%s> line:%u column:%u function<%s> :: %s\n"
				, sevMsg
				, From
				, pszFile != nullptr ? pszFile : ""
				, uLine
				, uColumn
				, pszFunction != nullptr ? pszFunction : ""
				, message
			);
		}
	}
	// Add a reporter interface for handling reports
	void AddReporter( IReporter *r )
	{
		if( !r ) {
			return;
		}

		auto &reporters = ReportersList();

		for( IReporter *&x : reporters ) {
			if( x == r ) {
				return;
			}
		}
		
		reporters.AddTail( r );
	}
	// Remove an added reporter interface
	void RemoveReporter( IReporter *r )
	{
		if( !r ) {
			return;
		}
		
		auto &reporters = ReportersList();

		auto end = reporters.end();
		auto start = reporters.begin();
		for( auto iter = start; iter != end; ++iter ) {
			if( *iter == r ) {
				reporters.Remove( iter );
				return;
			}
		}
	}

	/*
	===========================================================================
	
		UTILITY :: DEBUG LOG REPORTER (REPORTER IMPLEMENTATION)

		Writes reports to "debug.log"
	
	===========================================================================
	*/
	class DebugLogReporter: public virtual IReporter
	{
	public:
		// Register this reporter with the internal reporting system
		static void Install()
		{
			AddReporter( &GetInstance() );
		}
		// Remove this reporter From the internal reporting system
		static void Uninstall()
		{
			RemoveReporter( &GetInstance() );
		}

		// UNDOC: Handle a report
		virtual void Report( const SReportDetails &details, const char *message )
		{
#ifdef __STDC_WANT_SECURE_LIB__
			FILE *fp = nullptr;
			if( fopen_s( &fp, "debug.log", "a+" ) != 0 ) {
				return;
			}
#else
			FILE *fp = fopen( "debug.log", "a+" );
#endif
			if( !fp ) {
				return;
			}

			const ESeverity sev = details.Severity;
			const int From = details.From;
			const char *const pszFile = details.pszFile;
			const uint32 uLine = pszFile != nullptr ? details.uLine : 0;
			const uint32 uColumn = uLine > 0 ? details.uColumn : 0;
			const char *const pszFunction = pszFile != nullptr ? details.pszFunction : nullptr;

			( void )From; //not handling where a report comes From, currently
			
			const char *sevMsg = nullptr;
			switch( sev )
			{
				case ESeverity::Verbose:
					break;
				case ESeverity::Normal:
					break;
					
				case ESeverity::Debug:
					sevMsg = "***DEBUG*** ";
					break;
				case ESeverity::Hint:
					sevMsg = "Hint ";
					break;
				case ESeverity::Warning:
					sevMsg = "Warning ";
					break;
				case ESeverity::Error:
					sevMsg = "Error ";
					break;
			}

			const char *const x = pszFunction != nullptr ? "in " : "";
			const char *const y = pszFunction != nullptr ? pszFunction : "";
			const char *const z = pszFunction != nullptr ? ": " : "";
			
			if( pszFile != nullptr ) {
				if( uLine > 0 ) {
					if( uColumn > 0 ) {
						fprintf( fp, "[%s(%u:%u)]\n\t%s%s%s%s%s\n\n",
							pszFile, uLine, uColumn, sevMsg, x, y, z, message );
					} else {
						fprintf( fp, "[%s(%u)]\n\t%s%s%s%s%s\n\n",
							pszFile, uLine, sevMsg, x, y, z, message );
					}
				} else {
					fprintf( fp, "[%s]\n\t%s%s%s%s%s\n\n",
						pszFile, sevMsg, x, y, z, message );
				}
			} else {
				fprintf( fp, "%s%s%s%s%s\n", sevMsg, x, y, z, message );
			}
			
			fclose( fp );
		}

	private:
		// UNDOC: Singleton
		static DebugLogReporter &GetInstance()
		{
			static DebugLogReporter instance;
			return instance;
		}

		// UNDOC: Constructor
		DebugLogReporter(): IReporter()
		{
		}
		// UNDOC: Destructor
		virtual ~DebugLogReporter()
		{
			RemoveReporter( this );
		}
	};

	void InstallDebugLogReporter()
	{
		DebugLogReporter::Install();
	}
	void UninstallDebugLogReporter()
	{
		DebugLogReporter::Uninstall();
	}

	/*
	===========================================================================
	
		UTILITY :: CONSOLE REPORTER (REPORTER IMPLEMENTATION)

		Implements colored console output for reports
	
	===========================================================================
	*/
	class ConsoleReporter: public virtual IReporter
	{
	public:
		// Register this reporter with the internal reporting system
		static void Install()
		{
			AddReporter( &GetInstance() );
		}
		// Remove this reporter From the internal reporting system
		static void Uninstall()
		{
			RemoveReporter( &GetInstance() );
		}

#define VS_STYLE_REPORT 1

		// UNDOC: Handle a report
		virtual void Report( const SReportDetails &details, const char *message )
		{
			const ESeverity sev = details.Severity;
			const int From = details.From;
			const char *const pszFile = details.pszFile;
			const uint32 uLine = pszFile != nullptr ? details.uLine : 0;
			const uint32 uColumn = uLine > 0 ? details.uColumn : 0;
			const char *const pszFunction = pszFile != nullptr ? details.pszFunction : nullptr;

			( void )From; //not handling where a report is From, currently

			const unsigned char curCol = GetCurrentColors( mStdErr );
			const unsigned char bgCol = curCol & 0xF0;

			Color sevCol = kColor_White;
			const char *sevMsg = nullptr;
			
			switch( sev ) {
			case ESeverity::Verbose:
				break;
			case ESeverity::Normal:
				break;
				
			case ESeverity::Debug:
				sevCol = kColor_LightMagenta;
				sevMsg = "***DEBUG***";
				break;
			case ESeverity::Warning:
				sevCol = kColor_LightBrown;
				sevMsg = "WARNING";
				break;
			case ESeverity::Hint:
				sevCol = kColor_LightGreen;
				sevMsg = "HINT";
				break;
			case ESeverity::Error:
				sevCol = kColor_LightRed;
				sevMsg = "ERROR";
				break;
			}
			
#if !VS_STYLE_REPORT
			if( sevMsg != nullptr ) {
				SetCurrentColors( mStdErr, sevCol | bgCol );
				WriteString( mStdErr, sevMsg );

				SetCurrentColors( mStdErr, curCol );
				WriteString( mStdErr, ": " );
			}
#endif

			if( pszFile != nullptr ) {
#if !VS_STYLE_REPORT
				SetCurrentColors( mStdErr, kColor_Cyan | bgCol );
				WriteString( mStdErr, "[" );
#endif

				SetCurrentColors( mStdErr, kColor_White ); //black bg
				WriteString( mStdErr, pszFile );
				
				if( uLine > 0 ) {
					SetCurrentColors( mStdErr, kColor_DarkGray | bgCol );
					WriteString( mStdErr, "(" );

					static const int bufn = 32;
					char buf[ bufn ];
					char *p = &buf[ bufn - 1 ];
					*p = '\0';

					uint32 n = uLine;
					while( n > 0 ) {
						AX_ASSERT( p > &buf[ 0 ] );

						*--p = ( char )( '0' + n%10 );
						n /= 10;
					}
					
					SetCurrentColors( mStdErr, kColor_LightGray ); //black bg
					WriteString( mStdErr, p ); //uLine number

					if( uColumn > 0 ) {
						p = &buf[ bufn - 1 ];

						n = uColumn;
						while( n > 0 ) {
							AX_ASSERT( p > &buf[ 0 ] );

							*--p = ( char )( '0' + n%10 );
							n /= 10;
						}

						SetCurrentColors( mStdErr, kColor_DarkGray );
						WriteString( mStdErr, "," );

						SetCurrentColors( mStdErr, kColor_LightGray );
						WriteString( mStdErr, p ); //uColumn number
					}

					SetCurrentColors( mStdErr, kColor_DarkGray | bgCol );
					WriteString( mStdErr, ")" );
				}

#if !VS_STYLE_REPORT
				SetCurrentColors( mStdErr, kColor_Cyan | bgCol );
				WriteString( mStdErr, "] " );
#else
				SetCurrentColors( mStdErr, curCol );
				WriteString( mStdErr, ": " );
#endif
			}

#if VS_STYLE_REPORT
			if( sevMsg != nullptr ) {
				SetCurrentColors( mStdErr, sevCol | bgCol );
				WriteString( mStdErr, sevMsg );

				SetCurrentColors( mStdErr, curCol );
				WriteString( mStdErr, ": " );
			}
#endif

			if( pszFunction != nullptr ) {
				SetCurrentColors( mStdErr, curCol );
				WriteString( mStdErr, "in " );

				SetCurrentColors( mStdErr, kColor_White );
				WriteString( mStdErr, pszFunction );

				SetCurrentColors( mStdErr, curCol );
				WriteString( mStdErr, ": " );
			}

			SetCurrentColors( mStdErr, curCol );
			WriteString( mStdErr, message );
			WriteString( mStdErr, "\n" );

#if defined( _WIN32 )
			if( sev != ESeverity::Error ) {
				return;
			}

			char buf[ 1024 ];
# if !defined( _MSC_VER )
#  define sprintf_s snprintf
#  define strcat_s(Dst,DstN,Src) strncat(Dst,Src,DstN)
# endif

			if( pszFile != nullptr ) {
				if( uLine > 0 ) {
					sprintf_s( buf, sizeof( buf ),
						"File: %s\nLine: %u\n\n%s",
						pszFile, uLine, message );
				} else {
					sprintf_s( buf, sizeof( buf ), "[File: %s]\n%s",
						pszFile, message );
				}
			} else {
				sprintf_s( buf, sizeof( buf ), "%s", message );
			}
			buf[ sizeof( buf ) - 1 ] = '\0';

			char exePath[ 512 ];
			System::GetAppPath( exePath );

# pragma warning( push )
# pragma warning( suppress: 6054 )
			char *namePart = strrchr( exePath, '\\' );
# pragma warning( pop )
			if( namePart != nullptr ) {
				++namePart;
			}

			char *const extPart = namePart != nullptr ? strrchr( namePart, '.' ) : nullptr;
			if( extPart != nullptr ) {
				*extPart = '\0';
			}

			char title[ 512 ];
			sprintf_s( title, sizeof( title ), "Error - %s", namePart );

			const HWND wnd = NULL;
			const UINT icon = MB_ICONERROR;

			if( IsDebuggerPresent() ) {
				strcat_s( buf, sizeof( buf ), "\n\nTrigger breakpoint?" );

				const int r = MessageBoxA( wnd, buf, title, icon | MB_YESNO );

				if( r == IDYES ) {
					DebugBreak();
				}
			}
#endif
		}

	private:
		// Color codes used internally
		//
		// NOTE: If used as-is then these represent foreground (text) colors
		//       with a black background color.
		enum Color : unsigned char
		{
			kColor_Black		= 0x0,
			kColor_Blue			= 0x1,
			kColor_Green		= 0x2,
			kColor_Cyan			= 0x3,
			kColor_Red			= 0x4,
			kColor_Magenta		= 0x5,
			kColor_Brown		= 0x6,
			kColor_LightGrey	= 0x7,
			kColor_DarkGrey		= 0x8,
			kColor_LightBlue	= 0x9,
			kColor_LightGreen	= 0xA,
			kColor_LightCyan	= 0xB,
			kColor_LightRed		= 0xC,
			kColor_LightMagenta	= 0xD,
			kColor_LightBrown	= 0xE,
			kColor_White		= 0xF,
			
			kColor_Purple		= kColor_Magenta,
			kColor_Yellow		= kColor_LightBrown,
			kColor_Violet		= kColor_LightMagenta,

			kColor_LightGray	= kColor_LightGrey,
			kColor_DarkGray		= kColor_DarkGrey
		};

#if defined( _WIN32 )
		typedef HANDLE FileHandle;
#else
		typedef FILE *FileHandle;
#endif
		FileHandle mStdOut;
		FileHandle mStdErr;

		// UNDOC: Singleton
		static ConsoleReporter &GetInstance()
		{
			static ConsoleReporter instance;
			return instance;
		}

		// UNDOC: Constructor
		ConsoleReporter(): IReporter(), mStdOut( NULL ), mStdErr( NULL )
		{
#if defined( _WIN32 )
			mStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
			if( mStdOut == INVALID_HANDLE_VALUE ) {
				mStdOut = ( FileHandle )0;
			}

			mStdErr = GetStdHandle( STD_ERROR_HANDLE );
			if( mStdErr == INVALID_HANDLE_VALUE ) {
				mStdErr = ( FileHandle )0;
			}
#else
			mStdOut = stdout;
			mStdErr = stderr;
#endif

			AddReporter( this );
		}
		// UNDOC: Destructor
		virtual ~ConsoleReporter()
		{
			RemoveReporter( this );
		}
		
		// No copy constructor
		ConsoleReporter( const ConsoleReporter & );
		// No assignment operator
		ConsoleReporter &operator=( const ConsoleReporter & );

		// Retrieve the current colors of the given console handle
		static unsigned char GetCurrentColors( FileHandle f )
		{
			if( !f ) {
				return 0x07;
			}
			
#if defined( _WIN32 )
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			
			if( GetConsoleScreenBufferInfo( f, &csbi ) ) {
				return csbi.wAttributes & 0xFF;
			}
#else
			//
			//	TODO: Support this on GNU/Linux...
			//
#endif

			return 0x07;
		}
		// Set the current colors for the given console handle
		static void SetCurrentColors( FileHandle f, unsigned char colors )
		{
			if( !f ) {
				return;
			}

#if defined( _WIN32 )
			SetConsoleTextAttribute( f, ( WORD )colors );
#else
			//
			//	MAP ORDER (0-7):
			//	Black, Blue, Green, Cyan, Red, Magenta, Yellow, Grey
			//
			//	TERMINAL COLOR ORDER (0-7):
			//	Black, Red, Green, Yellow, Blue, Magenta, Cyan, Grey
			//
			static const char *const mapF[16] = {
				"\x1b[30;22m", "\x1b[34;22m", "\x1b[32;22m", "\x1b[36;22m",
				"\x1b[31;22m", "\x1b[35;22m", "\x1b[33;22m", "\x1b[37;22m",
				"\x1b[30;1m", "\x1b[34;1m", "\x1b[32;1m", "\x1b[36;1m",
				"\x1b[31;1m", "\x1b[35;1m", "\x1b[33;1m", "\x1b[37;1m"
			};
			
			const char *const code = mapF[ colors & 0x0F ];
			fwrite( ( const void * )code, strlen( code ), 1, f );
#endif
		}
		// Write a string of text to the given console handle
		static void WriteString( FileHandle f, const char *s )
		{
#if defined( _WIN32 )
			//
			//	FIXME: WriteFile() causes crash in some cases
			//
			
			FILE *fp = stdout;
			if( f == GetStdHandle( STD_ERROR_HANDLE ) ) {
				fp = stderr;
			}
			
			fprintf( fp, "%s", s );
#else
			fprintf( f, "%s", s );
#endif
		}
	};

	void InstallConsoleReporter()
	{
		ConsoleReporter::Install();
	}
	void UninstallConsoleReporter()
	{
		ConsoleReporter::Uninstall();
	}

}

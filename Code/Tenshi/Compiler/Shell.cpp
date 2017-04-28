#include "_PCH.hpp"
#include "Shell.hpp"

namespace Tenshi { namespace Compiler {

	MShell &MShell::GetInstance()
	{
		static MShell instance;
		return instance;
	}

	MShell::MShell()
	: m_bLoggingEnabled( !!AX_DEBUG_ENABLED )
	, m_Filters()
	, m_Commands()
	, m_CommandMap()
	{
		AX_EXPECT_MEMORY( m_CommandMap.Init( TENSHI_SHELLCOMMAND_ALLOWED, Ax::ECase::Insensitive ) );

#ifdef _WIN32
		if( GetConsoleOutputCP() != CP_UTF8 ) {
			SetConsoleOutputCP( CP_UTF8 );
		}
#endif
	}
	MShell::~MShell()
	{
	}
	
	bool MShell::GetWindowsSafeCommandLine( Ax::String &InoutString, const Ax::TArray< Ax::String > &Args ) const
	{
		// Save some time by requesting most of our needed memory up-front
		//
		// If this fails it's fine as it's only meant as a performance
		// optimization. (Keep us from re-allocating all the time.)
		InoutString.Reserve( InoutString.Len() + Args.Num()*128 );

		// Process each argument
		for( Ax::uintptr i = 0; i < Args.Num(); ++i ) {
			// Reference to the current argument in the array
			const Ax::String &Arg = Args[ i ];

			// If this is not the first argument to the string then add a space
			if( !InoutString.IsEmpty() ) {
				if( !InoutString.Append( ' ' ) ) {
					return false;
				}
			}

			// Determine whether surrounding quotes are needed
			const bool bNeedQuotes = ( Arg.Find( ' ' ) >= 0 || Arg.Find( '\t' ) >= 0 || Arg.IsEmpty() );

			// Add an opening quote if quotes are necessary
			if( bNeedQuotes && !InoutString.Append( '\"' ) ) {
				return false;
			}

			// Current parsing pointer for the argument
			const char *p = Arg.CString();
			// Base pointer (for adding a series of characters all at once)
			const char *b = p;

			// Parse this argument
			for(;;) {
				// Backslashes might need special handling
				if( *p == '\\' ) {
					// Flush the current range of text
					if( !InoutString.Append( b, p ) ) {
						return false;
					}

					// Number of backslashes
					size_t n = 0;
					// Starting location of the first backslash
					const char *const t = p;

					// Count the backslashes
					while( *p == '\\' ) {
						++n;
						++p;
					}

					// Backslashes followed by a quotation mark is handled specially
					if( *p == '\"' ) {
						// Eat the quote
						++p;

						// Add a pair of backslashes for each of the counted
						// backslashes. (Windows will treat each two as one
						// because they are followed by a double quote.)
						while( n > 0 ) {
							--n;
							if( !InoutString.Append( "\\\\" ) ) {
								return false;
							}
						}

						// Escape the quote
						if( !InoutString.Append( "\\\"" ) ) {
							return false;
						}
					} else {
						// Turns out special handling wasn't necessary, so just
						// copy the backslashes verbatim
						if( !InoutString.Append( t, p ) ) {
							return false;
						}
					}

					// Reset the base pointer and resume the loop
					b = p;
					continue;
				}

				// Quotation marks need to be escaped
				if( *p == '\"' ) {
					// Flush the current range of text
					if( !InoutString.Append( b, p ) ) {
						return false;
					}
					// Escape the quote
					if( !InoutString.Append( "\\\"" ) ) {
						return false;
					}

					// Set the base and pointer to one after the quote
					b = ++p;
					continue;
				}

				// If the end of the string has been reached then exit the loop
				if( *p == '\0' ) {
					// Flush the current range of text before exiting
					if( !InoutString.Append( b, p ) ) {
						return false;
					}

					break;
				}

				// This character was nothing special, so skip it (it will be
				// flushed to CommandArgs later)
				++p;
			}

			// Add a closing quote if necessary
			if( bNeedQuotes && !InoutString.Append( '\"' ) ) {
				return false;
			}
		}

		// Done!
		return true;
	}

	void MShell::LogCommand( const Ax::TArray< Ax::String > &Args, const char *pszPrefix ) const
	{
		if( !m_bLoggingEnabled ) {
			return;
		}

		Ax::String CommandLine;
		Ax::uintptr cReserveBytes = 0;
		for( const Ax::String &Arg : Args ) {
			cReserveBytes += Arg.Len() + 8;
		}
		AX_EXPECT_MEMORY( CommandLine.Reserve( cReserveBytes ) );
		for( const Ax::String &Arg : Args ) {
			if( CommandLine.IsEmpty() ) {
				AX_EXPECT_MEMORY( CommandLine.Append( ' ' ) );
			}

			if( Arg.Find( ' ' ) >= 0 || Arg.Find( '\t' ) >= 0 || Arg.Find( '\"' ) >= 0 || Arg.Find( '\\' ) >= 0 ) {
				AX_EXPECT_MEMORY( CommandLine.Append( Arg.Escape().Quote() ) );
			} else {
				AX_EXPECT_MEMORY( CommandLine.Append( Arg ) );
			}
		}

		LogCommand( CommandLine, pszPrefix );
	}
	void MShell::LogCommand( const Ax::String &CommandLine, const char *pszPrefix ) const
	{
		if( !m_bLoggingEnabled ) {
			return;
		}

		if( !pszPrefix ) {
			pszPrefix = ">";
		}

		Ax::BasicStatusf( "%s%s", pszPrefix, CommandLine.CString() );
	}

	int MShell::Run( const Ax::TArray< Ax::String > &Args )
	{
		if( Args.IsEmpty() || Args[ 0 ].IsEmpty() ) {
			return EINVAL;
		}

		auto *const pEntry = m_CommandMap.Find( Args[ 0 ] );
		if( pEntry != nullptr && pEntry->pData != nullptr ) {
			LogCommand( Args, "$builtin>" );
			return pEntry->pData->Run( Args );
		}

#ifdef _WIN32
		Ax::String AppName;
		Ax::String CommandArgs;
		Ax::TArray< Ax::uint16 > AppNameW;
		Ax::TArray< Ax::uint16 > CommandArgsW;

		if( !GetWindowsSafeCommandLine( CommandArgs, Args ) ) {
			Ax::Errorf( Args[ 0 ], "Ran out of memory in MShell::GetWindowsSafeCommandLine()" );
			return ENOMEM;
		}

		if( Args[ 0 ].CaseEndsWith( ".bat" ) || Args[ 0 ].CaseEndsWith( ".cmd" ) ) {
			if( !AppName.Assign( "cmd.exe" ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while selecting command prompt to execute file" );
				return ENOMEM;
			}

			Ax::String Shuffle;
			Ax::TArray< Ax::String > ShuffleArgs;

			bool bWin = true;
			bWin &= ShuffleArgs.Append();
			bWin &= ShuffleArgs.Append( "/c" );
			bWin &= ShuffleArgs.Append( Args[ 0 ] );
			bWin &= ShuffleArgs.Num() == 3 && ShuffleArgs[ 1 ].Len() == 2;
			bWin &= ShuffleArgs.Num() == 3 && ShuffleArgs[ 2 ].Len() == Args[ 0 ].Len();
			
			if( !bWin ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while reconfiguring command-line for Windows (cmd.exe)" );
				return ENOMEM;
			}

			if( !GetWindowsSafeCommandLine( Shuffle, ShuffleArgs ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while preparing safe command-line for Windows (cmd.exe)" );
				return ENOMEM;
			}

			if( !Shuffle.Append( ' ' ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while fixing safe command-line for Windows (cmd.exe)" );
				return ENOMEM;
			}

			if( !CommandArgs.Prepend( Shuffle ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while inserting batch filename into command-line for Windows (cmd.exe)" );
				return ENOMEM;
			}
		} else if( !Args[ 0 ].CaseEndsWith( ".exe" ) && !Args[ 0 ].CaseEndsWith( ".com" ) ) {
			if( !AppName.Assign( Args[ 0 ] ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while preparing process name for Windows" );
				return ENOMEM;
			}

			if( !AppName.Append( ".exe" ) ) {
				Ax::Errorf( Args[ 0 ], "Ran out of memory while appending '.exe' to process name for Windows" );
				return ENOMEM;
			}
		}

		LogCommand( CommandArgs, ">" );

		bool bAppNameW = false;
		if( AppName.IsEmpty() ) {
			bAppNameW = Args[ 0 ].ToUTF16( AppNameW );
		} else {
			bAppNameW = AppName.ToUTF16( AppNameW );
		}
		if( !bAppNameW ) {
			Ax::Errorf( Args[ 0 ], "Ran out of memory while converting UTF-8 process name to UTF-16 for Windows" );
			return ENOMEM;
		}

		if( !CommandArgs.ToUTF16( CommandArgsW ) ) {
			Ax::Errorf( Args[ 0 ], "Ran out of memory while converting UTF-8 command line to UTF-16 for Windows" );
			return ENOMEM;
		}

		static const Ax::uintptr kMaxBytes = 4096;

		Ax::String Text;
		if( !Text.Reserve( kMaxBytes ) ) {
			Ax::Errorf( Args[ 0 ], "Failed to allocate enough space for reading output" );
			return ENOMEM;
		}

		STARTUPINFOW StartInfo;

		StartInfo.cb = sizeof( StartInfo );
		StartInfo.lpReserved = nullptr;
		StartInfo.lpDesktop = nullptr;
		StartInfo.lpTitle = nullptr;
		StartInfo.dwX = 0;
		StartInfo.dwY = 0;
		StartInfo.dwXSize = 0;
		StartInfo.dwYSize = 0;
		StartInfo.dwXCountChars = 0;
		StartInfo.dwYCountChars = 0;
		StartInfo.dwFillAttribute = 0;
		StartInfo.dwFlags = 0;
		StartInfo.wShowWindow = 0;
		StartInfo.cbReserved2 = 0;
		StartInfo.lpReserved2 = nullptr;
		StartInfo.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
		StartInfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
		StartInfo.hStdError = GetStdHandle( STD_ERROR_HANDLE );

		HANDLE hOutRd = NULL, hOutWr = NULL;
		HANDLE hErrRd = NULL, hErrWr = NULL;

		AX_SCOPE_GUARD({CloseHandle(hOutRd);CloseHandle(hOutWr);CloseHandle(hErrRd);CloseHandle(hErrWr);});

		IOutputFilter *pFilter = nullptr;
		for( Ax::uintptr j = m_Filters.Num(); j > 0; --j ) {
			const Ax::uintptr i = j - 1;
			IOutputFilter *const pCheck = m_Filters[ i ];
			if( !pCheck || !pCheck->CanProcess( Args ) ) {
				continue;
			}

			pFilter = pCheck;
			break;
		}

		if( pFilter != nullptr && !CreatePipe( &hOutRd, &hOutWr, nullptr, 0 ) ) {
			pFilter = nullptr;
		}

		if( pFilter != nullptr && !CreatePipe( &hErrRd, &hErrWr, nullptr, 0 ) ) {
			pFilter = nullptr;
		}

		if( pFilter != nullptr ) {
			StartInfo.dwFlags |= STARTF_USESTDHANDLES;

			StartInfo.hStdOutput = hOutWr;
			StartInfo.hStdError = hErrWr;
		}

		PROCESS_INFORMATION ProcInfo;
		if( !CreateProcessW
		(
			( const wchar_t * )AppNameW.Pointer(),
			( wchar_t * )CommandArgsW.Pointer(),
			nullptr,	// lpProcessAttributes
			nullptr,	// lpThreadAttributes
			FALSE,		// bInheritHandles
			0,			// dwCreationFlags
			nullptr,	// lpEnvironment
			nullptr,	// lpCurrentDirectory
			&StartInfo,
			&ProcInfo
		) ) {
			Ax::Errorf( Args[ 0 ], "Could not find or execute process" );
			return EEXIST;
		}

		if( pFilter != nullptr ) {
			DWORD dwResult;
			bool bDidEnterOut = false;
			bool bDidEnterErr = false;

			for(;;) {
				char Data[ kMaxBytes ];
				DWORD cReadBytes = 0;

				if( ReadFile( hOutRd, &Data[ 0 ], sizeof( Data ), &cReadBytes, nullptr ) ) {
					AX_EXPECT( cReadBytes <= kMaxBytes );
					Text.Assign( &Data[ 0 ], &Data[ cReadBytes ] );
					
					if( !bDidEnterOut ) {
						bDidEnterOut = true;
						pFilter->Enter( EStandardStream::Output );
					}

					pFilter->Write( EStandardStream::Output, Text );
				}

				if( ReadFile( hErrRd, &Data[ 0 ], sizeof( Data ), &cReadBytes, nullptr ) ) {
					AX_EXPECT( cReadBytes <= kMaxBytes );
					Text.Assign( &Data[ 0 ], &Data[ cReadBytes ] );

					if( !bDidEnterErr ) {
						bDidEnterErr = true;
						pFilter->Enter( EStandardStream::Error );
					}

					pFilter->Write( EStandardStream::Error, Text );
				}
			
				dwResult = WaitForSingleObject( ProcInfo.hProcess, 250 );
				if( dwResult != WAIT_TIMEOUT ) {
					break;
				}
			}
			
			if( bDidEnterOut ) {
				pFilter->Leave( EStandardStream::Output );
			}
			if( bDidEnterErr ) {
				pFilter->Leave( EStandardStream::Error );
			}
		}
		
		WaitForSingleObject( ProcInfo.hProcess, INFINITE );

		DWORD dwExitCode = EXIT_FAILURE;
		( void )GetExitCodeProcess( ProcInfo.hProcess, &dwExitCode );

		CloseHandle( ProcInfo.hThread );
		CloseHandle( ProcInfo.hProcess );

		if( m_bLoggingEnabled ) {
			Ax::Statusf( Args[ 0 ], "Process exited with code %i (0x%.8X)", ( int )dwExitCode, ( unsigned int )dwExitCode );
		}

		return ( int )dwExitCode;
#else
		LogCommand( Args, ">" );

		AX_ASSERT_MSG( false, "MShell::Run() Unimplemented" );

		//# error This has not been implemented for your platform.
#endif
	}

	int MShell::Run( const char *pszCommand )
	{
		AX_ASSERT_NOT_NULL( pszCommand );
		if( !pszCommand ) {
			return EINVAL;
		}

		Ax::String Command;

		if( !Command.Assign( pszCommand ) ) {
			Ax::BasicErrorf( "Ran out of memory while preparing to execute: %s", pszCommand );
			return ENOMEM;
		}

		return Run( Command );
	}
	int MShell::Run( const Ax::String &Command )
	{
		Ax::TArray< Ax::String > Args;
		Args = Command.SplitUnquoted( " ", '\\', Ax::EKeepQuotes::No );
		return Run( Args );
	}
	int MShell::Runfv( const char *pszFormat, va_list args )
	{
		Ax::String Command;

		AX_ASSERT_NOT_NULL( pszFormat );
		if( !pszFormat ) {
			return EINVAL;
		}

		if( !Command.FormatV( pszFormat, args ) ) {
			Ax::BasicErrorf( "Ran out of memory while preparing to execute formatted command: %s", pszFormat );
			return ENOMEM;
		}

		return Run( Command );
	}
	int MShell::Runf( const char *pszFormat, ... )
	{
		Ax::String Command;
		va_list args;

		AX_ASSERT_NOT_NULL( pszFormat );
		if( !pszFormat ) {
			return EINVAL;
		}

		va_start( args, pszFormat );
		const int iResult = Runfv( pszFormat, args );
		va_end( args );

		return Run( Command );
	}

}}

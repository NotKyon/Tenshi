#pragma once

#include <Core/Manager.hpp>
#include <Core/String.hpp>

#include <Collections/Array.hpp>
#include <Collections/Dictionary.hpp>

// Characters allowed to be used in built-in command names
//
// Commands are case-insensitive (e.g., "abc" and "ABC" are equivalent)
#define TENSHI_SHELLCOMMAND_ALLOWED\
	AX_DICT_ALPHALOWER AX_DICT_DIGITS "-_."

namespace Tenshi { namespace Compiler {

	enum class EStandardStream
	{
		Output,
		Error
	};

	// Built-in command for execution by the shell
	class IShellCommand
	{
	public:
		IShellCommand() {}
		virtual ~IShellCommand() {}

		// Run the command -- return EXIT_SUCCESS on success or errno on failure
		virtual int Run( const Ax::TArray< Ax::String > &Args ) = 0;
	};

	// Output filter (for processing text from a shell command)
	class IOutputFilter
	{
	public:
		IOutputFilter() {}
		virtual ~IOutputFilter() {}

		// Ask whether this filter wants to process output from the given command
		//
		// The first element in the Args array will be the name of the process
		//
		// Return true if this filter can be used to process output for the
		// given process.
		virtual bool CanProcess( const Ax::TArray< Ax::String > &Args ) const = 0;

		// Invoked before any output from the given stream is processed
		virtual void Enter( EStandardStream Stream ) {}
		// Invoked after all output from the given stream has been processed
		virtual void Leave( EStandardStream Stream ) {}
		// Invoked to process output for the given stream
		virtual void Write( EStandardStream Stream, const Ax::String &Text ) = 0;
	};

	class MShell
	{
	public:
		static MShell &GetInstance();

		int Run( const Ax::TArray< Ax::String > &Args );

		int Run( const char *pszCommand );
		int Run( const Ax::String &Command );
		int Runfv( const char *pszFormat, va_list args );
		int Runf( const char *pszFormat, ... );
		
		// Convert the given argument array into a version CommandLineToArgvW()
		// will process properly (except in UTF-8 instead of UTF-16).
		//
		// How Windows parses command arguments:
		// https://msdn.microsoft.com/en-us/library/windows/desktop/17w5ykft%28v=vs.85%29.aspx
		//
		// InoutString will be appended to, not assigned to. If it already has
		// data in it, that data will not be erased.
		//
		// Returns true upon success.
		bool GetWindowsSafeCommandLine( Ax::String &InoutString, const Ax::TArray< Ax::String > &Args ) const;

	private:
		bool						m_bLoggingEnabled;
		Ax::TArray<IOutputFilter *>	m_Filters;
		Ax::TArray<IShellCommand *>	m_Commands;
		Ax::TDictionary<IShellCommand>
									m_CommandMap;

		MShell();
		~MShell();

		void LogCommand( const Ax::TArray< Ax::String > &Args, const char *pszPrefix ) const;
		void LogCommand( const Ax::String &CommandLine, const char *pszPrefix ) const;

		AX_DELETE_COPYFUNCS(MShell);
	};

	static Ax::TManager<MShell>		Shell;

}}

#pragma once

#include "../Platform/BuildConf.hpp"
#include "../Platform/Platform.hpp"
#include "../Core/Types.hpp"
#include "Assert.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef AX_DEFAULT_REPORT_CHANNEL
# define AX_DEFAULT_REPORT_CHANNEL 0
#endif

namespace Ax
{

	/*
	===========================================================================
	
		REPORTER

		Handle logging and reports
	
	===========================================================================
	*/

	// Specifies the Severity of any given report
	enum class ESeverity
	{
		// Verbose (unnecessary) output
		Verbose,
		// No Severity, just a "normal" report (status, help, etc)
		Normal,
		// Debug text for development purposes
		Debug,
		// Indication of a better way to do something
		Hint,
		// A potentially unwanted or non-optimal situation was detected
		Warning,
		// A definitely unwanted or unworkable situation has occurred
		Error
	};

	// Report details
	struct SReportDetails
	{
									// The Severity of the report
		ESeverity					Severity;
									// Which subsystem the report is coming From (this is arbitrary)
		int							From;
									// Name of the pszFile which the report is affecting (this can be NULL)
		const char *				pszFile;
									// Line number within the given pszFile (ignored if 0 or if 'pszFile' is NULL)
		uint32						uLine;
									// Column number on the given uLine within the pszFile (ignored if 0 or
									// 'uLine' is ignored)
		uint32						uColumn;
									// Name of the pszFunction the report concerns (this can be NULL)
		const char *				pszFunction;

		inline SReportDetails()
		: Severity( ESeverity::Normal )
		, From( AX_DEFAULT_REPORT_CHANNEL )
		, pszFile( nullptr )
		, uLine( 0 )
		, uColumn( 0 )
		, pszFunction( nullptr )
		{
		}
		inline SReportDetails( ESeverity Sev, const char *pszFile = nullptr, uint32 uLine = 0, uint32 uColumn = 0,
		const char *pszFunction = nullptr )
		: Severity( Sev )
		, From( AX_DEFAULT_REPORT_CHANNEL )
		, pszFile( pszFile )
		, uLine( uLine )
		, uColumn( uColumn )
		, pszFunction( pszFunction )
		{
		}
		inline SReportDetails( ESeverity Sev, const char *pszFile, uint32 uLine, const char *pszFunction )
		: Severity( Sev )
		, From( AX_DEFAULT_REPORT_CHANNEL )
		, pszFile( pszFile )
		, uLine( uLine )
		, uColumn( 0 )
		, pszFunction( pszFunction )
		{
		}
		inline SReportDetails( ESeverity Sev, int From, const char *pszFile = nullptr, uint32 uLine = 0, uint32 uColumn = 0,
		const char *pszFunction = nullptr )
		: Severity( Sev )
		, From( From )
		, pszFile( pszFile )
		, uLine( uLine )
		, uColumn( uColumn )
		, pszFunction( pszFunction )
		{
		}
		inline SReportDetails( ESeverity Sev, int From, const char *pszFile, uint32 uLine, const char *pszFunction )
		: Severity( Sev )
		, From( From )
		, pszFile( pszFile )
		, uLine( uLine )
		, uColumn( 0 )
		, pszFunction( pszFunction )
		{
		}
		inline SReportDetails( const SReportDetails &x )
		: Severity( x.Severity )
		, From( x.From )
		, pszFile( x.pszFile )
		, uLine( x.uLine )
		, uColumn( x.uColumn )
		, pszFunction( x.pszFunction )
		{
		}

		inline SReportDetails &operator=( const SReportDetails &x )
		{
			Severity = x.Severity;
			From = x.From;
			pszFile = x.pszFile;
			uLine = x.uLine;
			uColumn = x.uColumn;
			pszFunction = x.pszFunction;

			return *this;
		}
	};

	// Base interface class for reporters
	class IReporter
	{
	public:
		// UNDOC: Constructor
		IReporter()
		{
		}
		// UNDOC: Destructor
		virtual ~IReporter()
		{
		}

		// Handle a report
		//
		// Sev: Severity of the report (see ESeverity above)
		// pszFile: File the report concerns
		// uLine: Line the report concerns
		// message: Description of the report
		virtual void Report( const SReportDetails &details, const char *message ) = 0;
	};

	// Submit a report to all listening reporters
	void Report( const SReportDetails &details, const char *message );

	inline void Report( ESeverity Sev, const char *pszFile, int uLine, const char *message )
	{
		AX_ASSERT_NOT_NULL( message );

		SReportDetails details;

		details.Severity = Sev;
		details.pszFile = pszFile;
		details.uLine = ( uint32 )uLine;

		Report( details, message );
	}

	// Add a reporter interface for handling reports
	void AddReporter( IReporter *r );

	// Remove an added reporter interface
	void RemoveReporter( IReporter *r );

	// IReporter proxy
	class ReportProxy
	{
	public:
		inline ReportProxy()
		: mDetails( ESeverity::Normal )
		{
		}
		inline ReportProxy( const ReportProxy &x )
		: mDetails( x.mDetails )
		{
		}
		inline ReportProxy( ESeverity Sev, int From = AX_DEFAULT_REPORT_CHANNEL )
		: mDetails( Sev, From )
		{
		}
		inline ReportProxy( ESeverity Sev, const char *pszFile )
		: mDetails( Sev, pszFile )
		{
		}
		inline ReportProxy( ESeverity Sev, int From, const char *pszFile, uint32 uLine, uint32 uColumn,
		const char *pszFunction )
		: mDetails( Sev, From, pszFile, uLine, uColumn, pszFunction )
		{
		}
		inline ReportProxy( const SReportDetails &details )
		: mDetails( details )
		{
		}
		inline ~ReportProxy()
		{
		}

		inline ReportProxy operator[]( int From ) const
		{
			return ReportProxy( mDetails.Severity, From, mDetails.pszFile, mDetails.uLine, mDetails.uColumn,
				mDetails.pszFunction );
		}
		inline ReportProxy operator()( const char *pszFile ) const
		{
			return ReportProxy( mDetails.Severity, mDetails.From, pszFile, mDetails.uLine, mDetails.uColumn,
				mDetails.pszFunction );
		}
		inline ReportProxy operator()( const char *pszFile, uint32 uLine ) const
		{
			return ReportProxy( mDetails.Severity, mDetails.From, pszFile, uLine, mDetails.uColumn, mDetails.pszFunction );
		}
		inline ReportProxy operator()( const char *pszFile, uint32 uLine, uint32 uColumn ) const
		{
			return ReportProxy( mDetails.Severity, mDetails.From, pszFile, uLine, uColumn, mDetails.pszFunction );
		}
		inline ReportProxy operator()( const char *pszFile, uint32 uLine, const char *pszFunction ) const
		{
			return ReportProxy( mDetails.Severity, mDetails.From, pszFile, uLine, mDetails.uColumn, pszFunction );
		}
		inline ReportProxy operator()( const char *pszFile, uint32 uLine, uint32 uColumn, const char *pszFunction ) const
		{
			return ReportProxy( mDetails.Severity, mDetails.From, pszFile, uLine, uColumn, pszFunction );
		}

		inline ReportProxy &Submit( const char *message )
		{
			Report( mDetails, message );
			return *this;
		}

		inline ReportProxy &operator<<( const char *message )
		{
			Report( mDetails, message );
			return *this;
		}
		inline ReportProxy &operator+=( const char *message )
		{
			Report( mDetails, message );
			return *this;
		}

	private:
		SReportDetails mDetails;
	};

	// AX_TRACE(msg) submits a debug report concerning this code IF in debug mode
#if AX_DEBUG_ENABLED
# define AX_TRACE( msg )\
	Ax::Report( Ax::ESeverity::Debug, __FILE__, __LINE__, msg )
#else
# define AX_TRACE( msg )\
	( void )0
#endif

#ifndef _WIN32
# define vsprintf_s vsnprintf
#endif

#define AX_DO_REPORT_F( _Sev_, _File_, _Line_, _Fmt_ )\
	char buf[ 1024 ];\
	\
	va_list args;\
	va_start( args, _Fmt_ );\
	vsprintf_s( buf, sizeof( buf ), _Fmt_, args );\
	buf[ sizeof( buf ) - 1 ] = '\0';\
	va_end( args );\
	\
	Report( _Sev_, _File_, _Line_, buf )

	// Report a warning
	inline void Warnf( const char *pszFile, int uLine, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Warning, pszFile, uLine, fmt );
	}
	// Report an error
	inline void Errorf( const char *pszFile, int uLine, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Error, pszFile, uLine, fmt );
	}
	// Report debugging information
	inline void Debugf( const char *pszFile, int uLine, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Debug, pszFile, uLine, fmt );
	}
	// Report status
	inline void Statusf( const char *pszFile, int uLine, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Normal, pszFile, uLine, fmt );
	}

	// Report a warning
	inline void Warnf( const char *pszFile, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Warning, pszFile, 0, fmt );
	}
	// Report an error
	inline void Errorf( const char *pszFile, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Error, pszFile, 0, fmt );
	}
	// Report debugging information
	inline void Debugf( const char *pszFile, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Debug, pszFile, 0, fmt );
	}
	// Report status
	inline void Statusf( const char *pszFile, const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( pszFile );
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Normal, pszFile, 0, fmt );
	}

	// Report a warning
	inline void BasicWarnf( const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Warning, nullptr, 0, fmt );
	}
	// Report an error
	inline void BasicErrorf( const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Error, nullptr, 0, fmt );
	}
	// Report debugging information
	inline void BasicDebugf( const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Debug, nullptr, 0, fmt );
	}
	// Report status
	inline void BasicStatusf( const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Normal, nullptr, 0, fmt );
	}
	// Same as BasicStatusf (report status) -- includes new-uLine in output
	inline void Printf( const char *fmt, ... )
	{
		AX_ASSERT_NOT_NULL( fmt );

		AX_DO_REPORT_F( ESeverity::Normal, nullptr, 0, fmt );
	}

	static ReportProxy g_VerboseLog( ESeverity::Verbose );
	static ReportProxy g_InfoLog( ESeverity::Normal );
	static ReportProxy g_DebugLog( ESeverity::Debug );
	static ReportProxy g_HintLog( ESeverity::Hint );
	static ReportProxy g_WarningLog( ESeverity::Warning );
	static ReportProxy g_ErrorLog( ESeverity::Error );

#define AX_REFERENCE_LOG( Kind, From )\
	Ax::Kind##Log[ From ]( __FILE__, __LINE__, AX_PRETTY_FUNCTION )

#define AX_VERBOSE_LOG Ax::g_VerboseLog ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )
#define AX_INFO_LOG    Ax::g_InfoLog    ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )
#define AX_DEBUG_LOG   Ax::g_DebugLog   ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )
#define AX_HINT_LOG    Ax::g_HintLog    ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )
#define AX_WARNING_LOG Ax::g_WarningLog ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )
#define AX_ERROR_LOG   Ax::g_ErrorLog   ( __FILE__, __LINE__, AX_PRETTY_FUNCTION )

#undef vsprintf_s

	// Install the debug.log reporter
	void InstallDebugLogReporter();
	// Uninstall the debug.log reporter
	void UninstallDebugLogReporter();

	// Install the colored console reporter
	void InstallConsoleReporter();
	// Uninstall the colored console reporter
	void UninstallConsoleReporter();

}

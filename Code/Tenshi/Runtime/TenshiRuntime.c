#ifdef _WIN32
# define TENSHI_FUNC				__declspec( dllexport )
# if defined( __GNUC__ )
#  include <_mingw.h>
#  undef MINGW_HAS_SECURE_API		/* No, you really don't. */
# endif
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
# undef min
# undef max
#endif

#define TENSHI_STATIC_LINK_ENABLED	1

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "TenshiRuntime.h"

#define CURRENT_MEMTAG				g_RTGlob.iCurrentMemtag

#ifdef _MSC_VER
# define CURRENT_FUNCTION			__FUNCTION__
#else
# define CURRENT_FUNCTION			__func__
#endif

#ifndef TRACE_ENABLED
# ifdef _DEBUG
#  define TRACE_ENABLED				1
# else
#  define TRACE_ENABLED				0
# endif
#endif

#ifndef MEMTRACE_ENABLED
# define MEMTRACE_ENABLED			0
#endif
#ifndef STRTRACE_ENABLED
# define STRTRACE_ENABLED			0
#endif

#ifndef SAFE_HANDLES_ENABLED
# ifdef _DEBUG
#  define SAFE_HANDLES_ENABLED		1
# else
#  define SAFE_HANDLES_ENABLED		0
# endif
#endif

#if TRACE_ENABLED
# define TRACE(...)					teLogf(\
										TELOG_DEBUG | TENSHI_FACILITY | TELOG_C_TRACE,\
										TENSHI_MODNAME,\
										__FILE__, __LINE__, CURRENT_FUNCTION,\
										( const char * )0,\
										__VA_ARGS__)
#else
# define TRACE(...)					((void)0)
#endif

#undef TENSHI_MODNAME
#undef TENSHI_FACILITY
#define TENSHI_MODNAME				"$core"
#define TENSHI_FACILITY				kTenshiLog_CoreRT

#define FREEPTR						((void*)0)
#define RESERVEDPTR					((void*)1)

#ifdef _MSC_VER
# pragma section( ".CRT$XCU", read )
# define CTOR(Fn_)\
	static void __cdecl Fn_( void );\
	__declspec( allocate( ".CRT$XCU" ) ) void( __cdecl *PFn##Fn_##_ )( void ) = Fn_;\
	static void __cdecl Fn_( void )
#else
# define CTOR(Fn_)\
	static void Fn_( void ) __attribute__((constructor));\
	static void Fn_( void )
#endif


static TenshiRuntimeGlob_t			g_RTGlob;
static struct TenshiEngineTypes_s	g_EngineTypes;
static struct TenshiMemblockAPI_s	g_MemblockAPI;
static struct TenshiLoggingAPI_s	g_LoggingAPI;
static TenshiObjectPool_t *			g_RNGPool;


/*
===============================================================================

	LOGGING

===============================================================================
*/

TENSHI_FUNC const char *TENSHI_CALL teGetReportPriorityStr( TenshiReportPriority_t x )
{
	switch( x )
	{
	case kTenshiLog_Debug:			return "debug";
	case kTenshiLog_Info:			return "info";
	case kTenshiLog_Notice:			return "notice";
	case kTenshiLog_Warning:		return "warning";
	case kTenshiLog_Error:			return "error";
	case kTenshiLog_Critical:		return "critical";
	case kTenshiLog_Alert:			return "alert";
	case kTenshiLog_Panic:			return "panic";
	}

	return "(unknown-priority)";
}
TENSHI_FUNC const char *TENSHI_CALL teGetReportFacilityStr( TenshiReportFacility_t x )
{
	switch( x )
	{
	case kTenshiLog_UserCode:		return "app.usr";
	case kTenshiLog_ThirdParty:		return "app.tpc";

	case kTenshiLog_CoreRT:			return "rt";
	case kTenshiLog_CoreRT_Memory:	return "rt.mm";
	case kTenshiLog_CoreRT_Object:	return "rt.obj";
	case kTenshiLog_CoreRT_Type:	return "rt.ty";
	case kTenshiLog_CoreRT_String:	return "rt.str";
	case kTenshiLog_CoreRT_Array:	return "rt.arr";
	case kTenshiLog_CoreRT_List:	return "rt.ls";
	case kTenshiLog_CoreRT_BTree:	return "rt.bt";

	case kTenshiLog_MemblockAPI:	return "api.mm";
	case kTenshiLog_CVarAPI:		return "api.cv";
	case kTenshiLog_FileAPI:		return "api.fs";
	case kTenshiLog_SystemAPI:		return "api.sys";
	case kTenshiLog_MathAPI:		return "api.m";
	case kTenshiLog_AsyncAPI:		return "api.tsk";
	case kTenshiLog_NetworkAPI:		return "api.net";
	case kTenshiLog_WindowingAPI:	return "api.wnd";
	case kTenshiLog_InputAPI:		return "api.in";
	case kTenshiLog_AudioAPI:		return "api.snd";
	case kTenshiLog_RendererAPI:	return "api.r";
	case kTenshiLog_ImageAPI:		return "api.img";
	case kTenshiLog_Basic2DAPI:		return "api.b2";
	case kTenshiLog_Basic3DAPI:		return "api.b3";
	case kTenshiLog_TerrainAPI:		return "api.ter";
	case kTenshiLog_ParticlesAPI:	return "api.fx";
	case kTenshiLog_PhysicsAPI:		return "api.phy";
	case kTenshiLog_BakerAPI:		return "api.bkr";
	case kTenshiLog_VRAPI:			return "api.vr";
	}

	return "(unknown-facility)";
}
TENSHI_FUNC const char *TENSHI_CALL teGetReportCauseStr( TenshiReportCause_t x )
{
	switch( x )
	{
	case kTenshiLog_Intentional:			return "app.log";
	case kTenshiLog_Init:					return "app.init";
	case kTenshiLog_Fini:					return "app.fini";

	case kTenshiLog_InternalFile:			return "io.int";
	case kTenshiLog_ExternalFile:			return "io.ext";

	case kTenshiLog_Trace:					return "dbg.trace";
	case kTenshiLog_Development:			return "dbg.devel";
	case kTenshiLog_Stats:					return "dbg.stats";

	case kTenshiLog_OutOfMemory:			return "env.no-memory";
	case kTenshiLog_BufferOverflow:			return "sec.buf-of";
	case kTenshiLog_BufferUnderflow:		return "sec.buf-uf";
	case kTenshiLog_FailedCheck:			return "bad";
	case kTenshiLog_FailedCheck_IsNull:		return "bad.is-null";
	case kTenshiLog_FailedCheck_NotNull:	return "bad.not-null";
	}

	return "(unknown-cause)";
}

static void TENSHI_CALL DefaultSubmitReport_f( const TenshiReport_t *p )
{
#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
# define spf sprintf_s
# define vspf vsprintf_s
#else
# define spf snprintf
# define vspf vsnprintf
#endif
#define P(x) (!(x)?"(null)":(x))

	char szFilePart[ 512 ];
	char szFuncExpr[ 256 ];
	const char *pszPromptName;
	const char *pszFacL, *pszFac;
	const char *pszCauseL, *pszCauseR, *pszCause;

	if( !p ) {
		return;
	}

	if( p->pszFilenameUTF8 != ( const char * )0 ) {
		if( p->LineNumber ) {
			if( p->Column ) {
				spf( szFilePart, sizeof( szFilePart ) - 1, "%s(%u:%u): ",
					p->pszFilenameUTF8, p->LineNumber, p->Column );
			} else {
				spf( szFilePart, sizeof( szFilePart ) - 1, "%s(%u): ",
					p->pszFilenameUTF8, p->LineNumber );
			}
		} else {
			spf( szFilePart, sizeof( szFilePart ) - 1, "%s: ",
				p->pszFilenameUTF8 );
		}

		szFilePart[ sizeof( szFilePart ) - 1 ] = '\0';
	} else {
		szFilePart[ 0 ] = '\0';
	}

	if( p->pszFunctionUTF8 != ( const char * )0 ) {
		if( p->pszExpressionUTF8 != ( const char * )0 ) {
			spf( szFuncExpr, sizeof( szFuncExpr ) - 1, " in %s: `%s`: ",
				p->pszFunctionUTF8, p->pszExpressionUTF8 );
		} else {
			spf( szFuncExpr, sizeof( szFuncExpr ) - 1, " in %s: ",
				p->pszFunctionUTF8 );
		}
		szFuncExpr[ sizeof( szFuncExpr ) - 1 ] = '\0';
	} else if( p->pszExpressionUTF8 != ( const char * )0 ) {
		spf( szFuncExpr, sizeof( szFuncExpr ) - 1, " `%s`: ",
			p->pszExpressionUTF8 );
		szFuncExpr[ sizeof( szFuncExpr ) - 1 ] = '\0';
	} else {
		szFuncExpr[ 0 ] = ':';
		szFuncExpr[ 1 ] = ' ';
		szFuncExpr[ 2 ] = '\0';
	}

	pszPromptName = teGetReportPriorityStr( p->Priority );
	if( p->Priority == kTenshiLog_Critical ) {
		pszPromptName = "error: **1:CRITICAL**";
	} else if( p->Priority == kTenshiLog_Alert ) {
		pszPromptName = "error: **2:ALERT**";
	} else if( p->Priority == kTenshiLog_Panic ) {
		pszPromptName = "error: **3:FATAL**";
	}

	if( p->Cause == kTenshiLog_ExternalFile ) {
		pszFacL = "";
		pszFac = "";
		
		pszCauseL = "";
		pszCause = "";
		pszCauseR = "";
	} else {
		pszFacL = "[";
		pszFac = teGetReportFacilityStr( p->Facility );

		pszCauseL = ":";
		pszCause = teGetReportCauseStr( p->Cause );
		pszCauseR = "] ";
	}

	fprintf( stderr, "%s%s%s%s%s%s%s%s%s\n",
		pszFacL, pszFac, pszCauseL, pszCause, pszCauseR,
		szFilePart, pszPromptName, szFuncExpr,
		P(p->pszMessageUTF8 ) );
}

#ifdef _WIN32
static HANDLE g_hLogger = INVALID_HANDLE_VALUE;

static void __cdecl CloseLogger( void )
{
	if( g_hLogger != INVALID_HANDLE_VALUE ) {
		CloseHandle( g_hLogger );
		g_hLogger = INVALID_HANDLE_VALUE;
	}
}
static int OpenLogger( void )
{
	static const wchar_t *const pwszPipeName = L"\\\\.\\pipe\\axia_debug_log";

	g_hLogger = CreateFileW( pwszPipeName, GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
	if( g_hLogger == INVALID_HANDLE_VALUE ) {
		return 0;
	}

	atexit( &CloseLogger );
	return 1;
}

static void Logger__WriteBuf( unsigned char **bufp, unsigned char *bufe, const char *pszsrc )
{
	unsigned char *p;

	p = *bufp;
	while( p < bufe ) {
		*p++ = *pszsrc;
		if( *pszsrc == '\0' ) {
			break;
		}
		++pszsrc;
	}

	*bufp = p;
}
static void TENSHI_CALL LoggerSubmitReport_f( const TenshiReport_t *p )
{
#pragma pack(push,1)
	union {
		unsigned char buf[ 8192 ];
		struct {
			TenshiUInt32_t bytes;
			TenshiUInt16_t flags;
			TenshiUInt32_t line;
			TenshiUInt16_t column;
			int posixerr;
			TenshiUInt32_t systemerr;
			TenshiUInt32_t pid;
			TenshiUInt32_t tid;
		} h;
	} x;
#pragma pack(pop)
	unsigned char *bufp;
	unsigned char *bufe;

	if( !p ) {
		return;
	}

	if( g_hLogger == INVALID_HANDLE_VALUE ) {
		DefaultSubmitReport_f( p );
		return;
	}

	x.h.flags = p->Priority | p->Facility | p->Cause;
	x.h.line = p->LineNumber;
	x.h.column = p->Column < 0x10000 ? ( TenshiUInt16_t )p->Column : 0;
	x.h.posixerr = p->POSIXErrorCode;
	x.h.systemerr = p->SystemErrorCode;
	x.h.pid = p->ProcessID;
	x.h.tid = p->ThreadID;

	bufp = &x.buf[ sizeof( x.h ) ];
	bufe = &x.buf[ sizeof( x.buf ) ];

	Logger__WriteBuf( &bufp, bufe, p->pszModuleNameUTF8 );
	Logger__WriteBuf( &bufp, bufe, p->pszFilenameUTF8 );
	Logger__WriteBuf( &bufp, bufe, p->pszFunctionUTF8 );
	Logger__WriteBuf( &bufp, bufe, p->pszExpressionUTF8 );
	Logger__WriteBuf( &bufp, bufe, p->pszMessageUTF8 );

	x.h.bytes = ( TenshiUInt32_t )( size_t )( bufp - &x.buf[ 0 ] );

	WriteFile( g_hLogger, &x.buf[0], x.h.bytes, NULL, NULL );
}
#endif

static void CaptureReportSysInfo( TenshiReport_t *p )
{
	p->POSIXErrorCode = errno;

#ifdef _WIN32
	p->ProcessID = GetCurrentProcessId();
	p->ThreadID = GetCurrentThreadId();
	p->SystemErrorCode = GetLastError();
#else
	p->ProcessID = 0;
	p->ThreadID = 0;
	p->SystemErrorCode = p->POSIXErrorCode;
#endif
}
static void ProcessLogFlags( TenshiReport_t *p, TenshiUInt16_t flags )
{
	p->Priority = ( TenshiReportPriority_t )( flags & TENSHI_LOG_PRIORITY_MASK );
	p->Facility = ( TenshiReportFacility_t )( flags & TENSHI_LOG_FACILITY_MASK );
	p->Cause = ( TenshiReportCause_t )( flags & TENSHI_LOG_CAUSE_MASK );

	if( flags & kTenshiLogF_SystemInfo ) {
		CaptureReportSysInfo( p );
	} else {
		p->ProcessID = 0;
		p->ThreadID = 0;
		p->POSIXErrorCode = 0;
		p->SystemErrorCode = 0;
	}
}

static TenshiReport_t *InitReport( TenshiReport_t *p, TenshiUInt16_t flags,
const char *pszModUTF8, const char *pszFileUTF8, TenshiUInt32_t Line,
TenshiUInt32_t Column, const char *pszFuncUTF8, const char *pszExprUTF8,
const char *pszMessageUTF8 )
{
	ProcessLogFlags( p, flags );

	p->pszModuleNameUTF8 = pszModUTF8;

	p->pszFilenameUTF8 = pszFileUTF8;
	p->LineNumber = Line;
	p->Column = Column;
	p->pszFunctionUTF8 = pszFuncUTF8;
	p->pszExpressionUTF8 = pszExprUTF8;

	p->pszMessageUTF8 = pszMessageUTF8;

	return p;
}

TENSHI_FUNC void TENSHI_CALL teLogfv( TenshiUInt16_t flags, const char *pszMod,
const char *pszFile, TenshiUInt32_t Line, const char *pszFunc,
const char *pszExpr, const char *pszFormatMessage, va_list args )
{
	char szBuf[ 4096 ];
	TenshiReport_t r;
	TenshiFnSubmitReport_t pfnSubmit;

	pfnSubmit = g_LoggingAPI.pfnSubmitReport;
	if( !pfnSubmit || ( flags & TENSHI_LOG_PRIORITY_MASK ) < ( TenshiUInt16_t )g_LoggingAPI.MinimumPriority ) {
		return;
	}

	szBuf[ 0 ] = '\0';
	if( pszFormatMessage != ( const char * )0 ) {
		vspf( szBuf, sizeof( szBuf ) - 1, pszFormatMessage, args );
		szBuf[ sizeof( szBuf ) - 1 ] = '\0';
	}

	InitReport( &r, flags, pszMod, pszFile, Line, 0, pszFunc, pszExpr, szBuf );

	pfnSubmit( &r );
}
TENSHI_FUNC void TENSHI_CALL teLogf( TenshiUInt16_t flags, const char *pszMod,
const char *pszFile, TenshiUInt32_t Line, const char *pszFunc,
const char *pszExpr, const char *pszFormatMessage, ... )
{
	va_list args;

	va_start( args, pszFormatMessage );
	teLogfv( flags, pszMod, pszFile, Line, pszFunc, pszExpr,
		pszFormatMessage, args );
	va_end( args );
}



/*
===============================================================================

	RUNTIME ERROR

===============================================================================
*/

static void TENSHI_CALL DefaultRuntimeError_f( const TenshiReport_t *p, TenshiUInt32_t ErrorId )
{
	if( !g_LoggingAPI.pfnSubmitReport ) {
		DefaultSubmitReport_f( p );
	} else {
		g_LoggingAPI.pfnSubmitReport( p );
	}

	( void )ErrorId;
}

TENSHI_FUNC void TENSHI_CALL teRuntimeError( TenshiReportFacility_t Facility, const char *pszModName, TenshiUInt32_t ErrorId )
{
	TenshiReport_t r;
	char szMessage[ 512 ];

	ProcessLogFlags( &r, TELOG_PANIC | Facility | TELOG_C_FAIL | TELOGF_SYSINFO );

	if( !pszModName ) {
		/* TODO: Determine module by analyzing return address */
	}

	r.pszModuleNameUTF8 = pszModName;

	/* TODO: Determine file, line, function, and expression from debug information if available */
	r.pszFilenameUTF8 = ( const char * )0;
	r.LineNumber = 0;
	r.Column = 0;
	r.pszFunctionUTF8 = ( const char *)0;
	r.pszExpressionUTF8 = ( const char * )0;

	/* TODO: Load the appropriate message for this error */
	spf( szMessage, sizeof( szMessage ) - 1, "Runtime Error %u", ErrorId );
	szMessage[ sizeof( szMessage ) - 1 ] = '\0';

	r.pszMessageUTF8 = &szMessage[ 0 ];

	if( !g_RTGlob.pfnRuntimeErrorCallback ) {
		DefaultRuntimeError_f( &r, ErrorId );
	} else {
		g_RTGlob.pfnRuntimeErrorCallback( &r, ErrorId );
	}

	exit( ErrorId );
}



/*
===============================================================================

	MODULE HANDLING

===============================================================================
*/

typedef int( TENSHI_CALL *FnPluginInit_t )( TenshiRuntimeGlob_t * );
typedef void( TENSHI_CALL *FnPluginFini_t )( void );

extern const char *					tenshi__modNames__[];
extern FnPluginInit_t				tenshi__modInits__[];
extern FnPluginFini_t				tenshi__modFinis__[];

extern TenshiUIntPtr_t				tenshi__numMods__;

static const char *StrOrNull( const char *p )
{
	return !!p ? p : "(null)";
}

static void InitModules( void )
{
	TenshiUIntPtr_t i;

	TRACE( "Enter" );

	for( i = 0; i < tenshi__numMods__; ++i ) {
		const char *pszModName;

		pszModName = StrOrNull( tenshi__modNames__[ i ] );

		if( !tenshi__modInits__[ i ] ) {
			TRACE( "Module '%s' has no initialization routine... Skipping.", pszModName );
			continue;
		}

		TRACE( "Initializing module '%s'...", pszModName );
		if( tenshi__modInits__[ i ]( &g_RTGlob ) & 1 ) {
			TRACE( "Module initialization succeeded." );
			continue;
		}

		TRACE( "Module initialization failed." );
		while( i > 0 ) {
			--i;
			TRACE( "Closing module '%s'...",
				StrOrNull( tenshi__modNames__[ i ] ) );
			if( !tenshi__modFinis__[ i ] ) {
				continue;
			}

			tenshi__modFinis__[ i ]();
		}

		fprintf( stderr, "ERROR: Failed to initialize module \"%s\"\n", pszModName );

		TRACE( "Abort" );
		exit( EXIT_FAILURE );
	}

	TRACE( "Leave" );
}
static void FiniModules( void )
{
	TenshiUIntPtr_t i;

	TRACE( "Enter" );

	i = tenshi__numMods__;
	while( i > 0 ) {
		--i;

#if TRACE_ENABLED
		const char *pszModName;
		
		pszModName = StrOrNull( tenshi__modNames__[ i ] );

		TRACE( "Closing module '%s'...", pszModName );
#endif
		if( !tenshi__modFinis__[ i ] ) {
			continue;
		}

		tenshi__modFinis__[ i ]();
	}

	TRACE( "Leave" );
}


/*
===============================================================================

	RUNTIME (CORE)

===============================================================================
*/

static const TenshiUIntPtr_t kNumAgesPerIndex = sizeof( TenshiUIntPtr_t )*8/4;

extern TenshiUInt32_t				tenshi__numTypes__;
extern TenshiType_t					tenshi__types__[];

CTOR( teInitGlob )
{
	static struct TenshiRTTypeInfo_s ti;

	ti.cTypes = (TenshiUIntPtr_t)tenshi__numTypes__;
	ti.pTypes = &tenshi__types__[0];

	memset( &g_RTGlob, 0, sizeof( g_RTGlob ) );
	memset( &g_EngineTypes, 0, sizeof( g_EngineTypes ) );

	g_RTGlob.uRuntimeVersion = TENSHI_RTGLOB_VERSION;

	g_RTGlob.pTypeInfo = &ti;
	g_RTGlob.pEngineTypes = &g_EngineTypes;

	g_RTGlob.pfnAlloc = &teAlloc;
	g_RTGlob.pfnDealloc = &teDealloc;

	g_RTGlob.pfnString = &teStrAlloc;
	g_RTGlob.pfnStrDup = &teStrDup;

	g_RTGlob.pfnLogfv = &teLogfv;

	g_RTGlob.pfnRuntimeErrorCallback = &DefaultRuntimeError_f;
	g_RTGlob.pfnRuntimeError = &teRuntimeError;

	g_RTGlob.pMemblockAPI = &g_MemblockAPI;
	g_RTGlob.pLoggingAPI = &g_LoggingAPI;

	g_EngineTypes.pfnAllocPool = &teAllocEnginePool;
	g_EngineTypes.pfnObjectExists = &teEngineObjectExists;
	g_EngineTypes.pfnReserveObjects = &teReserveIndexes;
	g_EngineTypes.pfnAllocObject = &teAllocEngineObject;
	g_EngineTypes.pfnDeallocObject = &teDeallocEngineObject;
	g_EngineTypes.pfnUnwrapObject = &teUnwrapEngineObject;

	g_LoggingAPI.pfnSubmitReport = &DefaultSubmitReport_f;
#ifdef _WIN32
	if( OpenLogger() ) {
		g_LoggingAPI.pfnSubmitReport = &LoggerSubmitReport_f;
	}
#endif
#ifdef _DEBUG
	g_LoggingAPI.MinimumPriority = kTenshiLog_Debug;
#else
	g_LoggingAPI.MinimumPriority = kTenshiLog_Info;
#endif
	g_LoggingAPI.DefaultFlags = kTenshiLogF_SystemInfo;

	g_MemblockAPI.pMemblockPool = teAllocEnginePool( &teMemblockAlloc_f, &teMemblockDealloc_f );

	g_MemblockAPI.pfnAllocMemblock = &teAllocMemblock;
	g_MemblockAPI.pfnMakeMemblock = &teMakeMemblock;
	g_MemblockAPI.pfnDeleteMemblock = &teDeleteMemblock;
	g_MemblockAPI.pfnMemblockExist = &teMemblockExist;
	g_MemblockAPI.pfnGetMemblockPtr = &teGetMemblockPtr;
	g_MemblockAPI.pfnGetMemblockSize = &teGetMemblockSize;

	g_RNGPool = teAllocEnginePool( &teRNGAlloc_f, &teRNGDealloc_f );

	InitModules();
}
static void __cdecl teFini( void )
{
	FiniModules();

	while( g_EngineTypes.cTypes > 0 ) {
		teFiniEnginePool( &g_EngineTypes.Pools[ g_EngineTypes.cTypes - 1 ] );
		--g_EngineTypes.cTypes;
	}

	g_RTGlob.pEngineTypes = NULL;
}

TENSHI_FUNC TenshiRuntimeGlob_t *TENSHI_CALL teGetGlob( void )
{
	return &g_RTGlob;
}

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_Memory

TENSHI_FUNC void *TENSHI_CALL teAlloc( TenshiUIntPtr_t cBytes, int Memtag )
{
	TenshiUIntPtr_t cAllocBytes;
	void *p;

	( void )Memtag;

	if( cBytes == 0 ) {
		return NULL;
	}

	cAllocBytes = cBytes + ( cBytes%16 != 0 ? cBytes + 16 - ( cBytes%16 ) : 0 );
	p = malloc( cAllocBytes );
#if MEMTRACE_ENABLED
	TRACE( "p=%p :: +%u byte%s",
		p, ( unsigned int )cBytes, cBytes == 1 ? "" : "s" );
#endif
	return p;
}
TENSHI_FUNC void TENSHI_CALL teDealloc( void *pData )
{
#if MEMTRACE_ENABLED
	if( !!pData ) {
		TRACE( "p=%p", pData );
	}
#endif
	free( pData );
#if MEMTRACE_ENABLED
	TRACE( "-" );
#endif
}

TENSHI_FUNC void TENSHI_CALL teAutoprint( const char *pszText )
{
	if( g_RTGlob.pfnAutoprintCallback != NULL ) {
		g_RTGlob.pfnAutoprintCallback( 0, pszText );
	} else {
		printf( "%s\n", pszText != NULL ? pszText : "" );
	}
}
TENSHI_FUNC int TENSHI_CALL teSafeSync( void )
{
	if( g_RTGlob.pfnSafeSyncCallback != NULL ) {
		return g_RTGlob.pfnSafeSyncCallback();
	}

	return 1;
}

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_Object

TENSHI_FUNC TenshiObjectPool_t *TENSHI_CALL teAllocEnginePool( TenshiFnAllocObject_t pfnAlloc, TenshiFnDeallocObject_t pfnDealloc )
{
	TenshiObjectPool_t *pPool;

	if( g_EngineTypes.cTypes == TENSHI_MAX_ENGINE_TYPES ) {
		return NULL;
	}

	pPool = &g_EngineTypes.Pools[ g_EngineTypes.cTypes++ ];

	pPool->pfnAlloc = pfnAlloc;
	pPool->pfnDealloc = pfnDealloc;
	pPool->ppObjects = NULL;
	pPool->pAges = NULL;
	pPool->cCapacity = 0;

	return pPool;
}
TENSHI_FUNC void TENSHI_CALL teFiniEnginePool( TenshiObjectPool_t *pPool )
{
	pPool->pfnAlloc = NULL;

	while( pPool->cCapacity > 0 ) {
		void **ppObj;

		ppObj = &pPool->ppObjects[ --pPool->cCapacity ];
		if( !*ppObj ) {
			continue;
		}

		pPool->pfnDealloc( *ppObj );
		*ppObj = NULL;
	}

	pPool->pfnDealloc = NULL;

	free( ( void * )pPool->ppObjects );
	pPool->ppObjects = NULL;

#if SAFE_HANDLES_ENABLED
	free( ( void * )pPool->pAges );
	pPool->pAges = NULL;
#endif
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teAllocEngineIndex( TenshiObjectPool_t *pPool, TenshiIndex_t uIndex )
{
	static const TenshiIndex_t kGrain = 4096/sizeof( void * );
	TenshiIndex_t cCapacity;
	TenshiIndex_t n;
	void **p;
#if SAFE_HANDLES_ENABLED
	TenshiUIntPtr_t *q;
#endif

	if( uIndex < pPool->cCapacity ) {
		return TENSHI_TRUE;
	}
	if( uIndex > TENSHI_MAX_INDEX ) {
		TRACE( "fail: index=%u capacity=%u",
			( unsigned )uIndex, ( unsigned )pPool->cCapacity );
		return TENSHI_FALSE;
	}

	cCapacity = uIndex - uIndex%kGrain + kGrain;
	p = ( void ** )realloc( ( void * )pPool->ppObjects, sizeof( void * )*cCapacity );
	if( !p ) {
		TRACE( "fail: out of memory (pool->objects); oldcap=%u newcap=%u",
			( unsigned )pPool->cCapacity, ( unsigned )cCapacity );
		return TENSHI_FALSE;
	}
#if SAFE_HANDLES_ENABLED
	q = ( TenshiUIntPtr_t * )realloc( ( void * )pPool->pAges, sizeof( TenshiUIntPtr_t )*cCapacity );
	if( !q ) {
		TRACE( "fail: out of memory (pool->ages); oldcap=%u newcap=%u",
			( unsigned )pPool->cCapacity, ( unsigned )cCapacity );
		free( ( void * )p );
		return TENSHI_FALSE;
	}
#endif

	n = cCapacity - pPool->cCapacity;
	memset( ( void * )( p + pPool->cCapacity ), 0, n*sizeof( void* ) );
#if SAFE_HANDLES_ENABLED
	memset( ( void * )( q + pPool->cCapacity ), 0, n*sizeof( TenshiUIntPtr_t ) );
#endif

	pPool->ppObjects = p;
#if SAFE_HANDLES_ENABLED
	pPool->pAges = q;
#endif
	pPool->cCapacity = n;

	return TENSHI_TRUE;
}
TENSHI_FUNC void TENSHI_CALL teReserveIndexes( TenshiObjectPool_t *pPool, TenshiIndex_t uBeginIndex, TenshiIndex_t uEndIndex )
{
	TenshiIndex_t i;

	if( !teAllocEngineIndex( pPool, uEndIndex ) ) {
		TRACE( "fail: out of memory" );
		return;
	}

	for( i = uBeginIndex; i < uEndIndex; ++i ) {
		if( pPool->ppObjects[ i ] != FREEPTR ) {
			continue;
		}

		pPool->ppObjects[ i ] = RESERVEDPTR;
	}
}
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teFindEngineIndex( TenshiObjectPool_t *pPool )
{
	TenshiIndex_t i, j;

	for( j = pPool->cCapacity; j > 0; --j ) {
		i = j - 1;

		if( pPool->ppObjects[ i ] == FREEPTR ) {
			return i;
		}
	}

	i = pPool->cCapacity;

	if( !teAllocEngineIndex( pPool, i ) ) {
		return TENSHI_INVALID_INDEX;
	}

	return i;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teEngineObjectExists( const TenshiObjectPool_t *pPool, TenshiIndex_t uIndex )
{
	TenshiIndex_t i;

	i = ( uIndex & TENSHI_MAX_INDEX ) - 1;

	if( i >= pPool->cCapacity ) {
		return TENSHI_FALSE;
	}

	if( pPool->ppObjects[ i ] == FREEPTR || pPool->ppObjects[ i ] == RESERVEDPTR ) {
		return TENSHI_FALSE;
	}

	return TENSHI_TRUE;
}
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocEngineObject( TenshiObjectPool_t *pPool, TenshiIndex_t uIndex, void *pParm )
{
	TenshiIndex_t i;

#ifdef _DEBUG
	if( !pPool->pfnAlloc ) {
		TRACE( "Attempted to allocate object during shut-down" );
		return 0;
	}
#endif

	i = !uIndex ? teFindEngineIndex( pPool ) : uIndex - 1;
	if( i >= TENSHI_MAX_INDEX || !teAllocEngineIndex( pPool, i ) ) {
		TRACE( "Invalid index or unable to allocate" );
		return 0;
	}

	pPool->ppObjects[ i ] = pPool->pfnAlloc( pParm );
	return teWrapEngineObject( pPool, i );
}
TENSHI_FUNC void TENSHI_CALL teDeallocEngineObject( TenshiObjectPool_t *pPool, TenshiIndex_t uIndex )
{
	TenshiUIntPtr_t i;
	void *p;

	if( !uIndex ) {
		return;
	}

	p = teUnwrapEngineObject( pPool, uIndex );
	if( !p ) {
		return;
	}

	i = ( uIndex & TENSHI_MAX_INDEX ) - 1;

	pPool->pfnDealloc( p );
	pPool->ppObjects[ i ] = FREEPTR;
#if SAFE_HANDLES_ENABLED
	{
		TenshiUIntPtr_t uAge;
		TenshiUIntPtr_t uAgeMask;

		uAgeMask = ~( 0xF<<( i%kNumAgesPerIndex ) );

		uAge = ( ( ( pPool->pAges[ i/kNumAgesPerIndex ] >> ( i%kNumAgesPerIndex ) ) + 1 ) & 0xF );
		pPool->pAges[ i/kNumAgesPerIndex ] &= uAgeMask;
		pPool->pAges[ i/kNumAgesPerIndex ] |= uAge<<( i%kNumAgesPerIndex );
	}
#endif
}
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teWrapEngineObject( TenshiObjectPool_t *pPool, TenshiIndex_t uUnwrappedIndex )
{
#if SAFE_HANDLES_ENABLED
	TenshiIndex_t uIndex;

	uIndex = uUnwrappedIndex + 1;
	return
		( ( ( ( TenshiIndex_t )( pPool - &g_EngineTypes.Pools[0] ) ) & 0x3F )<<26 ) |
		( ( ( pPool->pAges[ uUnwrappedIndex/kNumAgesPerIndex ] >> ( uUnwrappedIndex%kNumAgesPerIndex ) ) & 0xF ) << 22 ) |
		( uIndex & TENSHI_MAX_INDEX );
#else
	( void )pPool;
	return uUnwrappedIndex + 1;
#endif
}
TENSHI_FUNC void *TENSHI_CALL teUnwrapEngineObject( TenshiObjectPool_t *pPool, TenshiIndex_t uIndex )
{
	TenshiIndex_t i;

	i = ( uIndex & TENSHI_MAX_INDEX ) - 1;

#if SAFE_HANDLES_ENABLED
	if( i >= pPool->cCapacity ) {
		TRACE( "Index (%u) is outside of the pool's capacity (%u)", i, pPool->cCapacity );
		return NULL;
	}

	if( ( uIndex & ~TENSHI_MAX_INDEX ) != 0 ) {
		if( pPool != &g_EngineTypes.Pools[ ( uIndex>>26 ) & 0x3F ] ) {
			TRACE( "Index (%u) refers to a different pool", i );
			return NULL;
		}

		if( pPool->pAges[ i/kNumAgesPerIndex ] >> ( i%kNumAgesPerIndex ) != ( ( uIndex>>22 ) & 0xF ) ) {
			TRACE( "Loose index detected (%u)", i );
			return NULL;
		}
	}
#endif

	return pPool->ppObjects[ i ] != RESERVEDPTR ? pPool->ppObjects[ i ] : NULL;
}


/*
===============================================================================

	STRINGS

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_String

TENSHI_FUNC char *TENSHI_CALL teStrAlloc( char *p, TenshiUIntPtr_t n )
{
	char *q;

#if STRTRACE_ENABLED
	TRACE( "p=%p; n=%u", ( void * )p, ( unsigned int )n );
#endif

	q = n > 0 ? teAlloc( n + 1, TENSHI_MEMTAG_STRING ) : NULL;

	if( p != NULL ) {
		if( q != NULL ) {
			memcpy( q, p, strlen( p ) + 1 );
		}

		teDealloc( p );
		p = NULL;
	} else if( q != NULL ) {
		*q = '\0';
		q[ n ] = '\0';
	}

	if( n > 0 && !q ) {
		fprintf( stderr, "ERROR: Out of memory\n" );
		exit( EXIT_FAILURE );
	}

	return q;
}
TENSHI_FUNC char *TENSHI_CALL teStrReclaim( char *s )
{
	teDealloc( s );
	return NULL;
}

TENSHI_FUNC char *TENSHI_CALL teStrDup( const char *s )
{
	size_t n;
	char *p;

#if STRTRACE_ENABLED
	TRACE( "s=%p", ( const void * )s );
#endif

	if( !s ) {
		return NULL;
	}

	n = strlen( s );
	p = teStrAlloc( NULL, n );

	memcpy( p, s, n + 1 );

	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStrConcat( const char *a, const char *b )
{
	size_t alen, blen;
	size_t len;
	char *p;

#if STRTRACE_ENABLED
	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );
#endif

	if( !a || !b ) {
		if( a != NULL ) {
			return teStrDup( a );
		}
		
		if( b != NULL ) {
			return teStrDup( b );
		}

		return NULL;
	}

	alen = strlen( a );
	blen = strlen( b );
	len = alen + blen;

#if STRTRACE_ENABLED
	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );
#endif

	p = teStrAlloc( NULL, len );

	memcpy( p, a, alen );
	memcpy( p + alen, b, blen + 1 );

	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStrFindRm( const char *a, const char *b )
{
#define MAX_OCCURRENCES 512
	const char *occurrences[ MAX_OCCURRENCES ];
	size_t c_occurrences;
	size_t i;

	const char *base;
	const char *s;
	const char *t;

	size_t alen, blen;
	size_t len;
	
	char *p;
	char *q;

#if STRTRACE_ENABLED
	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );
#endif

	if( !a || !b ) {
		return ( char * )a;
	}

	blen = strlen( b );
	if( !blen ) {
		return ( char * )a;
	}

	alen = strlen( a );
	p = NULL;
	q = NULL;

#if STRTRACE_ENABLED
	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );
#endif

	len = alen;
	s = a;
	base = a;
	do {
		c_occurrences = 0;
		for(;;) {
			t = strstr( s, b );
			if( !t ) {
				occurrences[ c_occurrences++ ] = strchr( s, '\0' );
				s = NULL;
				break;
			}

#if STRTRACE_ENABLED
			TRACE
			(
				"#%.2u@%p (len{%u}-=(t{%p}-s{%p}){%u}){%u}",
				( unsigned int )c_occurrences, ( const void * )t,
				( unsigned int )len,
				( const void * )t, ( const void * )s,
				( unsigned int )( size_t )( t - s ),
				( unsigned int )( len - ( size_t )( t - s ) )
			);
#endif

			occurrences[ c_occurrences++ ] = t;
			len -= blen;
			s = t + blen;

			if( c_occurrences == MAX_OCCURRENCES ) {
				break;
			}
		}

		q -= ( size_t )p;
		p = teStrAlloc( p, len );
		q += ( size_t )p;
#if STRTRACE_ENABLED
		TRACE( "cp(p=%p,len=%u)x%u",
			( const void * )p, ( unsigned int )len,
			( unsigned int )c_occurrences );
#endif

		for( i = 0; i < c_occurrences; ++i ) {
			t = occurrences[ i ];

#if STRTRACE_ENABLED
			TRACE
			(
				"~%.2u dst=q{%p} src=base{%p}, cnt=(t{%p} - base{%p}){%u}",
				i,
				( const void * )q, ( const void * )base,
				( const void * )t, ( const void * )base,
				( unsigned int )( size_t )( t - base )
			);
#endif

			memcpy( q, base, t - base );
			q += ( size_t )( t - base );
			base += ( size_t )( t - base ) + blen;
		}
	} while( s != NULL );

	if( q != NULL ) {
		*q = '\0';
	}
#if STRTRACE_ENABLED
	TRACE( "ret=%p, ret.num=%u", ( const void * )p, ( unsigned int )( size_t )( q - p ) );
#endif

	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStrRepeat( const char *s, TenshiUIntPtr_t n )
{
	unsigned int i;
	size_t slen;
	char *p;

#if STRTRACE_ENABLED
	TRACE( "s=%p, n=%u", ( const void * )s, n );
#endif

	slen = s != NULL && n > 0 ? strlen( s ) : 0;
	if( !slen ) {
		return NULL;
	}

	p = teStrAlloc( NULL, slen*( size_t )n );

	for( i = 0; i < n; ++i ) {
		memcpy( p + slen*i, s, slen );
	}

	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStrCatDir( const char *a, const char *b )
{
	size_t alen, blen;
	size_t len;
	char *p;
	char chinsert;

#if STRTRACE_ENABLED
	TRACE( "a=%p, b=%p", ( const void * )a, ( const void * )b );
#endif

	if( !a || !b ) {
		if( a != NULL ) {
			return teStrDup( a );
		}

		if( b != NULL ) {
			return teStrDup( b );
		}

		return NULL;
	}

	alen = strlen( a );
	blen = strlen( b );

#if STRTRACE_ENABLED
	TRACE( "a.len=%u, b.len=%u", ( unsigned int )alen, ( unsigned int )blen );
#endif

	len = alen + blen;
	chinsert = '\0';

	if( alen > 0 && a[ alen - 1 ] != '/' ) {
#ifdef _WIN32
		if( a[ alen - 1 ] != '\\' ) {
			chinsert = '/';
			++len;
		}
#else
		chinsert = '/';
		++len;
#endif
	}

	p = teStrAlloc( NULL, len );

	if( alen > 0 ) {
		memcpy( p, a, alen );
	}

	if( chinsert != '\0' ) {
		p[ alen++ ] = chinsert;
	}

	memcpy( p + alen, b, blen );

	return p;
}

static void teCastSignPart_( char *pchsign, TenshiInt64_t *pv )
{
	if( !pchsign || !pv || *pv >= 0 ) {
		if( pchsign != NULL ) {
			*pchsign = '\0';
		}
		return;
	}

	*pchsign = '-';
	*pv = -*pv;
}
static void teCastSignPartWrite_( char **pbuf, char *sbuf, char chsign )
{
	if( !pbuf || !*pbuf || !sbuf || *pbuf <= sbuf || chsign == '\0' ) {
		return;
	}

	*--*pbuf = chsign;
}
static void teCastUIntPart_( char **pbuf, char *sbuf, TenshiUInt64_t v )
{
	TenshiUInt64_t n;
	char *p;

	if( !pbuf || !*pbuf || !sbuf ) {
		return;
	}

	p = *pbuf;
	n = v;

	if( !v ) {
		*--p = '0';
		*pbuf = p;
		return;
	}

	while( p > sbuf && n > 0 ) {
		*--p = '0' + n%10;
		n /= 10;
	}

	*pbuf = p;
}

TENSHI_FUNC char *TENSHI_CALL teCastInt8ToStr( TenshiInt8_t i )
{
	return teCastInt64ToStr( ( TenshiInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastInt16ToStr( TenshiInt16_t i )
{
	return teCastInt64ToStr( ( TenshiInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastInt32ToStr( TenshiInt32_t i )
{
	return teCastInt64ToStr( ( TenshiInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastInt64ToStr( TenshiInt64_t i )
{
	char buf[ 64 ], *pbuf;
	char chsign;

	pbuf = &buf[ sizeof( buf ) - 1 ];
	*pbuf = '\0';

	teCastSignPart_( &chsign, &i );
	teCastUIntPart_( &pbuf, &buf[0], ( TenshiUInt64_t )i );
	teCastSignPartWrite_( &pbuf, &buf[0], chsign );

	return teStrDup( pbuf );
}
TENSHI_FUNC char *TENSHI_CALL teCastInt128ToStr( TenshiInt128Struct_t i )
{
	static int didshow = 0;
	( void )i;
	if( !didshow ) {
		didshow = 1;
		fprintf( stderr, "ERROR: Casting 128-bit integer to string not supported (yet)\n" );
	}
	return NULL;
}

TENSHI_FUNC char *TENSHI_CALL teCastUInt8ToStr( TenshiUInt8_t i )
{
	return teCastUInt64ToStr( ( TenshiUInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastUInt16ToStr( TenshiUInt16_t i )
{
	return teCastUInt64ToStr( ( TenshiUInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastUInt32ToStr( TenshiUInt32_t i )
{
	return teCastUInt64ToStr( ( TenshiUInt64_t )i );
}
TENSHI_FUNC char *TENSHI_CALL teCastUInt64ToStr( TenshiUInt64_t i )
{
	char buf[ 64 ], *pbuf;

	pbuf = &buf[ sizeof( buf ) - 1 ];
	*pbuf = '\0';

	teCastUIntPart_( &pbuf, &buf[0], i );

	return teStrDup( pbuf );
}
TENSHI_FUNC char *TENSHI_CALL teCastUInt128ToStr( TenshiInt128Struct_t i )
{
	static int didshow = 0;
	( void )i;
	if( !didshow ) {
		didshow = 1;
		fprintf( stderr, "ERROR: Casting 128-bit unsigned integer to string not supported (yet)\n" );
	}
	return NULL;
}

/* FIXME: These call into sprintf -- no please */

TENSHI_FUNC char *TENSHI_CALL teCastFloat32ToStr( float f )
{
	char buf[ 128 ];

#ifdef _MSC_VER
	sprintf_s( buf, sizeof( buf ), "%g", f );
#else
	snprintf( buf, sizeof( buf ), "%g", f );
#endif

	return teStrDup( buf );
}
TENSHI_FUNC char *TENSHI_CALL teCastFloat64ToStr( double f )
{
	char buf[ 128 ];

#ifdef _MSC_VER
	sprintf_s( buf, sizeof( buf ), "%g", f );
#else
	snprintf( buf, sizeof( buf ), "%g", f );
#endif

	return teStrDup( buf );
}

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrInstance_Init_f( TenshiType_t *pType, void *pInstance )
{
	( void )pType;

	*((char**)pInstance) = (char*)0;
	return TENSHI_TRUE;
}
TENSHI_FUNC void TENSHI_CALL teStrInstance_Fini_f( TenshiType_t *pType, void *pInstance )
{
	( void )pType;
	*( char ** )pInstance = teStrAlloc( *( char ** )pInstance, 0 );
}
TENSHI_FUNC void TENSHI_CALL teStrInstance_Copy_f( TenshiType_t *pType, void *pDstInstance, const void *pSrcInstance )
{
	( void )pType;
	*( char ** )pDstInstance = teStrDup( *( const char *const * )pSrcInstance );
}
TENSHI_FUNC void TENSHI_CALL teStrInstance_Move_f( TenshiType_t *pType, void *pDstInstance, void *pSrcInstance )
{
	( void )pType;

	*( char ** )pDstInstance = *( char ** )pSrcInstance;
	*( char ** )pSrcInstance = ( char * )0;
}

/* -------------------------------------------------------------------------- */

TENSHI_FUNC int TENSHI_CALL teStr_Asc( const char *s )
{
	if( !s ) {
		return 0;
	}

	return +*s;
}
TENSHI_FUNC char *TENSHI_CALL teStr_Chr( TenshiUInt32_t utf32cp )
{
	TenshiUIntPtr_t n;
	char *dst;
	char buf[ 5 ];

	dst = &buf[ 0 ];

	if( utf32cp > 0x10000 ) {
		*dst++ = ( TenshiUInt8_t )( 0xF0 | ( ( utf32cp>>18 ) & 0x07 ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>>12 ) & 0x3F ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>> 6 ) & 0x3F ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>> 0 ) & 0x3F ) );
	} else if( utf32cp > 0x7FF ) {
		*dst++ = ( TenshiUInt8_t )( 0xE0 | ( ( utf32cp>>12 ) & 0x0F ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>> 6 ) & 0x3F ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>> 0 ) & 0x3F ) );
	} else if( utf32cp > 0x7F ) {
		*dst++ = ( TenshiUInt8_t )( 0xC0 | ( ( utf32cp>> 6 ) & 0x1F ) );
		*dst++ = ( TenshiUInt8_t )( 0x80 | ( ( utf32cp>> 0 ) & 0x3F ) );
	} else {
		*dst++ = ( TenshiUInt8_t )utf32cp;
	}
	*dst = '\0';

	return teStrDup( buf );
}

TENSHI_FUNC char *TENSHI_CALL teStr_Bin( TenshiUInt32_t x )
{
	char buf[ 33 ];
	char *dst;

	dst = &buf[ 32 ];
	*dst = '\0';

	do {
		*--dst = '0' + ( char )( unsigned char )( x%2 );
		x /= 2;
	} while( x != 0 );

	return teStrDup( dst );
}
TENSHI_FUNC char *TENSHI_CALL teStr_Hex( TenshiUInt32_t x )
{
	static const char *const digits = "0123456789ABCDEF";
	char buf[ 9 ];
	char *dst;

	dst = &buf[ 8 ];
	*dst = '\0';

	do {
		*--dst = digits[ x%16 ];
		x /= 16;
	} while( x != 0 );

	return teStrDup( dst );
}
TENSHI_FUNC char *TENSHI_CALL teStr_Oct( TenshiUInt32_t x )
{
	char buf[ 12 ];
	char *dst;

	dst = &buf[ 11 ];
	*dst = '\0';

	do {
		*--dst = '0' + ( char )( unsigned char )( x%8 );
		x /= 8;
	} while( x != 0 );

	return teStrDup( dst );
}

TENSHI_FUNC char *TENSHI_CALL teStr_Lower( const char *s )
{
	char *p, *q;

	if( !s || !( p = teStrDup( s ) ) ) {
		return ( char * )0;
	}

	for( q = p; *q != '\0'; ++q ) {
		if( *q >= 'A' && *q <= 'Z' ) {
			*q = *q - 'A' + 'a';
		}
	}

	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStr_Upper( const char *s )
{
	char *p, *q;

	if( !s || !( p = teStrDup( s ) ) ) {
		return ( char * )0;
	}

	for( q = p; *q != '\0'; ++q ) {
		if( *q >= 'a' && *q <= 'z' ) {
			*q = *q - 'a' + 'A';
		}
	}

	return p;
}

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teStr_Len( const char *s )
{
	if( !s ) {
		return 0;
	}

	return strlen( s );
}
TENSHI_FUNC int TENSHI_CALL teStr_SortCmp( const char *a, const char *b )
{
	if( !a || !b ) {
		return a > b ? 1 : b > a ? -1 : 0;
	}

	return strcmp( a, b );
}
TENSHI_FUNC int TENSHI_CALL teStr_SortCmpCase( const char *a, const char *b )
{
	if( !a || !b ) {
		return a > b ? 1 : b > a ? -1 : 0;
	}

#ifdef _WIN32
	return _stricmp( a, b );
#else
	return strcasecmp( a, b );
#endif
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrEq( const char *a, const char *b )
{
	return a == b ? 1 : !a || !b ? 0 : strcmp( a, b ) == 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrEqCase( const char *a, const char *b )
{
	return
		a == b ? 1 : !a || !b ? 0 :
#ifdef _WIN32
			_stricmp( a, b ) == 0
#else
			strcasecmp( a, b ) == 0
#endif
			;
}

TENSHI_FUNC char *TENSHI_CALL teStr_Left( const char *s, TenshiIntPtr_t n )
{
	TenshiUIntPtr_t slen;
	TenshiUIntPtr_t rlen;
	char *p;

	if( !s ) {
		return ( char * )0;
	}

	slen = teStr_Len( s );
	rlen = ( TenshiUIntPtr_t )( n < 0 ? slen - n : n );

	if( rlen >= slen ) {
		return teStrDup( s );
	}

	if( !( p = teStrAlloc( NULL, rlen ) ) ) {
		return ( char * )0;
	}

	memcpy( ( void * )p, ( const void * )s, rlen );
	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStr_Mid( const char *s, TenshiIntPtr_t pos )
{
	return teStr_MidLen( s, pos, 1 );
}
TENSHI_FUNC char *TENSHI_CALL teStr_MidLen( const char *s, TenshiIntPtr_t pos, TenshiUIntPtr_t len )
{
	TenshiUIntPtr_t slen, off;
	char *p;

	if( !s || !len ) {
		return ( char * )0;
	}

	slen = teStr_Len( s );
	off = ( TenshiUIntPtr_t )( pos >= 0 ? ( pos < slen ? pos : slen ) : slen + pos );

	if( off + len > slen ) {
		len = slen - off;
	}

	if( !len ) {
		return ( char * )0;
	}

	if( !( p = teStrAlloc( NULL, len ) ) ) {
		return ( char * )0;
	}

	memcpy( ( void * )p, ( const void * )( s + off ), len );
	return p;
}
TENSHI_FUNC char *TENSHI_CALL teStr_Right( const char *s, TenshiIntPtr_t n )
{
	TenshiUIntPtr_t slen, len;

	if( !s || !n ) {
		return ( char * )0;
	}

	slen = teStr_Len( s );
	len = ( TenshiUIntPtr_t )( n >= 0 ? ( n <= slen ? n : slen ) : slen + n );

	if( !len ) {
		return ( char * )0;
	}

	return teStrDup( s + ( slen - len ) );
}
TENSHI_FUNC char *TENSHI_CALL teStr_Skip( const char *s, TenshiUIntPtr_t n )
{
	if( !s ) {
		return ( char * )0;
	}

	while( n > 0 ) {
		if( *s == '\0' ) {
			break;
		}

		++s;
		--n;
	}

	return teStrDup( s );
}
TENSHI_FUNC char *TENSHI_CALL teStr_Drop( const char *s, TenshiUIntPtr_t n )
{
	TenshiUIntPtr_t slen;
	char *p;

	if( !s ) {
		return ( char * )0;
	}

	if( !n ) {
		return teStrDup( s );
	}

	slen = teStr_Len( s );
	if( n >= slen ) {
		return ( char * )0;
	}

	if( !( p = teStrAlloc( NULL, slen - n ) ) ) {
		return ( char * )0;
	}

	memcpy( ( void * )p, ( const void * )s, slen - n );
	return p;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_HasPrefix( const char *s, const char *prefix )
{
	if( !prefix || *prefix == '\0' ) {
		return 1;
	}
	if( !s || *s == '\0' ) {
		return 0;
	}

	return strncmp( s, prefix, strlen( prefix ) ) == 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_HasSuffix( const char *s, const char *suffix )
{
	TenshiUIntPtr_t alen, blen;

	if( !suffix || *suffix == '\0' ) {
		return 1;
	}
	if( !s || *s == '\0' ) {
		return 0;
	}

	alen = teStr_Len( s );
	blen = teStr_Len( suffix );

	if( blen > alen ) {
		return 0;
	}

	return strcmp( s + ( alen - blen ), suffix ) == 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_Contains( const char *s, const char *search )
{
	if( !search || *search == '\0' ) {
		return 1;
	}
	if( !s || *s == '\0' ) {
		return 0;
	}

	return strstr( s, search ) != ( const char * )0;
}


/*
===============================================================================

	TYPES

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_Type

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsTypeTrivial( const TenshiType_t *pType )
{
	return ( pType != NULL && ( pType->Flags & kTenshiTypeF_FullTrivial ) == kTenshiTypeF_FullTrivial ) ? TENSHI_TRUE : TENSHI_FALSE;
}
#define teTypeHasTrivialInit(pType_) (!(pType_) || ((pType_)->Flags & kTenshiTypeF_TrivialInit))
#define teTypeHasTrivialFini(pType_) (!(pType_) || ((pType_)->Flags & kTenshiTypeF_TrivialFini))
#define teTypeHasTrivialCopy(pType_) (!(pType_) || ((pType_)->Flags & kTenshiTypeF_TrivialCopy))
#define teTypeHasTrivialMove(pType_) (!(pType_) || ((pType_)->Flags & kTenshiTypeF_TrivialMove))

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teInitTypeInstance( TenshiType_t *pType, void *pInstance )
{
	return pType != NULL ? pType->pfnInit( pType, pInstance ) : TENSHI_FALSE;
}
TENSHI_FUNC void TENSHI_CALL teFiniTypeInstance( TenshiType_t *pType, void *pInstance )
{
	if( !pType || !pInstance ) {
		return;
	}

	pType->pfnFini( pType, pInstance );
}
TENSHI_FUNC void TENSHI_CALL teCopyTypeInstance( TenshiType_t *pType, void *pDstInstance, const void *pSrcInstance )
{
	if( !pType || !pDstInstance || !pSrcInstance ) {
		return;
	}

	pType->pfnCopy( pType, pDstInstance, pSrcInstance );
}
TENSHI_FUNC void TENSHI_CALL teMoveTypeInstance( TenshiType_t *pType, void *pDstInstance, void *pSrcInstance )
{
	if( !pType || !pDstInstance || !pSrcInstance ) {
		return;
	}

	pType->pfnMove( pType, pDstInstance, pSrcInstance );
}

TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teNewTypeObject( TenshiType_t *pType )
{
	TenshiTypeObject_t *pObj;

	if( !pType ) {
		return NULL;
	}

	pObj = ( TenshiTypeObject_t * )teAlloc( sizeof( TenshiTypeObject_t ) + pType->cBytes, CURRENT_MEMTAG );
	if( !pObj ) {
		return NULL;
	}

	pObj->cReferences = 1;
	pObj->pType = pType;

	if( !teInitTypeInstance( pType, ( void * )( pObj + 1 ) ) ) {
		teDealloc( ( void * )pObj );
		return NULL;
	}

	return pObj;
}
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teDeleteTypeObject( TenshiTypeObject_t *pObj )
{
	if( !pObj ) {
		return NULL;
	}

	if( --pObj->cReferences != 0 ) {
		return NULL;
	}

	teFiniTypeInstance( pObj->pType, ( void * )( pObj + 1 ) );
	teDealloc( ( void * )pObj );

	return NULL;
}
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teKeepTypeObject( TenshiTypeObject_t *pObj )
{
	if( !pObj ) {
		return NULL;
	}

	++pObj->cReferences;
	return pObj;
}
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teCopyTypeObject( const TenshiTypeObject_t *pObj )
{
	TenshiTypeObject_t *pCopy;
	if( !pObj ) {
		return NULL;
	}
	
	pCopy = ( TenshiTypeObject_t * )teAlloc( sizeof( TenshiTypeObject_t ) + pObj->pType->cBytes, CURRENT_MEMTAG );
	if( !pCopy ) {
		return NULL;
	}

	pCopy->cReferences = 1;
	pCopy->pType = pObj->pType;

	teCopyTypeInstance( pObj->pType, ( void * )( pCopy + 1 ), ( const void * )( pObj + 1 ) );
	return pCopy;
}
TENSHI_FUNC void *TENSHI_CALL teGetTypeObject( TenshiTypeObject_t *pObj )
{
	return pObj != NULL ? ( void * )( pObj + 1 ) : NULL;
}
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teGetTypeObjectSize( const TenshiTypeObject_t *pObj )
{
	return pObj != NULL ? pObj->pType->cBytes : 0;
}

TENSHI_FUNC TenshiType_t *TENSHI_CALL teFixType( TenshiType_t *p )
{
	static TenshiType_t NativeTy[ 20 ];
	static int DidInit = 0;

	const unsigned n = ( unsigned )( sizeof( NativeTy )/sizeof( NativeTy[ 0 ] ) );
	unsigned i;

	if( !p || ((TenshiUIntPtr_t)p) > (TenshiUIntPtr_t)n ) {
		return p;
	}

	if( !DidInit ) {
		for( i = 0; i < n; ++i ) {
			NativeTy[ i ].Flags = kTenshiTypeF_FullTrivial;
			NativeTy[ i ].pfnInit = ( TenshiFnInstanceInit_t )0;
			NativeTy[ i ].pfnFini = ( TenshiFnInstanceFini_t )0;
			NativeTy[ i ].pfnCopy = ( TenshiFnInstanceCopy_t )0;
			NativeTy[ i ].pfnMove = ( TenshiFnInstanceMove_t )0;
		}

		NativeTy[  0 ].cBytes = 1;
		NativeTy[  1 ].cBytes = 2;
		NativeTy[  2 ].cBytes = 4;
		NativeTy[  3 ].cBytes = 8;
		NativeTy[  4 ].cBytes = 16;
		NativeTy[  5 ].cBytes = sizeof( void * );

		NativeTy[  6 ].cBytes = 1;
		NativeTy[  7 ].cBytes = 2;
		NativeTy[  8 ].cBytes = 4;
		NativeTy[  9 ].cBytes = 8;
		NativeTy[ 10 ].cBytes = 16;
		NativeTy[ 11 ].cBytes = sizeof( void * );

		NativeTy[ 12 ].cBytes = 2;
		NativeTy[ 13 ].cBytes = 4;
		NativeTy[ 14 ].cBytes = 8;

		NativeTy[ 15 ].cBytes = 1;

		NativeTy[ 16 ].cBytes = sizeof( char * );
		NativeTy[ 17 ].cBytes = sizeof( void * );
		NativeTy[ 18 ].cBytes = sizeof( void * );
		NativeTy[ 19 ].cBytes = sizeof( void * );

		NativeTy[  0 ].pszName = "int8";
		NativeTy[  0 ].pszPattern = "C";

		NativeTy[  1 ].pszName = "int16";
		NativeTy[  1 ].pszPattern = "N";

		NativeTy[  2 ].pszName = "int32";
		NativeTy[  2 ].pszPattern = "L";

		NativeTy[  3 ].pszName = "int64";
		NativeTy[  3 ].pszPattern = "R";

		NativeTy[  4 ].pszName = "int128";
		NativeTy[  4 ].pszPattern = "EI16";

		NativeTy[  5 ].pszName = "intptr";
		NativeTy[  5 ].pszPattern = "IP";

		NativeTy[  6 ].pszName = "uint8";
		NativeTy[  6 ].pszPattern = "Y";

		NativeTy[  7 ].pszName = "uint16";
		NativeTy[  7 ].pszPattern = "W";

		NativeTy[  8 ].pszName = "uint32";
		NativeTy[  8 ].pszPattern = "D";

		NativeTy[  9 ].pszName = "uint64";
		NativeTy[  9 ].pszPattern = "Q";

		NativeTy[ 10 ].pszName = "uint128";
		NativeTy[ 10 ].pszPattern = "EU16";

		NativeTy[ 11 ].pszName = "uintptr";
		NativeTy[ 11 ].pszPattern = "UP";

		NativeTy[ 12 ].pszName = "float16";
		NativeTy[ 12 ].pszPattern = "A";

		NativeTy[ 13 ].pszName = "float32";
		NativeTy[ 13 ].pszPattern = "F";

		NativeTy[ 14 ].pszName = "float64";
		NativeTy[ 14 ].pszPattern = "O";

		NativeTy[ 15 ].pszName = "boolean";
		NativeTy[ 15 ].pszPattern = "B";

		NativeTy[ 16 ].pszName = "string";
		NativeTy[ 16 ].pszPattern = "G";
		NativeTy[ 16 ].pfnInit = &teStrInstance_Init_f;
		NativeTy[ 16 ].pfnFini = &teStrInstance_Fini_f;
		NativeTy[ 16 ].pfnCopy = &teStrInstance_Copy_f;
		NativeTy[ 16 ].pfnMove = &teStrInstance_Move_f;

		NativeTy[ 17 ].pszName = "array";
		NativeTy[ 17 ].pszPattern = "H";

		NativeTy[ 18 ].pszName = "list";
		NativeTy[ 18 ].pszPattern = "K";

		NativeTy[ 19 ].pszName = "btree";
		NativeTy[ 19 ].pszPattern = "M";

		DidInit = 1;
	}

	return &NativeTy[ ( ( TenshiUIntPtr_t )p ) - 1 ];
}


/*
===============================================================================

	ARRAYS

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_Array

static const TenshiArray_t *ArrayFromConstData( const void *pData )
{
	return ( ( const TenshiArray_t * )pData ) - 1;
}
static TenshiArray_t *ArrayFromData( void *pData )
{
	return ( ( TenshiArray_t * )pData ) - 1;
}
#if 0
static const void *ConstDataFromArray( const TenshiArray_t *pArray )
{
	return ( const void * )( pArray + 1 );
}
#endif

static void *DataFromArray( const TenshiArray_t *pArray )
{
	return ( void * )( pArray + 1 );
}

#if 0
static const void *ArrayConstItemPointer( const TenshiArray_t *pArray, TenshiUIntPtr_t uItemIndex )
{
	return ( const void * )( ( ( TenshiUIntPtr_t )ConstDataFromArray( pArray ) ) + uItemIndex*pArray->cItemBytes );
}
static void *ArrayItemPointer( TenshiArray_t *pArray, TenshiUIntPtr_t uItemIndex )
{
	return ( void * )( ( ( TenshiUIntPtr_t )DataFromArray( pArray ) ) + uItemIndex*pArray->cItemBytes );
}
#endif

#if 0
static TenshiUIntPtr_t CalculateMultidimensionalIndex( const TenshiArray_t *pArray, const TenshiUIntPtr_t *pDimIndexes, TenshiUIntPtr_t cDims )
{
	TenshiUIntPtr_t uCalcIndex;
	TenshiUIntPtr_t uDimMult;
	TenshiUIntPtr_t i;

	if( cDims > pArray->cDimensions ) {
		return TENSHI_ARRAY_INVALID_INDEX;
	}

	/* H*W*z + W*y + x */
	
	uCalcIndex = 0;
	uDimMult = 1;
	for( i = 0; i < cDims; ++i ) {
		uCalcIndex += pDimIndexes[ i ]*uDimMult;
		uDimMult *= pArray->uDimensions[ i ];
	}

	return uCalcIndex;
}
#endif

static TenshiUIntPtr_t CalculateItemCount( const TenshiUIntPtr_t *pDimensions, TenshiUIntPtr_t cDimensions )
{
	TenshiUIntPtr_t cItems;
	TenshiUIntPtr_t i;

	cItems = cDimensions > 0 ? 1 : 0;
	for( i = 0; i < cDimensions; ++i ) {
		cItems *= pDimensions[ i ];
	}

	return cItems;
}

TENSHI_FUNC void *TENSHI_CALL teArrayUndim( void *pArrayData )
{
	TenshiArray_t *pArr;

	TRACE( "pArrayData=%p", pArrayData );

	if( !pArrayData ) {
		return NULL;
	}

	pArr = ArrayFromData( pArrayData );
	if( !teTypeHasTrivialFini( pArr->pItemType ) ) {
		TenshiUIntPtr_t uDataAddr;
		TenshiUIntPtr_t i;
		
		uDataAddr = ( TenshiUIntPtr_t )pArrayData;
		for( i = 0; i < pArr->cItems; ++i ) {
			teFiniTypeInstance( pArr->pItemType, ( void * )uDataAddr );
			uDataAddr += pArr->cItemBytes;
		}
	}

	teDealloc( ( void * )pArr );
	return NULL;
}
TENSHI_FUNC void *TENSHI_CALL teArrayDim( const TenshiUIntPtr_t *pDimensions, TenshiUIntPtr_t cDimensions, TenshiType_t *pItemType )
{
	TenshiUIntPtr_t cItemBytes;
	TenshiUIntPtr_t cItems;
	TenshiUIntPtr_t cDataBytes;
	TenshiUIntPtr_t cBytes;
	TenshiArray_t *pArr;
	TenshiUIntPtr_t i;
	void *pBaseData;

	TRACE( "Enter" );

	pItemType = teFixType( pItemType );
	if( !pItemType ) {
		TRACE( "Leave: Received null pItemType" );
		return NULL;
	}

	cItemBytes = pItemType->cBytes;
	if( !cItemBytes ) {
		TRACE( "Leave: cItemBytes == 0 for pItemType<%s>", pItemType->pszName );
		return NULL;
	}

	cItems = CalculateItemCount( pDimensions, cDimensions );
	cDataBytes = cItemBytes*cItems;
	cBytes = sizeof( TenshiArray_t ) + cDataBytes;

	pArr = ( TenshiArray_t * )teAlloc( cBytes, g_RTGlob.iCurrentMemtag );
	if( !pArr ) {
		TRACE( "Leave: Alloc failed" );
		return NULL;
	}

	for( i = 0; i < cDimensions; ++i ) {
		pArr->uDimensions[ i ] = pDimensions[ i ];
	}
	for( i = cDimensions; i < TENSHI_ARRAY_MAX_DIMENSIONS; ++i ) {
		pArr->uDimensions[ i ] = 0;
	}
	pArr->cDimensions = cDimensions;

	pArr->cItems = cItems;
	pArr->cItemBytes = cItemBytes;
	pArr->pItemType = pItemType;
	pArr->uIndex = 0;

	pBaseData = DataFromArray( pArr );
	if( teTypeHasTrivialInit( pItemType ) ) {
		memset( pBaseData, 0, cDataBytes );
	} else {
		TenshiUIntPtr_t uDataAddr;

		uDataAddr = ( TenshiUIntPtr_t )pBaseData;
		for( i = 0; i < cItems; ++i ) {
			teInitTypeInstance( pItemType, ( void * )uDataAddr );
			uDataAddr += cItemBytes;
		}
	}

	TRACE( "Leave: Success pBaseData=%p", pBaseData );
	return pBaseData;
}
TENSHI_FUNC void *TENSHI_CALL teArrayRedim( void *pOldArrayData, const TenshiUIntPtr_t *pDimensions, TenshiUIntPtr_t cDimensions, TenshiType_t *pItemType )
{
	TenshiArray_t *pOldArray;
	TenshiArray_t *pArr;
	TenshiUIntPtr_t cItems;
	TenshiUIntPtr_t cDataBytes;
	TenshiUIntPtr_t cBytes;
	TenshiUIntPtr_t i;
	TenshiUIntPtr_t uOldBases		[ TENSHI_ARRAY_MAX_DIMENSIONS ];
	TenshiUIntPtr_t uNewBases		[ TENSHI_ARRAY_MAX_DIMENSIONS ];
	void *pOldBaseAddr;
	void *pNewBaseAddr;
	TenshiUIntPtr_t uOldBaseAddr;
	TenshiUIntPtr_t uNewBaseAddr;
	TenshiUIntPtr_t cMoveDims;
	TenshiUIntPtr_t uDim;
	TenshiUIntPtr_t uDstDimAddr;
	TenshiUIntPtr_t uSrcDimAddr;
	TenshiUIntPtr_t cOldItems;
	TenshiUIntPtr_t cNewItems;
	TenshiUIntPtr_t cMoveItems;
	TenshiUIntPtr_t cMakeItems;
	TenshiUIntPtr_t uItem;

	TRACE( "Enter" );

	pItemType = teFixType( pItemType );
	if( !pItemType ) {
		TRACE( "Leave: pItemType is null" );
		return NULL;
	}

	if( !pOldArrayData ) {
		TRACE( "Leave: No old data; just call Dim" );
		return teArrayDim( pDimensions, cDimensions, pItemType );
	}

	pOldArray = ArrayFromData( pOldArrayData );
	if( pOldArray->pItemType != pItemType ) {
		TRACE( "Leave: Old array type does not match given type; old=%p new=%p",
			( void * )pOldArray->pItemType, ( void * )pItemType );
		return NULL;
	}

	cItems = CalculateItemCount( pDimensions, cDimensions );
	cDataBytes = pOldArray->cItemBytes*cItems;
	cBytes = sizeof( TenshiArray_t ) + cDataBytes;

	pArr = ( TenshiArray_t * )teAlloc( cBytes, g_RTGlob.iCurrentMemtag );
	if( !pArr ) {
		TRACE( "Leave: Alloc failed" );
		return NULL;
	}

	for( i = 0; i < cDimensions; ++i ) {
		pArr->uDimensions[ i ] = pDimensions[ i ];
	}
	for( i = cDimensions; i < TENSHI_ARRAY_MAX_DIMENSIONS; ++i ) {
		pArr->uDimensions[ i ] = 0;
	}
	pArr->cDimensions = cDimensions;

	uOldBases[ 0 ] = 0;
	for( i = 1; i < pOldArray->cDimensions; ++i ) {
		uOldBases[ i ] = uOldBases[ i - 1 ] + pOldArray->uDimensions[ i ];
	}

	uNewBases[ 0 ] = 0;
	for( i = 1; i < cDimensions; ++i ) {
		uNewBases[ i ] = uNewBases[ i - 1 ] + pDimensions[ i ];
	}

	pArr->cItems = cItems;
	pArr->cItemBytes = pOldArray->cItemBytes;
	pArr->pItemType = pOldArray->pItemType;
	pArr->uIndex = 0;

	pOldBaseAddr = DataFromArray( pOldArray );
	pNewBaseAddr = DataFromArray( pArr );

	uOldBaseAddr = ( TenshiUIntPtr_t )pOldBaseAddr;
	uNewBaseAddr = ( TenshiUIntPtr_t )pNewBaseAddr;

	cMoveDims = pOldArray->cDimensions < cDimensions ? pOldArray->cDimensions : cDimensions;
	
	for( uDim = 0; uDim < cMoveDims; ++uDim ) {
		uDstDimAddr = uNewBaseAddr + uNewBases[ uDim ];
		uSrcDimAddr = uOldBaseAddr + uOldBases[ uDim ];

		cOldItems = pOldArray->uDimensions[ uDim ];
		cNewItems = pArr->uDimensions[ uDim ];

		cMoveItems = cOldItems < cNewItems ? cOldItems : cNewItems;
		cMakeItems = cOldItems < cNewItems ? cNewItems - cOldItems : 0;

		if( teTypeHasTrivialMove( pItemType ) ) {
			memcpy( ( void * )uDstDimAddr, ( const void * )uSrcDimAddr, cMoveItems*pArr->cItemBytes );
		} else {
			for( uItem = 0; uItem < cMoveItems; ++uItem ) {
				teMoveTypeInstance( pItemType, ( void * )uDstDimAddr, ( void * )uSrcDimAddr );
				uDstDimAddr += pArr->cItemBytes;
				uSrcDimAddr += pArr->cItemBytes;
			}
		}
		if( teTypeHasTrivialInit( pItemType ) ) {
			memset( ( void * )( uDstDimAddr + cMoveItems*pArr->cItemBytes ), 0, cMakeItems*pArr->cItemBytes );
		} else {
			for( uItem = 0; uItem < cMakeItems; ++uItem ) {
				teInitTypeInstance( pItemType, ( void * )uDstDimAddr );
				uDstDimAddr += pArr->cItemBytes;
			}
		}
	}
	
	teArrayUndim( pOldArrayData );

	while( uDim < cDimensions ) {
		uDstDimAddr = uNewBaseAddr + uNewBases[ uDim ];
		cMakeItems = pArr->uDimensions[ uDim ];

		if( teTypeHasTrivialInit( pItemType ) ) {
			memset( ( void * )uDstDimAddr, 0, cMakeItems*pArr->cItemBytes );
		} else {
			for( uItem = 0; uItem < cMakeItems; ++uItem ) {
				teInitTypeInstance( pItemType, ( void * )uDstDimAddr );
				uDstDimAddr += pArr->cItemBytes;
			}
		}

		++uDim;
	}

	TRACE( "Leave: Success; pNewBaseAddr=%p",  pNewBaseAddr );
	return pNewBaseAddr;
}

TENSHI_FUNC void *TENSHI_CALL teArrayInsertElements( void *pArrayData, TenshiUIntPtr_t uBefore, const void *pItems, TenshiUIntPtr_t cItems )
{
	TenshiArray_t *pArr;
	TenshiUIntPtr_t cNewItems;
	TenshiUIntPtr_t cNewBytes;
	TenshiArray_t *pNewArr;
	TenshiUIntPtr_t uDim;
	void *pNewArrData;
	TenshiUIntPtr_t uDstData;
	TenshiUIntPtr_t uSrcData;
	TenshiUIntPtr_t uTop;
	TenshiUIntPtr_t i;

	if( !pArrayData || !cItems ) {
		return pArrayData;
	}

	pArr = ArrayFromData( pArrayData );
	if( pArr->cDimensions != 1 ) {
		return NULL;
	}

	cNewItems = pArr->cItems + cItems;
	cNewBytes = pArr->cItemBytes*cNewItems;

	pNewArr = ( TenshiArray_t * )teAlloc( sizeof( TenshiArray_t ) + cNewBytes, CURRENT_MEMTAG );
	if( !pNewArr ) {
		return NULL;
	}

	for( uDim = 0; uDim < TENSHI_ARRAY_MAX_DIMENSIONS; ++uDim ) {
		pNewArr->uDimensions[ uDim ] = pArr->uDimensions[ uDim ];
	}
	pNewArr->cDimensions = pArr->cDimensions;
	pNewArr->cItems = cNewItems;
	pNewArr->cItemBytes = pArr->cItemBytes;
	pNewArr->pItemType = pArr->pItemType;

	pNewArrData = DataFromArray( pNewArr );
	uDstData = ( TenshiUIntPtr_t )pNewArrData;
	uSrcData = ( TenshiUIntPtr_t )pArrayData;

	uTop = uBefore <= pArr->cItems ? uBefore : pArr->cItems;
	pNewArr->uIndex = uTop;

	if( teTypeHasTrivialMove( pArr->pItemType ) ) {
		memcpy( ( void * )uDstData, ( const void * )uSrcData, uTop*pArr->cItemBytes );
		uDstData += uTop*pArr->cItemBytes;

		if( !pItems ) {
			if( teTypeHasTrivialInit( pArr->pItemType ) ) {
				memset( ( void * )uDstData, 0, cItems*pArr->cItemBytes );
			} else {
				for( i = 0; i < uTop; ++i ) {
					teInitTypeInstance( pArr->pItemType, ( void * )uDstData );
					uDstData += pArr->cItemBytes;
				}
			}
		} else {
			memcpy( ( void * )uDstData, pItems, cItems*pArr->cItemBytes );
		}
	} else {
		for( i = 0; i < uTop; ++i ) {
			teMoveTypeInstance( pArr->pItemType, ( void * )uDstData, ( void * )uSrcData );
			teFiniTypeInstance( pArr->pItemType, ( void * )uSrcData );

			uDstData += pArr->cItemBytes;
			uSrcData += pArr->cItemBytes;
		}

		if( !pItems ) {
			for( i = 0; i < cItems; ++i ) {
				teInitTypeInstance( pArr->pItemType, ( void * )uDstData );
				uDstData += pArr->cItemBytes;
			}
		} else {
			uSrcData = ( TenshiUIntPtr_t )pItems;
			for( i = 0; i < cItems; ++i ) {
				teCopyTypeInstance( pArr->pItemType, ( void * )uDstData, ( const void * )uSrcData );
				uDstData += pArr->cItemBytes;
				uSrcData += pArr->cItemBytes;
			}
		}
	}

	/* why is this here? */
	uSrcData = ( TenshiUIntPtr_t )pArrayData + uTop*pArr->cItemBytes;
	for( i = uTop; i < pArr->cItems; ++i ) {
		teMoveTypeInstance( pArr->pItemType, ( void * )uDstData, ( void * )uSrcData );
		teFiniTypeInstance( pArr->pItemType, ( void * )uSrcData );

		uDstData += pArr->cItemBytes;
		uSrcData += pArr->cItemBytes;
	}

	teDealloc( ( void * )pArr );
	return pNewArrData;
}
TENSHI_FUNC void TENSHI_CALL teArrayDeleteElements( void *pArrayData, TenshiUIntPtr_t uFirst, TenshiUIntPtr_t cDeletes )
{
	TenshiArray_t *pArr;
	TenshiUIntPtr_t uBaseAddr;
	TenshiUIntPtr_t uTop;
	TenshiUIntPtr_t uRemainder;
	TenshiUIntPtr_t cRemoved;
	TenshiUIntPtr_t uItemAddr;
	TenshiUIntPtr_t uDstAddr;
	TenshiUIntPtr_t uSrcAddr;
	TenshiUIntPtr_t i;

	if( !pArrayData ) {
		return;
	}

	pArr = ArrayFromData( pArrayData );
	if( uFirst >= pArr->cItems || !cDeletes ) {
		return;
	}

	uBaseAddr = ( TenshiUIntPtr_t )pArrayData;

	uTop = uFirst + cDeletes > pArr->cItems ? pArr->cItems : uFirst + cDeletes;
	if( !teTypeHasTrivialFini( pArr->pItemType ) ) {
		uItemAddr = uBaseAddr + uTop*pArr->cItemBytes;
		for( i = uTop; i > uFirst; --i ) {
			uItemAddr -= pArr->cItemBytes;
			teFiniTypeInstance( pArr->pItemType, ( void * )uItemAddr );
		}
	}

	uRemainder = pArr->cItems - uTop;
	uDstAddr = uBaseAddr + uFirst*pArr->cItemBytes;
	uSrcAddr = uBaseAddr + uTop*pArr->cItemBytes;
	for( i = 0; i < uRemainder; ++i ) {
		teMoveTypeInstance( pArr->pItemType, ( void * )uDstAddr, ( void * )uSrcAddr );
		teFiniTypeInstance( pArr->pItemType, ( void * )uSrcAddr );

		uDstAddr += pArr->cItemBytes;
		uSrcAddr += pArr->cItemBytes;
	}

	cRemoved = uTop - uFirst;
	pArr->cItems -= cRemoved;
}

TENSHI_FUNC void TENSHI_CALL teArrayIndexToBottom( void *pArrayData )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return;
	}

	pArr = ArrayFromData( pArrayData );
	pArr->uIndex = pArr->cItems > 0 ? pArr->cItems - 1 : 0;
}
TENSHI_FUNC void TENSHI_CALL teArrayIndexToTop( void *pArrayData )
{
	if( !pArrayData ) {
		return;
	}

	ArrayFromData( pArrayData )->uIndex = 0;
}
TENSHI_FUNC void TENSHI_CALL teArrayIndexToNext( void *pArrayData )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return;
	}

	pArr = ArrayFromData( pArrayData );
	if( pArr->uIndex + 1 < pArr->cItems ) {
		++pArr->uIndex;
	}
}
TENSHI_FUNC void TENSHI_CALL teArrayIndexToPrevious( void *pArrayData )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return;
	}

	pArr = ArrayFromData( pArrayData );
	if( pArr->uIndex > 0 ) {
		--pArr->uIndex;
	}
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teArrayIsIndexValid( const void *pArrayData )
{
	const TenshiArray_t *pArr;

	if( !pArrayData ) {
		return TENSHI_FALSE;
	}

	pArr = ArrayFromConstData( pArrayData );
	return pArr->uIndex < pArr->cItems ? TENSHI_TRUE : TENSHI_FALSE;
}

TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtTop( void *pArrayData )
{
	return teArrayInsertElements( pArrayData, 0, NULL, 1 );
}
TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtBottom( void *pArrayData )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return NULL;
	}

	pArr = ArrayFromData( pArrayData );
	return teArrayInsertElements( pArrayData, pArr->cItems, NULL, 1 );
}
TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtElement( void *pArrayData, TenshiUIntPtr_t uElement )
{
	return teArrayInsertElements( pArrayData, uElement, NULL, 1 );
}

TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtTop( void *pArrayData, TenshiUIntPtr_t cItems )
{
	return teArrayInsertElements( pArrayData, 0, NULL, cItems );
}
TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtBottom( void *pArrayData, TenshiUIntPtr_t cItems )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return NULL;
	}

	pArr = ArrayFromData( pArrayData );
	return teArrayInsertElements( pArrayData, pArr->cItems, NULL, cItems );
}
TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtElement( void *pArrayData, TenshiUIntPtr_t uElement, TenshiUIntPtr_t cItems )
{
	return teArrayInsertElements( pArrayData, uElement, NULL, cItems );
}

TENSHI_FUNC void TENSHI_CALL teArrayDeleteElementAt( void *pArrayData, TenshiUIntPtr_t uIndex )
{
	teArrayDeleteElements( pArrayData, uIndex, 1 );
}
TENSHI_FUNC void TENSHI_CALL teArrayDeleteElement( void *pArrayData )
{
	if( !pArrayData ) {
		return;
	}

	teArrayDeleteElements( pArrayData, ArrayFromConstData( pArrayData )->uIndex, 1 );
}

TENSHI_FUNC void TENSHI_CALL teEmptyArray( void *pArrayData )
{
	if( !pArrayData ) {
		return;
	}

	teArrayDeleteElements( pArrayData, 0, ArrayFromConstData( pArrayData )->cItems );
}

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayLen( const void *pArrayData )
{
	return pArrayData != NULL ? ArrayFromConstData( pArrayData )->cItems : 0;
}
TENSHI_FUNC TenshiIntPtr_t TENSHI_CALL teArrayCount( const void *pArrayData )
{
	return ( ( TenshiIntPtr_t )teArrayLen( pArrayData ) ) - 1;
}

TENSHI_FUNC void TENSHI_CALL teArrayIndexToStack( void *pArrayData )
{
	teArrayIndexToBottom( pArrayData );
}
TENSHI_FUNC void *TENSHI_CALL teAddToStack( void *pArrayData )
{
	void *pNewArrayData;

	pNewArrayData = teArrayInsertAtBottom( pArrayData );
	if( !pNewArrayData ) {
		return NULL;
	}

	teArrayIndexToBottom( pNewArrayData );
	return pNewArrayData;
}
TENSHI_FUNC void TENSHI_CALL teRemoveFromStack( void *pArrayData )
{
	TenshiArray_t *pArr;

	if( !pArrayData ) {
		return;
	}

	pArr = ArrayFromData( pArrayData );

	if( pArr->cItems == 0 ) {
		return;
	}

	teArrayDeleteElements( pArrayData, pArr->cItems - 1, 1 );
	teArrayIndexToBottom( pArrayData );
}

TENSHI_FUNC void TENSHI_CALL teArrayIndexToQueue( void *pArrayData )
{
	teArrayIndexToTop( pArrayData );
}
TENSHI_FUNC void *TENSHI_CALL teAddToQueue( void *pArrayData )
{
	void *pNewArrayData;

	pNewArrayData = teArrayInsertAtTop( pArrayData );
	if( !pNewArrayData ) {
		return NULL;
	}

	teArrayIndexToBottom( pNewArrayData );
	return pNewArrayData;
}
TENSHI_FUNC void TENSHI_CALL teRemoveFromQueue( void *pArrayData )
{
	teArrayDeleteElements( pArrayData, 0, 1 );
	teArrayIndexToTop( pArrayData );
}

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayCurrentIndex( const void *pArrayData )
{
	const TenshiArray_t *pArr;

	if( !pArrayData ) {
		return 0;
	}

	pArr = ArrayFromConstData( pArrayData );
	return pArr->uIndex;
}
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayDimensionLen( const void *pArrayData, TenshiUIntPtr_t uDim )
{
	const TenshiArray_t *pArr;

	if( !pArrayData ) {
		return 0;
	}

	pArr = ArrayFromConstData( pArrayData );
	return uDim < pArr->cDimensions ? pArr->uDimensions[ uDim ] : 0;
}


/*
===============================================================================

	LINKED LIST

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_List

static void List_FixCachedIndex( TenshiList_t *pList, TenshiListItem_t *pItem )
{
	if( pItem != pList->pCachedIndex ) {
		pList->pCachedIndex = NULL;
		pList->uCachedIndex = 0;
		return;
	}

	if( pItem->pNext != NULL ) {
		pList->pCachedIndex = pList->pCachedIndex->pNext;
	} else if( pItem->pPrev != NULL ) {
		pList->pCachedIndex = pList->pCachedIndex->pPrev;
		--pList->uCachedIndex;
	} else {
		pList->pCachedIndex = NULL;
		pList->uCachedIndex = 0;
	}
}
static void List_Unlink( TenshiList_t *pList, TenshiListItem_t *pItem )
{
	List_FixCachedIndex( pList, pItem );

	if( pItem->pPrev != NULL ) {
		pItem->pPrev->pNext = pItem->pNext;
	} else {
		pList->pHead = pItem->pNext;
	}

	if( pItem->pNext != NULL ) {
		pItem->pNext->pPrev = pItem->pPrev;
	} else {
		pList->pTail = pItem->pPrev;
	}

	pItem->pPrev = NULL;
	pItem->pNext = NULL;

	if( pList->pCurr == pItem ) {
		pList->pCurr = NULL;
	}

	if( pList->pCachedIndex == pItem ) {
		if( pList->pCachedIndex->pNext == NULL ) {
			pList->pCachedIndex = pList->pCachedIndex->pPrev;
			if( pList->pCachedIndex != NULL ) {
				--pList->uCachedIndex;
			}
		}
	} else {
		pList->uCachedIndex = 0;
		pList->pCachedIndex = NULL;
	}
}

TENSHI_FUNC TenshiList_t *TENSHI_CALL teNewList( TenshiType_t *pItemType )
{
	TenshiList_t *pList;

	if( !pItemType ) {
		return NULL;
	}

	pList = ( TenshiList_t * )teAlloc( sizeof( TenshiList_t ), g_RTGlob.iCurrentMemtag );
	if( !pList ) {
		return NULL;
	}

	pList->pHead = NULL;
	pList->pTail = NULL;
	pList->pCurr = NULL;

	pList->cItems = 0;
	pList->cItemBytes = pItemType->cBytes;
	pList->pItemType = pItemType;

	pList->uCachedIndex = 0;
	pList->pCachedIndex = NULL;

	return pList;
}
TENSHI_FUNC TenshiList_t *TENSHI_CALL teDeleteList( TenshiList_t *pList )
{
	if( !pList ) {
		return NULL;
	}

	teEmptyList( pList );
	teDealloc( ( void * )pList );

	return NULL;
}

TENSHI_FUNC void *TENSHI_CALL teListNodeItem( TenshiListItem_t *pItem )
{
	if( !pItem ) {
		return NULL;
	}

	return ( void * )( pItem + 1 );
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListNodeFromItem( void *pData )
{
	if( !pData ) {
		return NULL;
	}

	return ( ( TenshiListItem_t * )pData ) - 1;
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListFrontNode( TenshiList_t *pList )
{
	return pList != NULL ? pList->pHead : NULL;
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListBackNode( TenshiList_t *pList )
{
	return pList != NULL ? pList->pTail : NULL;
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL tePreviousListNode( TenshiListItem_t *pNode )
{
	return pNode != NULL ? pNode->pPrev : NULL;
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teNextListNode( TenshiListItem_t *pNode )
{
	return pNode != NULL ? pNode->pNext : NULL;
}

TENSHI_FUNC void *TENSHI_CALL teListFront( TenshiList_t *pList )
{
	return teListNodeItem( pList != NULL ? pList->pHead : NULL );
}
TENSHI_FUNC void *TENSHI_CALL teListBack( TenshiList_t *pList )
{
	return teListNodeItem( pList != NULL ? pList->pTail : NULL );
}
TENSHI_FUNC void *TENSHI_CALL teListPrevious( void *pData )
{
	TenshiListItem_t *pItem;

	if( !pData ) {
		return NULL;
	}

	pItem = ( ( TenshiListItem_t * )pData ) - 1;
	return teListNodeItem( pItem->pPrev );
}
TENSHI_FUNC void *TENSHI_CALL teListNext( void *pData )
{
	TenshiListItem_t *pItem;

	if( !pData ) {
		return NULL;
	}

	pItem = ( ( TenshiListItem_t * )pData ) - 1;
	return teListNodeItem( pItem->pNext );
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertBeforeNode( TenshiList_t *pList, TenshiListItem_t *pAfterNode )
{
	TenshiListItem_t *pBeforeNode;

	if( !pList ) {
		return NULL;
	}

	pBeforeNode = ( TenshiListItem_t * )teAlloc( sizeof( TenshiListItem_t ) + pList->cItemBytes, g_RTGlob.iCurrentMemtag );
	if( !pBeforeNode || !teInitTypeInstance( pList->pItemType, teListNodeItem( pBeforeNode ) ) ) {
		return NULL;
	}

	if( !pAfterNode ) {
		pBeforeNode->pPrev = pList->pTail;
		pBeforeNode->pNext = NULL;
	} else {
		pBeforeNode->pPrev = pAfterNode->pPrev;
		pBeforeNode->pNext = pAfterNode;
	}

	if( pBeforeNode->pPrev != NULL ) {
		pBeforeNode->pPrev->pNext = pBeforeNode;
	} else {
		pList->pHead = pBeforeNode;
	}
	if( pBeforeNode->pNext != NULL ) {
		pBeforeNode->pNext->pPrev = pBeforeNode;
	} else {
		pList->pTail = pBeforeNode;
	}

	++pList->cItems;

	pList->uCachedIndex = 0;
	pList->pCachedIndex = NULL;

	return pBeforeNode;
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertAfterNode( TenshiList_t *pList, TenshiListItem_t *pBeforeNode )
{
	return teListInsertBeforeNode( pList, pBeforeNode != NULL ? pBeforeNode->pNext : NULL );
}

TENSHI_FUNC void *TENSHI_CALL teListInsertBefore( TenshiList_t *pList, void *pAfter )
{
	return teListNodeItem( teListInsertBeforeNode( pList, teListNodeFromItem( pAfter ) ) );
}
TENSHI_FUNC void *TENSHI_CALL teListInsertAfter( TenshiList_t *pList, void *pBefore )
{
	return teListNodeItem( teListInsertAfterNode( pList, teListNodeFromItem( pBefore ) ) );
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertNode( TenshiList_t *pList )
{
	return teListInsertBeforeNode( pList, pList != NULL ? pList->pCurr : NULL );
}
TENSHI_FUNC void *TENSHI_CALL teListInsert( TenshiList_t *pList )
{
	return teListNodeItem( teListInsertNode( pList ) );
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListAddNodeToFront( TenshiList_t *pList )
{
	if( !pList ) {
		return NULL;
	}

	return teListInsertBeforeNode( pList, pList->pHead );
}
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListAddNodeToBack( TenshiList_t *pList )
{
	if( !pList ) {
		return NULL;
	}

	return teListInsertBeforeNode( pList, NULL );
}
TENSHI_FUNC void *TENSHI_CALL teListAddToFront( TenshiList_t *pList )
{
	return teListNodeItem( teListAddNodeToFront( pList ) );
}
TENSHI_FUNC void *TENSHI_CALL teListAddToBack( TenshiList_t *pList )
{
	return teListNodeItem( teListAddNodeToBack( pList ) );
}

TENSHI_FUNC void TENSHI_CALL teListDeleteNode( TenshiList_t *pList, TenshiListItem_t *pItem )
{
	if( !pList || !pItem ) {
		return;
	}

	List_Unlink( pList, pItem );
	--pList->cItems;

	teFiniTypeInstance( pList->pItemType, teListNodeItem( pItem ) );
	teDealloc( ( void * )pItem );
}

TENSHI_FUNC void TENSHI_CALL teListDeleteFront( TenshiList_t *pList )
{
	if( !pList || !pList->pHead ) {
		return;
	}

	teListDeleteNode( pList, pList->pHead );
}
TENSHI_FUNC void TENSHI_CALL teListDeleteBack( TenshiList_t *pList )
{
	if( !pList || !pList->pTail ) {
		return;
	}

	teListDeleteNode( pList, pList->pTail );
}
TENSHI_FUNC void TENSHI_CALL teListDelete( TenshiList_t *pList )
{
	if( !pList || !pList->pCurr ) {
		return;
	}

	teListDeleteNode( pList, pList->pCurr );
}

TENSHI_FUNC void TENSHI_CALL teListToFront( TenshiList_t *pList )
{
	if( !pList ) {
		return;
	}

	pList->pCurr = pList->pHead;
}
TENSHI_FUNC void TENSHI_CALL teListToBack( TenshiList_t *pList )
{
	if( !pList ) {
		return;
	}

	pList->pCurr = pList->pTail;
}
TENSHI_FUNC void TENSHI_CALL teListToPrevious( TenshiList_t *pList )
{
	if( !pList || !pList->pCurr ) {
		return;
	}

	pList->pCurr = pList->pCurr->pPrev;
}
TENSHI_FUNC void TENSHI_CALL teListToNext( TenshiList_t *pList )
{
	if( !pList || !pList->pCurr ) {
		return;
	}

	pList->pCurr = pList->pCurr->pNext;
}

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teListIsValid( const TenshiList_t *pList )
{
	if( !pList ) {
		return TENSHI_FALSE;
	}

	return pList->pCurr != NULL ? TENSHI_TRUE : TENSHI_FALSE;
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListCurrentNode( TenshiList_t *pList )
{
	return pList != NULL ? pList->pCurr : NULL;
}
TENSHI_FUNC void *TENSHI_CALL teListCurrent( TenshiList_t *pList )
{
	return teListNodeItem( teListCurrentNode( pList ) );
}
TENSHI_FUNC void TENSHI_CALL teListResetCurrent( TenshiList_t *pList )
{
	if( !pList ) {
		return;
	}

	pList->pCurr = NULL;
}

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teListIsEmpty( const TenshiList_t *pList )
{
	if( !pList ) {
		return TENSHI_TRUE;
	}

	return pList->pHead == NULL ? TENSHI_TRUE : TENSHI_FALSE;
}
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teListLen( const TenshiList_t *pList )
{
	if( !pList ) {
		return 0;
	}

	return pList->cItems;
}
TENSHI_FUNC void TENSHI_CALL teEmptyList( TenshiList_t *pList )
{
	if( !pList ) {
		return;
	}

	while( !teListIsEmpty( pList ) ) {
		teListDeleteNode( pList, pList->pTail );
	}
}

TENSHI_FUNC void TENSHI_CALL teListMoveToFront( TenshiList_t *pList )
{
	TenshiListItem_t *pCurr;

	if( !pList || !pList->pCurr ) {
		return;
	}

	pCurr = pList->pCurr;
	List_Unlink( pList, pCurr );

	if( pList->pHead != NULL ) {
		pCurr->pNext = pList->pHead;
		pList->pHead->pPrev = pCurr;
	}
	pList->pHead = pCurr;
}
TENSHI_FUNC void TENSHI_CALL teListMoveToBack( TenshiList_t *pList )
{
	TenshiListItem_t *pCurr;

	if( !pList || !pList->pCurr ) {
		return;
	}

	pCurr = pList->pCurr;
	List_Unlink( pList, pCurr );

	if( pList->pTail != NULL ) {
		pCurr->pPrev = pList->pTail;
		pList->pTail->pNext = pCurr;
	}
	pList->pTail = pCurr;
}
TENSHI_FUNC void TENSHI_CALL teListMoveToPrevious( TenshiList_t *pList )
{
	TenshiListItem_t *pNode;
	TenshiListItem_t *pBefore;

	if( !pList || !pList->pCurr || !pList->pCurr->pPrev ) {
		return;
	}

	pNode = pList->pCurr;
	pBefore = pList->pCurr->pPrev;
	List_Unlink( pList, pNode );

	pNode->pNext = pBefore;
	pNode->pPrev = pBefore->pPrev;
	if( pBefore->pPrev != NULL ) {
		pBefore->pPrev->pNext = pNode;
	} else {
		pList->pHead = pNode;
	}
	pBefore->pPrev = pNode;
	pList->pCurr = pNode;
}
TENSHI_FUNC void TENSHI_CALL teListMoveToNext( TenshiList_t *pList )
{
	TenshiListItem_t *pNode;
	TenshiListItem_t *pAfter;

	if( !pList || !pList->pCurr || !pList->pCurr->pNext ) {
		return;
	}

	pNode = pList->pCurr;
	pAfter = pList->pCurr->pNext;
	List_Unlink( pList, pNode );

	pNode->pPrev = pAfter;
	pNode->pNext = pAfter->pNext;
	if( pAfter->pNext != NULL ) {
		pAfter->pNext->pPrev = pNode;
	} else {
		pList->pTail = pNode;
	}
	pAfter->pNext = pNode;
	pList->pCurr = pNode;
}

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListNodeAt( TenshiList_t *pList, TenshiUIntPtr_t uIndex )
{
	if( !pList || uIndex >= pList->cItems ) {
		return NULL;
	}

	if( !pList->pCachedIndex ) {
		if( uIndex > pList->cItems/2 ) {
			pList->pCachedIndex = pList->pTail;
			pList->uCachedIndex = pList->cItems - 1;
		} else {
			pList->pCachedIndex = pList->pHead;
			pList->uCachedIndex = 0;
		}
	}

	while( pList->uCachedIndex < uIndex ) {
		pList->pCachedIndex = pList->pCachedIndex->pNext;
		++pList->uCachedIndex;
	}

	while( pList->uCachedIndex > uIndex ) {
		pList->pCachedIndex = pList->pCachedIndex->pPrev;
		--pList->uCachedIndex;
	}

	return pList->pCachedIndex;
}
TENSHI_FUNC void *TENSHI_CALL teListAt( TenshiList_t *pList, TenshiUIntPtr_t uIndex )
{
	return teListNodeItem( teListNodeAt( pList, uIndex ) );
}


/*
===============================================================================

	BINARY TREE

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT_BTree

TENSHI_FUNC TenshiBTree_t *TENSHI_CALL teNewBTree( TenshiType_t *pItemType )
{
	TenshiBTree_t *pBase;

	if( !pItemType ) {
		return NULL;
	}

	pBase = ( TenshiBTree_t * )teAlloc( sizeof( TenshiBTree_t ), g_RTGlob.iCurrentMemtag );
	if( !pBase ) {
		return NULL;
	}

	pBase->pRoot = NULL;
	pBase->pHead = NULL;
	pBase->pTail = NULL;

	pBase->pItemType = pItemType;
	pBase->cItemBytes = pItemType->cBytes;

	return pBase;
}
TENSHI_FUNC TenshiBTree_t *TENSHI_CALL teDeleteBTree( TenshiBTree_t *pBase )
{
	if( !pBase ) {
		return NULL;
	}

	teEmptyBTree( pBase );
	teDealloc( ( void * )pBase );

	return NULL;
}

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teBTreeIsEmpty( const TenshiBTree_t *pBase )
{
	return pBase == NULL || pBase->pHead == NULL ? TENSHI_TRUE : TENSHI_FALSE;
}
TENSHI_FUNC void TENSHI_CALL teEmptyBTree( TenshiBTree_t *pBase )
{
	TenshiBTreeNode_t *pNode;
	TenshiBTreeNode_t *pNext;

	if( !pBase ) {
		return;
	}

	for( pNode = pBase->pHead; pNode != NULL; pNode = pNext ) {
		pNext = pNode->pNext;

		pNode->pPrev = NULL;
		pNode->pNext = NULL;
		pNode->pPrnt = NULL;
		pNode->pLeft = NULL;
		pNode->pRght = NULL;
		pNode->pBase = NULL;

		teFiniTypeInstance( pBase->pItemType, ( void * )( pNode + 1 ) );
		teDealloc( ( void * )pNode );
	}

	pBase->pRoot = NULL;
	pBase->pHead = NULL;
	pBase->pTail = NULL;
}

static void BTree_LinkNode( TenshiBTree_t *pBase, TenshiBTreeNode_t *pNode, TenshiInt32_t iKey )
{
	pNode->pBase = pBase;

	pNode->Key = iKey;

	pNode->pPrnt = NULL;
	pNode->pLeft = NULL;
	pNode->pRght = NULL;

	pNode->pNext = NULL;
	pNode->pPrev = pBase->pTail;
	if( pBase->pTail != NULL ) {
		pBase->pTail->pNext = pNode;
	} else {
		pBase->pHead = pNode;
	}
	pBase->pTail = pNode;
}
static TenshiBTreeNode_t *BTree_Find( TenshiBTree_t *pBase, TenshiInt32_t iKey, TenshiBTreeNode_t *pCreate )
{
	TenshiBTreeNode_t *pNode;
	TenshiBTreeNode_t *pNext;
	TenshiBTreeNode_t *pTop;
	TenshiBTreeNode_t **ppSpot;

	if( !pBase->pRoot ) {
		if( pCreate != NULL ) {
			BTree_LinkNode( pBase, pCreate, iKey );

			pBase->pRoot = pCreate;
		}

		return pCreate;
	}

	pTop = NULL;
	for( pNode = pBase->pRoot; pNode != NULL; pNode = pNext ) {
		if( pNode->Key == iKey ) {
			return pNode;
		}

		ppSpot = ( iKey < pNode->Key ) ? &pNode->pLeft : &pNode->pRght;
		pTop = pNode;

		pNext = *ppSpot;
		if( pNext != NULL ) {
			continue;
		}

		if( pCreate != NULL ) {
			BTree_LinkNode( pBase, pCreate, iKey );

			pCreate->pPrnt = pTop;
			*ppSpot = pCreate;
		}

		return pCreate;
	}

	return NULL;
}

typedef enum BTreeFindCreateMode_e
{
	kBTFCMode_FindOnly,
	kBTFCMode_CreateOnly,
	kBTFCMode_FindOrCreate
} BTreeFindCreateMode_t;
static TenshiBTreeNode_t *BTree_FindCreate( TenshiBTree_t *pBase, TenshiInt32_t iKey, BTreeFindCreateMode_t Mode )
{
	static TenshiBTreeNode_t CreateNode;
	TenshiBTreeNode_t *pNode;

	if( !pBase ) {
		return NULL;
	}

	if( Mode == kBTFCMode_FindOnly ) {
		return BTree_Find( pBase, iKey, NULL );
	}

	pNode = BTree_Find( pBase, iKey, &CreateNode );
	if( pNode != &CreateNode ) {
		return Mode != kBTFCMode_CreateOnly ? pNode : NULL;
	}

	pNode = ( TenshiBTreeNode_t * )teAlloc( sizeof( TenshiBTreeNode_t ) + pBase->cItemBytes, CURRENT_MEMTAG );
	if( !pNode || !teInitTypeInstance( pBase->pItemType, ( void * )( pNode + 1 ) ) ) {
		if( CreateNode.pPrnt != NULL ) {
			if( CreateNode.pPrnt->pLeft == &CreateNode ) {
				CreateNode.pPrnt->pLeft = NULL;
			} else {
				CreateNode.pPrnt->pRght = NULL;
			}
		} else {
			pBase->pRoot = NULL;
		}

		if( CreateNode.pPrev != NULL ) {
			CreateNode.pPrev->pNext = CreateNode.pNext;
		} else {
			pBase->pHead = CreateNode.pNext;
		}
		if( CreateNode.pNext != NULL ) {
			CreateNode.pNext->pPrev = CreateNode.pPrev;
		} else {
			pBase->pTail = CreateNode.pPrev;
		}

		teDealloc( ( void * )pNode );
		return NULL;
	}

	return pNode;
}

TENSHI_FUNC void *TENSHI_CALL teBTreeItemFromNode( TenshiBTreeNode_t *pNode )
{
	return pNode != NULL ? ( void * )( pNode + 1 ) : NULL;
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeNodeFromItem( void *pItem )
{
	return pItem != NULL ? ( ( TenshiBTreeNode_t * )pItem ) - 1 : NULL;
}

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeFindNode( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return BTree_Find( pBase, iKey, NULL );
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeLookupNode( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return BTree_FindCreate( pBase, iKey, kBTFCMode_FindOrCreate );
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeInsertNode( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return BTree_FindCreate( pBase, iKey, kBTFCMode_CreateOnly );
}

TENSHI_FUNC void *TENSHI_CALL teBTreeFind( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return teBTreeItemFromNode( teBTreeFindNode( pBase, iKey ) );
}
TENSHI_FUNC void *TENSHI_CALL teBTreeLookup( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return teBTreeItemFromNode( teBTreeLookupNode( pBase, iKey ) );
}
TENSHI_FUNC void *TENSHI_CALL teBTreeInsert( TenshiBTree_t *pBase, TenshiInt32_t iKey )
{
	return teBTreeItemFromNode( teBTreeInsertNode( pBase, iKey ) );
}

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeDeleteNode( TenshiBTree_t *pBase, TenshiBTreeNode_t *pNode )
{
	TenshiBTreeNode_t *r, *s;

	if( !pNode || !pNode->pBase || ( pNode->pBase != pBase && pBase != NULL ) ) {
		return NULL;
	}

	pBase = pNode->pBase;

	r = pNode->pRght;
	if( !r ) {
		if( pNode->pPrnt != NULL ) {
			if( pNode->pPrnt->pLeft == pNode ) {
				pNode->pPrnt->pLeft = pNode->pLeft;
			} else {
				pNode->pPrnt->pRght = pNode->pLeft;
			}
		} else {
			pBase->pRoot = pNode->pLeft;
		}

		if( pNode->pLeft != NULL ) {
			pNode->pLeft->pPrnt = pNode->pPrnt;
		}
	} else if( !r->pLeft ) {
		r->pPrnt = pNode->pPrnt;

		r->pLeft = pNode->pLeft;
		if( r->pLeft != NULL ) {
			pNode->pLeft->pPrnt = r;
		}

		if( pNode->pPrnt != NULL ) {
			pNode->pPrnt->pLeft = r;
		} else {
			pBase->pRoot = r;
		}
	} else {
		s = r->pLeft;

		while( s->pLeft != NULL ) {
			s = s->pLeft;
		}
		s->pLeft = pNode->pLeft;

		s->pPrnt->pLeft = s->pRght;
		if( s->pPrnt->pLeft != NULL ) {
			s->pRght->pPrnt = s->pPrnt;
		}
		s->pRght = r;

		s->pPrnt = pNode->pPrnt;
		if( s->pPrnt != NULL ) {
			if( pNode->pPrnt->pLeft == pNode ) {
				pNode->pPrnt->pLeft = s;
			} else {
				pNode->pPrnt->pRght = s;
			}
		} else {
			pBase->pRoot = s;
		}
	}

	if( pNode->pPrev != NULL ) {
		pNode->pPrev->pNext = pNode->pNext;
	} else {
		pNode->pBase->pHead = pNode->pNext;
	}

	if( pNode->pNext != NULL ) {
		pNode->pNext->pPrev = pNode->pPrev;
	} else {
		pNode->pBase->pTail = pNode->pPrev;
	}

	teFiniTypeInstance( pBase->pItemType, ( void * )( pNode + 1 ) );
	teDealloc( ( void * )pNode );

	return NULL;
}
TENSHI_FUNC void *TENSHI_CALL teBTreeDelete( TenshiBTree_t *pBase, void *pItem )
{
	return teBTreeDeleteNode( pBase, teBTreeNodeFromItem( pItem ) );
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teBTreeNodeKey( const TenshiBTreeNode_t *pNode )
{
	return pNode != NULL ? pNode->Key : 0;
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teBTreeKey( const void *pItem )
{
	return teBTreeNodeKey( pItem != NULL ? ( ( const TenshiBTreeNode_t * )pItem ) - 1 : NULL );
}

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeFrontNode( TenshiBTree_t *pBase )
{
	return pBase != NULL ? pBase->pHead : NULL;
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeBackNode( TenshiBTree_t *pBase )
{
	return pBase != NULL ? pBase->pTail : NULL;
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreePreviousNode( TenshiBTreeNode_t *pNode )
{
	return pNode != NULL ? pNode->pPrev : NULL;
}
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeNextNode( TenshiBTreeNode_t *pNode )
{
	return pNode != NULL ? pNode->pNext : NULL;
}

TENSHI_FUNC void *TENSHI_CALL teBTreeFront( TenshiBTree_t *pBase )
{
	return teBTreeItemFromNode( teBTreeFrontNode( pBase ) );
}
TENSHI_FUNC void *TENSHI_CALL teBTreeBack( TenshiBTree_t *pBase )
{
	return teBTreeItemFromNode( teBTreeBackNode( pBase ) );
}
TENSHI_FUNC void *TENSHI_CALL teBTreePrevious( void *pItem )
{
	return teBTreeItemFromNode( teBTreePreviousNode( teBTreeNodeFromItem( pItem ) ) );
}
TENSHI_FUNC void *TENSHI_CALL teBTreeNext( void *pItem )
{
	return teBTreeItemFromNode( teBTreeNextNode( teBTreeNodeFromItem( pItem ) ) );
}


/*
===============================================================================

	MEMBLOCKS

===============================================================================
*/

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_MemblockAPI

TENSHI_FUNC void *TENSHI_CALL teMemblockAlloc_f( void *pParm )
{
	TenshiUIntPtr_t cBytes;
	void *p;

	cBytes = ( TenshiUIntPtr_t )pParm;
	if( !cBytes ) {
		return ( void * )0;
	}

	p = teAlloc( cBytes + sizeof( TenshiMemblock_t ), TENSHI_MEMTAG_MEMBLOCK );
	if( p != ( void * )0 ) {
		memset( p, 0, cBytes + sizeof( TenshiMemblock_t ) );
	}

	( ( TenshiMemblock_t * )p )->cBytes = cBytes;

	return p;
}
TENSHI_FUNC void TENSHI_CALL teMemblockDealloc_f( void *p )
{
	teDealloc( p );
}

static TENSHI_FORCEINLINE TenshiMemblock_t *teMemblock( TenshiIndex_t uIndex )
{
	return ( TenshiMemblock_t * )teUnwrapEngineObject( g_MemblockAPI.pMemblockPool, uIndex );
}

TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocMemblock( TenshiUIntPtr_t cBytes )
{
	return teAllocEngineObject( g_MemblockAPI.pMemblockPool, 0, ( void * )cBytes );
}
TENSHI_FUNC void TENSHI_CALL teMakeMemblock( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t cBytes )
{
	teAllocEngineObject( g_MemblockAPI.pMemblockPool, MemblockNumber, ( void * )cBytes );
}
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teDeleteMemblock( TenshiIndex_t MemblockNumber )
{
	teDeallocEngineObject( g_MemblockAPI.pMemblockPool, MemblockNumber );
	return 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teMemblockExist( TenshiIndex_t MemblockNumber )
{
	return teEngineObjectExists( g_MemblockAPI.pMemblockPool, MemblockNumber );
}

TENSHI_FUNC void *TENSHI_CALL teGetMemblockPtr( TenshiIndex_t MemblockNumber )
{
	TenshiMemblock_t *p;

	p = teMemblock( MemblockNumber );
	if( !p ) {
		return ( void * )0;
	}

	return ( void * )( p + 1 );
}
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teGetMemblockSize( TenshiIndex_t MemblockNumber )
{
	TenshiMemblock_t *p;

	p = teMemblock( MemblockNumber );
	if( !p ) {
		return 0;
	}

	return p->cBytes;
}

#define READ_MEMBLOCK(T_)\
	const TenshiMemblock_t *p;\
	\
	p = teMemblock( MemblockNumber );\
	if( !p || uPos + sizeof( T_ ) > p->cBytes ) {\
		return 0;\
	}\
	\
	return *( const T_ * )( ( const TenshiUInt8_t * )( p + 1 ) + uPos )
TENSHI_FUNC TenshiUInt8_t TENSHI_CALL teMemblockByte( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(TenshiUInt8_t);
}
TENSHI_FUNC TenshiUInt16_t TENSHI_CALL teMemblockWord( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(TenshiUInt16_t);
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teMemblockDword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(TenshiUInt32_t);
}
TENSHI_FUNC TenshiUInt64_t TENSHI_CALL teMemblockQword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(TenshiUInt64_t);
}
TENSHI_FUNC float TENSHI_CALL teMemblockFloat( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(float);
}
TENSHI_FUNC double TENSHI_CALL teMemblockFloat64( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos )
{
	READ_MEMBLOCK(double);
}
#undef READ_MEMBLOCK

#define WRITE_MEMBLOCK(T_)\
	TenshiMemblock_t *p;\
	\
	p = teMemblock( MemblockNumber );\
	if( !p || uPos + sizeof( T_ ) > p->cBytes ) {\
		return;\
	}\
	\
	*( T_ * )( ( TenshiUInt8_t * )( p + 1 ) + uPos ) = Value
TENSHI_FUNC void TENSHI_CALL teWriteMemblockByte( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt8_t Value )
{
	WRITE_MEMBLOCK(TenshiUInt8_t);
}
TENSHI_FUNC void TENSHI_CALL teWriteMemblockWord( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt16_t Value )
{
	WRITE_MEMBLOCK(TenshiUInt16_t);
}
TENSHI_FUNC void TENSHI_CALL teWriteMemblockDword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt32_t Value )
{
	WRITE_MEMBLOCK(TenshiUInt32_t);
}
TENSHI_FUNC void TENSHI_CALL teWriteMemblockQword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt64_t Value )
{
	WRITE_MEMBLOCK(TenshiUInt64_t);
}
TENSHI_FUNC void TENSHI_CALL teWriteMemblockFloat( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, float Value )
{
	WRITE_MEMBLOCK(float);
}
TENSHI_FUNC void TENSHI_CALL teWriteMemblockFloat64( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, double Value )
{
	WRITE_MEMBLOCK(double);
}
#undef WRITE_MEMBLOCK

TENSHI_FUNC void TENSHI_CALL teCopyMemblock( TenshiIndex_t MemblockFrom, TenshiIndex_t MemblockTo, TenshiUIntPtr_t uPosFrom, TenshiUIntPtr_t uPosTo, TenshiUIntPtr_t cBytes )
{
	const TenshiMemblock_t *pMemFrom;
	const void *pFrom;
	TenshiMemblock_t *pMemTo;
	void *pTo;

	pMemFrom = teMemblock( MemblockFrom );
	pMemTo = teMemblock( MemblockTo );

	if( !pMemFrom || !pMemTo || !cBytes ) {
		return;
	}

	if( uPosFrom + cBytes > pMemFrom->cBytes || uPosTo + cBytes > pMemTo->cBytes ) {
		return;
	}

	pFrom = ( const void * )( ( const TenshiUInt8_t * )( pMemFrom + 1 ) + uPosFrom );
	pTo = ( void * )( ( const TenshiUInt8_t * )( pMemTo + 1 ) + uPosTo );

	if( pMemFrom != pMemTo ) {
		memcpy( pTo, pFrom, cBytes );
	} else {
		memmove( pTo, pFrom, cBytes );
	}
}


/*
===============================================================================

	BASIC MATH

===============================================================================
*/

#define TENSHI_PI   3.1415926535897932384626433832795028841971693993751058209
#define TENSHI_PI_F 3.1415926535897932384626433832795028841971693993751058209f

typedef union {
	TenshiInt32_t  i;
	TenshiUInt32_t u;
	float          f;
} TenshiFloatInt32_t;

TENSHI_FUNC float TENSHI_CALL teUintBitsToFloat( TenshiUInt32_t x )
{
	TenshiFloatInt32_t v;

	v.u = x;
	return v.f;
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teFloatToUintBits( float x )
{
	TenshiFloatInt32_t v;

	v.f = x;
	return v.u;
}

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsNAN( float x )
{
	TenshiUInt32_t v;

	v = teFloatToUintBits( x );
	return ( v & 0x7F800000 ) == 0x7F800000 && ( v & 0x7FFFFF ) != 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsInf( float x )
{
	return ( teFloatToUintBits( x ) & 0x7FFFFFFF ) == 0x7F800000;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsZero( float x )
{
	return x >= -1e-8f && x <= 1e-8f;
}

TENSHI_FUNC float TENSHI_CALL teDegrees( float radians )
{
	return radians*180/TENSHI_PI_F;
}
TENSHI_FUNC float TENSHI_CALL teRadians( float degrees )
{
	return degrees/180*TENSHI_PI_F;
}

TENSHI_FUNC float TENSHI_CALL teCos( float degrees )
{
	return cosf( teRadians( degrees ) );
}
TENSHI_FUNC float TENSHI_CALL teSin( float degrees )
{
	return sinf( teRadians( degrees ) );
}
TENSHI_FUNC float TENSHI_CALL teTan( float degrees )
{
	return tanf( teRadians( degrees ) );
}
TENSHI_FUNC float TENSHI_CALL teAsin( float x )
{
	return teDegrees( asinf( x ) );
}
TENSHI_FUNC float TENSHI_CALL teAcos( float x )
{
	return teDegrees( acosf( x ) );
}
TENSHI_FUNC float TENSHI_CALL teAtan( float x )
{
	return teDegrees( atanf( x ) );
}
TENSHI_FUNC float TENSHI_CALL teAtanfull( float y, float x )
{
	return teDegrees( atan2f( y, x ) );
}
TENSHI_FUNC float TENSHI_CALL teHcos( float degrees )
{
	return coshf( teRadians( degrees ) );
}
TENSHI_FUNC float TENSHI_CALL teHsin( float degrees )
{
	return sinhf( teRadians( degrees ) );
}
TENSHI_FUNC float TENSHI_CALL teHtan( float degrees )
{
	return tanhf( teRadians( degrees) );
}
TENSHI_FUNC float TENSHI_CALL teSq( float x )
{
	return x*x;
}
TENSHI_FUNC float TENSHI_CALL teSqrt( float x )
{
	return sqrtf( x );
}
TENSHI_FUNC float TENSHI_CALL teAbs( float x )
{
	return fabsf( x );
}
TENSHI_FUNC float TENSHI_CALL teExp( float x )
{
	return expf( x );
}
TENSHI_FUNC float TENSHI_CALL teFloor( float x )
{
#if 0
	return floorf( x );
#else
	if( ( teFloatToUintBits( x ) & 0x801FFFFF ) > 0x80000000 ) {
		return ( float )( ( TenshiInt32_t )x - 1 );
	}

	return ( float )( TenshiInt32_t )x;
#endif
}
TENSHI_FUNC float TENSHI_CALL teCeil( float x )
{
	return ceilf( x );
}
TENSHI_FUNC float TENSHI_CALL teRound( float x )
{
	return roundf( x );
}
TENSHI_FUNC float TENSHI_CALL teFrac( float x )
{
	return x - teFloor( x );
}
TENSHI_FUNC float teSign( float x )
{
	return x < -1e-8f ? -1.0f : ( x > 1e-8f ? 1.0f : 0.0f );
}
TENSHI_FUNC float TENSHI_CALL teMinF( float a, float b )
{
#if TENSHI_SSE_ENABLED
	float r = 0.0f;
	_mm_store_ss( &r, _mm_min_ss( _mm_set_ss( a ), _mm_set_ss( b ) ) );
	return r;
#else
	return a < b ? a : b;
#endif
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teMinI( TenshiInt32_t a, TenshiInt32_t b )
{
	return a < b ? a : b;
}
TENSHI_FUNC float TENSHI_CALL teMaxF( float a, float b )
{
#if TENSHI_SSE_ENABLED
	float r = 0.0f;
	_mm_store_ss( &r, _mm_max_ss( _mm_set_ss( a ), _mm_set_ss( b ) ) );
	return r;
#else
	return a > b ? a : b;
#endif
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teMaxI( TenshiInt32_t a, TenshiInt32_t b )
{
	return a > b ? a : b;
}
TENSHI_FUNC float TENSHI_CALL teClamp( float x, float l, float h )
{
#if TENSHI_SSE_ENABLED
	float r = 0.0f;
	_mm_store_ss
	(
		&r,
		_mm_min_ss
		(
			_mm_max_ss( _mm_set_ss( x ), _mm_set_ss( l ) ),
			_mm_set_ss( h )
		)
	);
	return r;
#else
	float r;

	r = x;

	r = ( r + h - teAbs( r - h ) )*0.5f;
	r = ( r + l - teAbs( r - l ) )*0.5f;

	return r;
#endif
}
TENSHI_FUNC float TENSHI_CALL teSaturate( float x )
{
	return teClamp( x, 0.0f, 1.0f );
}
TENSHI_FUNC float TENSHI_CALL teSaturateSigned( float x )
{
	return teClamp( x, -1.0f, 1.0f );
}
TENSHI_FUNC float TENSHI_CALL teLerp( float x, float y, float t )
{
	return x + ( y - x )*t;
}
TENSHI_FUNC float TENSHI_CALL teCerp( float x, float y, float z, float w, float t )
{
	float a, b, c;

	a = ( w - z ) - ( x - y );
	b = ( x - y ) - a;
	c = z - x;
	return t*( t*( t*a + b ) + c ) + y;
}
TENSHI_FUNC float TENSHI_CALL teSlerp( float a, float b, float t )
{
	float cosom, sinom, omega, scale[ 2 ];

	if( t <= 0.0f ) {
		return a;
	}
	if( t >= 1.0f ) {
		return b;
	}

	cosom = fmodf( ( a*b )*( a*b ), 1.0f );

	if( ( 1.0f - cosom ) > 1e-8f ) {
		omega = acosf( cosom );
		sinom = sinf( omega );
		scale[ 0 ] = sinf( ( 1.0f - t )*omega )/sinom;
		scale[ 1 ] = sinf( t*omega )/sinom;
	} else {
		scale[ 0 ] = 1.0f - t;
		scale[ 1 ] = t;
	}

	return a*scale[ 0 ] + b*scale[ 1 ];
}
TENSHI_FUNC float TENSHI_CALL teWrap360( float angle )
{
	/*
		This simultaneously checks whether angle is above 360 or below 0
		0x43B40000 is the integer bits representation 360.0f
	*/
	if( teFloatToUintBits( angle ) >= 0x43B40000 ) {
		angle -= teFloor( angle/360.0f )*360.0f;
	}
	
	return angle;
}
TENSHI_FUNC float TENSHI_CALL teWrap180( float angle )
{
	angle = teWrap360( angle );
	return angle > 180.0f ? angle - 360.0f : angle;
}
TENSHI_FUNC float TENSHI_CALL teAngleDelta( float a, float b )
{
	return teWrap180( a - b );
}
TENSHI_FUNC float TENSHI_CALL teCurveValue( float a, float da, float sp )
{
	return da + ( a - da )/( sp < 1.0f ? 1.0f : sp );
}
TENSHI_FUNC float TENSHI_CALL teWrapValue( float da )
{
	return teWrap360( da );
}
TENSHI_FUNC float TENSHI_CALL teNewXValue( float x, float a, float b )
{
	return x + sinf( teRadians( a ) )*b;
}
TENSHI_FUNC float TENSHI_CALL teNewYValue( float y, float a, float b )
{
	return y - sinf( teRadians( a ) )*b;
}
TENSHI_FUNC float TENSHI_CALL teNewZValue( float z, float a, float b )
{
	return z + cosf( teRadians( a ) )*b;
}
TENSHI_FUNC float TENSHI_CALL teCurveAngle( float a, float da, float sp )
{
	float diff;

	a = teWrap360( a );
	da = teWrap360( da );

	if( diff < -180.0f ) {
		diff = a + 360.0f - da;
	} else if( diff > 180.0f ) {
		diff = a - ( da + 360.0f );
	}

	return teWrap360( da + diff/( sp < 1.0f ? 1.0f : sp ) );
}
TENSHI_FUNC float TENSHI_CALL teDot2D( float x1, float y1, float x2, float y2 )
{
	return x1*x2 + y1*y2;
}
TENSHI_FUNC float TENSHI_CALL teDot3D( float x1, float y1, float z1, float x2, float y2, float z2 )
{
	return x1*x2 + y1*y2 + z1*z2;
}
TENSHI_FUNC float TENSHI_CALL teLengthSq2D( float x, float y )
{
	return teDot2D( x, y, x, y );
}
TENSHI_FUNC float TENSHI_CALL teLengthSq3D( float x, float y, float z )
{
	return teDot3D( x, y, z, x, y, z );
}
TENSHI_FUNC float TENSHI_CALL teLength2D( float x, float y )
{
	return sqrtf( teLengthSq2D( x, y ) );
}
TENSHI_FUNC float TENSHI_CALL teLength3D( float x, float y, float z )
{
	return sqrtf( teLengthSq3D( x, y, z ) );
}
TENSHI_FUNC float TENSHI_CALL teDistance2D( float x1, float y1, float x2, float y2 )
{
	return teLength2D( x1 - x2, y1 - y2 );
}
TENSHI_FUNC float TENSHI_CALL teDistance3D( float x1, float y1, float z1, float x2, float y2, float z2 )
{
	return teLength3D( x1 - x2, y1 - y2, z1 - z2 );
}
TENSHI_FUNC float TENSHI_CALL tePercentF( float numerator, float denominator )
{
	return numerator/denominator*100.0f;
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL tePercentI( TenshiInt32_t numerator, TenshiInt32_t denominator )
{
	return numerator*100/denominator;
}

TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teArgb( TenshiUInt32_t r, TenshiUInt32_t g, TenshiUInt32_t b, TenshiUInt32_t a )
{
	return
		( ( a&0xFF )<<24 ) |
		( ( r&0xFF )<<16 ) |
		( ( g&0xFF )<< 8 ) |
		( ( b&0xFF )<< 0 );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgb( TenshiUInt32_t r, TenshiUInt32_t g, TenshiUInt32_t b )
{
	return teArgb( r, g, b, 0xFF );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbA( TenshiUInt32_t argb )
{
	return ( argb>>24 ) & 0xFF;
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbR( TenshiUInt32_t argb )
{
	return ( argb>>16 ) & 0xFF;
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbG( TenshiUInt32_t argb )
{
	return ( argb>>8 ) & 0xFF;
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbB( TenshiUInt32_t argb )
{
	return argb & 0xFF;
}
static TenshiUInt32_t teFloatToChannel( float x )
{
	return ( TenshiUInt32_t )( teSaturate( x )*255.0f );
}
static float teChannelToFloat( TenshiUInt32_t x )
{
	return ( ( float )x )/255.0f;
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teArgbF( float r, float g, float b, float a )
{
	return teArgb( teFloatToChannel( r ), teFloatToChannel( g ), teFloatToChannel( b ), teFloatToChannel( a ) );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbF( float r, float g, float b )
{
	return teArgb( teFloatToChannel( r ), teFloatToChannel( g ), teFloatToChannel( b ), 255 );
}
TENSHI_FUNC float TENSHI_CALL teRgbAF( TenshiUInt32_t argb )
{
	return teChannelToFloat( teRgbA( argb ) );
}
TENSHI_FUNC float TENSHI_CALL teRgbRF( TenshiUInt32_t argb )
{
	return teChannelToFloat( teRgbR( argb ) );
}
TENSHI_FUNC float TENSHI_CALL teRgbGF( TenshiUInt32_t argb )
{
	return teChannelToFloat( teRgbG( argb ) );
}
TENSHI_FUNC float TENSHI_CALL teRgbBF( TenshiUInt32_t argb )
{
	return teChannelToFloat( teRgbB( argb ) );
}


/*
===============================================================================

	RANDOM NUMBER

	(Using PCG32)

===============================================================================
*/

TENSHI_FUNC void TENSHI_CALL tePCGSeed( TenshiPCGState_t *r, TenshiUInt64_t state, TenshiUInt64_t seq )
{
	r->state  = 0;
	r->inc    = ( seq << 1 ) | 1;
	(void)tePCGRnd( r );
	r->state += state;
	(void)tePCGRnd( r );
}
TENSHI_FUNC void TENSHI_CALL tePCGRandomize( TenshiPCGState_t *r, TenshiUInt32_t seedval )
{
	TenshiUInt64_t state, seq;

	state = ( ( TenshiUInt64_t )seedval )<<32 | ~( ( seedval>>16 ) | ( seedval<<16 ) );
	seq   = state - seedval*seedval*seedval + state*state - seedval;

	tePCGSeed( r, state, seq );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL tePCGRnd( TenshiPCGState_t *r )
{
	TenshiUInt64_t oldstate;
	TenshiUInt32_t xorshifted, rot;

	oldstate = r->state;
	r->state = oldstate*6364136223846793005ULL + r->inc;

	xorshifted = ( ( oldstate >> 18 ) ^ oldstate ) >> 27;

	rot = oldstate >> 59;
	return ( xorshifted >> rot ) | ( xorshifted << ( ( -rot ) & 31 ) );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL tePCGBoundedRnd( TenshiPCGState_t *r, TenshiUInt32_t bound )
{
	TenshiUInt32_t threshold;
	TenshiUInt32_t remainingIters;
	TenshiUInt32_t value;

	threshold = -bound % bound;

	remainingIters = 32;
	do {
		value = tePCGRnd( r );
		if( value >= threshold ) {
			break;
		}
	} while( --remainingIters != 0 );

	return value%bound;
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL tePCGRangedRnd( TenshiPCGState_t *r, TenshiInt32_t lowBound, TenshiInt32_t highBound )
{
	return lowBound + tePCGBoundedRnd( r, highBound - lowBound );
}

#undef TENSHI_FACILITY
#define TENSHI_FACILITY				kTenshiLog_CoreRT

#define TENSHI_PCG_STATE_INIT		0x853c49e6748fea9bULL
#define TENSHI_PCG_INC_INIT			0xda3e39cb94b95bdbULL

TENSHI_FUNC void *TENSHI_CALL teRNGAlloc_f( void *pParm )
{
	union {
		TenshiPCGState_t *r;
		void *p;
	} p;

	((void)pParm);

	p.p = teAlloc( sizeof( TenshiPCGState_t ), TENSHI_MEMTAG_RNG );
	if( !p.p ) {
		return ( void *)0;
	}

	p.r->state = TENSHI_PCG_STATE_INIT;
	p.r->inc   = TENSHI_PCG_INC_INIT;

	return p.p;
}
TENSHI_FUNC void TENSHI_CALL teRNGDealloc_f( void *p )
{
	teDealloc( p );
}

static TENSHI_FORCEINLINE TenshiPCGState_t *teRNG( TenshiIndex_t uIndex )
{
	return ( TenshiPCGState_t * )teUnwrapEngineObject( g_RNGPool, uIndex );
}

TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocRNG( void )
{
	return teAllocEngineObject( g_RNGPool, 0, ( void * )0 );
}
TENSHI_FUNC void TENSHI_CALL teMakeRNG( TenshiIndex_t RNGNumber )
{
	teAllocEngineObject( g_RNGPool, RNGNumber, ( void * )0 );
}
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teDeleteRNG( TenshiIndex_t RNGNumber )
{
	teDeallocEngineObject( g_RNGPool, RNGNumber );
	return 0;
}
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teRNGExist( TenshiIndex_t RNGNumber )
{
	return teEngineObjectExists( g_RNGPool, RNGNumber );
}

TENSHI_FUNC void TENSHI_CALL teRandomizeRNG( TenshiIndex_t RNGNumber, TenshiUInt32_t seedval )
{
	TenshiPCGState_t *r;

	if( !( r = teRNG( RNGNumber ) ) ) {
		return;
	}

	tePCGRandomize( r, seedval );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRNGGenerate( TenshiIndex_t RNGNumber )
{
	TenshiPCGState_t *r;

	if( !( r = teRNG( RNGNumber ) ) ) {
		return 0;
	}

	return tePCGRnd( r );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRNGBoundedGenerate( TenshiIndex_t RNGNumber, TenshiUInt32_t bound )
{
	TenshiPCGState_t *r;

	if( !( r = teRNG( RNGNumber ) ) ) {
		return 0;
	}

	return tePCGBoundedRnd( r, bound );
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teRNGRangedGenerate( TenshiIndex_t RNGNumber, TenshiInt32_t lowBound, TenshiInt32_t highBound )
{
	TenshiPCGState_t *r;

	if( !( r = teRNG( RNGNumber ) ) ) {
		return 0;
	}

	return tePCGRangedRnd( r, lowBound, highBound );
}

static TenshiPCGState_t g_RNG = {
	TENSHI_PCG_STATE_INIT,
	TENSHI_PCG_INC_INIT
};
TENSHI_FUNC void TENSHI_CALL teRandomize( TenshiUInt32_t seedval )
{
	tePCGRandomize( &g_RNG, seedval );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRnd( void )
{
	return tePCGRnd( &g_RNG );
}
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRndBounded( TenshiUInt32_t bound )
{
	return tePCGBoundedRnd( &g_RNG, bound );
}
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teRndRanged( TenshiInt32_t lowBound, TenshiInt32_t highBound )
{
	return tePCGRangedRnd( &g_RNG, lowBound, highBound );
}


/*
===============================================================================

	PROGRAM INITIALIZATION

===============================================================================
*/

/*
	FIXME: Convert argc/argv to proper UTF-8 strings on Windows
	TODO: Look into widestring variants of argc/argv for other platforms
*/

extern void TenshiMain( void );

static int CommonMain( int argc, char **argv )
{
	g_RTGlob.argc = argc;
	g_RTGlob.argv = argv;

	/*teInitGlob();*/
	atexit( &teFini );

	TenshiMain();

	return EXIT_SUCCESS;
}

int main( int argc, char **argv )
{
	return CommonMain( argc, argv );
}

#ifdef _WIN32
# ifndef WINAPI
#  define WINAPI __stdcall
#  define HINSTANCE void*
#  define LPSTR char *
# endif
int WINAPI WinMain(HINSTANCE a, HINSTANCE b, LPSTR c, int d) {
	/* unused parameters */
	((void)a); ((void)b); ((void)c); ((void)d);

	return CommonMain( __argc, __argv );
}
#endif

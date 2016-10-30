#ifndef __TENSHI_RUNTIME_H__
#define __TENSHI_RUNTIME_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef TENSHI_STATIC_LINK_ENABLED
# define TENSHI_STATIC_LINK_ENABLED	0
#endif

#define TENSHI_MEMTAG_DEFAULT		0
#define TENSHI_MEMTAG_STRING		1
#define TENSHI_MEMTAG_MEMBLOCK		2
#define TENSHI_MEMTAG_RNG			3

#ifndef TENSHI_MEMTAG
# define TENSHI_MEMTAG				TENSHI_MEMTAG_DEFAULT
#endif

#ifndef TENSHI_ASSERT
# ifdef assert
#  define TENSHI_ASSERT(Expr_)		assert(Expr_)
# else
#  define TENSHI_ASSERT(Expr_)		(!!(Expr_)?1:(exit(1),0))
# endif
#endif

struct TenshiInt128Struct_s;
struct TenshiRuntimeGlob_s;
struct TenshiType_s;
struct TenshiObject_s;
struct TenshiArray_s;
struct TenshiString_s;
struct TenshiList_s;
struct TenshiListItem_s;
struct TenshiBTree_s;
struct TenshiBTreeNode_s;
struct TenshiObjectPool_s;
struct TenshiMemblock_s;
struct TenshiReport_s;
struct TenshiChecklist_s;
struct TenshiChecklistItem_s;

typedef unsigned char				TenshiUInt8_t;
typedef unsigned short				TenshiUInt16_t;
typedef unsigned int				TenshiUInt32_t;
typedef unsigned long long			TenshiUInt64_t;
typedef signed char					TenshiInt8_t;
typedef signed short				TenshiInt16_t;
typedef signed int					TenshiInt32_t;
typedef signed long long			TenshiInt64_t;
typedef ptrdiff_t					TenshiIntPtr_t;
typedef size_t						TenshiUIntPtr_t;

typedef TenshiUInt32_t				TenshiBoolean_t;
enum
{
	TENSHI_FALSE = 0,
	TENSHI_TRUE = 1
};

typedef TenshiUInt32_t				TenshiIndex_t;
#define TENSHI_MAX_INDEX			((TenshiIndex_t)0x003FFFFF)
#define TENSHI_INVALID_INDEX		((TenshiIndex_t)0xFFFFFFFF)

typedef struct TenshiInt128Struct_s	TenshiInt128Struct_t;
typedef struct TenshiRuntimeGlob_s	TenshiRuntimeGlob_t;
typedef struct TenshiType_s			TenshiType_t;
typedef struct TenshiTypeObject_s	TenshiTypeObject_t;
typedef struct TenshiArray_s		TenshiArray_t;
typedef struct TenshiString_s		TenshiString_t;
typedef struct TenshiList_s			TenshiList_t;
typedef struct TenshiListItem_s		TenshiListItem_t;
typedef struct TenshiBTree_s		TenshiBTree_t;
typedef struct TenshiBTreeNode_s	TenshiBTreeNode_t;
typedef struct TenshiObjectPool_s	TenshiObjectPool_t;
typedef struct TenshiMemblock_s		TenshiMemblock_t;
typedef struct TenshiReport_s		TenshiReport_t;
typedef struct TenshiChecklist_s	TenshiChecklist_t;
typedef struct TenshiChecklistItem_s TenshiChecklistItem_t;

#ifndef TENSHI_CALL
# ifdef _WIN32
#  if 0
#   define TENSHI_CALL				__stdcall
#  else
#   define TENSHI_CALL				__cdecl
#  endif
# else
#  define TENSHI_CALL
# endif
#endif

#ifndef TENSHI_EXTRNC
# ifdef __cplusplus
#  define TENSHI_EXTRNC				extern "C"
# else
#  define TENSHI_EXTRNC
# endif
#endif

#ifndef TENSHI_IMPORT
# ifdef _WIN32
#  define TENSHI_IMPORT				__declspec( dllimport )
# else
#  define TENSHI_IMPORT
# endif
#endif

#ifndef TENSHI_EXPORT
# ifdef _WIN32
#  define TENSHI_EXPORT				__declspec( dllexport )
# else
#  define TENSHI_EXPORT
# endif
#endif

#ifndef TENSHI_FUNC
# define TENSHI_FUNC				TENSHI_EXTRNC TENSHI_IMPORT
#endif

#ifndef TENSHI_SELECTANY
# if defined( _WIN32 ) || defined( _MSC_VER )
#  define TENSHI_SELECTANY			__declspec(selectany)
# else
#  define TENSHI_SELECTANY			__attribute__((weak))
# endif
#endif

#ifndef TENSHI_FORCEINLINE
# if defined( _MSC_VER )
#  define TENSHI_FORCEINLINE		__forceinline
# elif defined( __GNUC__ )
#  define TENSHI_FORCEINLINE		__inline __attribute__(( always_inline ))
# elif defined( __cplusplus )
#  define TENSHI_FORCEINLINE		inline
# else
#  define TENSHI_FORCEINLINE		__inline
# endif
#endif

#ifndef TENSHI_NORETURN
# if defined( _MSC_VER )
#  define TENSHI_NORETURN			__declspec(noreturn)
# elif defined( __GNUC__ )
#  define TENSHI_NORETURN			__attribute__((noreturn))
# else
#  define TENSHI_NORETURN
# endif
#endif

#ifndef TENSHI_CURRENT_FUNCTION
# if defined( _MSC_VER )
#  define TENSHI_CURRENT_FUNCTION	__FUNCTION__
# else
#  define TENSHI_CURRENT_FUNCTION	__func__
# endif
#endif

enum
{
	kTenshiTypeF_TrivialInit		= 0x01,
	kTenshiTypeF_TrivialFini		= 0x02,
	kTenshiTypeF_TrivialCopy		= 0x04,
	kTenshiTypeF_TrivialMove		= 0x08,

	kTenshiTypeF_FullTrivial		= 0x0F
};

/* specifies the priority for a given report */
typedef enum TenshiReportPriority_e
{
	/* message contains information typically only useful to programmers */
	kTenshiLog_Debug				= 0x0000,
	/* general informational messages */
	kTenshiLog_Info					= 0x0001,
	/* non-error conditions where handling them should be considered */
	kTenshiLog_Notice				= 0x0002,
	/* something that is technically valid but might be undesired */
	kTenshiLog_Warning				= 0x0003,
	/* something that is invalid or wrong, but can be recovered from */
	kTenshiLog_Error				= 0x0004,
	/* errors that within their context cannot be recovered from */
	kTenshiLog_Critical				= 0x0005,
	/* condition that should be corrected immediately, such as a corrupt database */
	kTenshiLog_Alert				= 0x0006,
	/* a panic condition -- the application cannot recover, this is a fatal error */
	kTenshiLog_Panic				= 0x0007
} TenshiReportPriority_t;
#define TENSHI_LOG_PRIORITY_MASK	0x0007 /* 0000 0000 0000 0111 */

/* specifies which system generated the report */
typedef enum TenshiReportFacility_e
{
	/* the report comes from user code */
	kTenshiLog_UserCode				= 0x0000,
	/* the report comes from third party code */
	kTenshiLog_ThirdParty			= 0x0008,

	/* TenshiRuntime.c (generic) */
	kTenshiLog_CoreRT				= 0x0010,
	/* TenshiRuntime.c (allocations / deallocations) */
	kTenshiLog_CoreRT_Memory		= 0x0018,
	/* TenshiRuntime.c (object pools and instances) */
	kTenshiLog_CoreRT_Object		= 0x0020,
	/* TenshiRuntime.c (language-side type instances) */
	kTenshiLog_CoreRT_Type			= 0x0028,
	/* TenshiRuntime.c (string handling) */
	kTenshiLog_CoreRT_String		= 0x0030,
	/* TenshiRuntime.c (array management) */
	kTenshiLog_CoreRT_Array			= 0x0038,
	/* TenshiRuntime.c (linked-list management) */
	kTenshiLog_CoreRT_List			= 0x0040,
	/* TenshiRuntime.c (binary-tree management) */
	kTenshiLog_CoreRT_BTree			= 0x0048,

	/* Memory-Block API	:: Used for more securely managing memory */
	kTenshiLog_MemblockAPI			= 0x0050,
	/* CVars API		:: Console/configuration variables API */
	kTenshiLog_CVarAPI				= 0x0058,
	/* File-System API	:: Create and delete files/directories; other I/O */
	kTenshiLog_FileAPI				= 0x0060,
	/* System API		:: Timing, dynamic libraries, and miscellaneous OS interfaces */
	kTenshiLog_SystemAPI			= 0x0068,
	/* Math API			:: Various algorithms; mathematics */
	kTenshiLog_MathAPI				= 0x0070,
	/* Async API		:: Parallel / jobs / tasking API for fine-grained asynchronous operations */
	kTenshiLog_AsyncAPI				= 0x0078,
	/* Network API		:: UDP and TCP based connection management */
	kTenshiLog_NetworkAPI			= 0x0080,
	/* Windowing API	:: Provides basic windowing and GUI support */
	kTenshiLog_WindowingAPI			= 0x0088,
	/* Input API		:: Streamlined keyboard/mouse polling input, and gamepad support */
	kTenshiLog_InputAPI				= 0x0090,
	/* Audio API		:: Interface to sound mixing and tracks; streamable and in-memory; recording */
	kTenshiLog_AudioAPI				= 0x0098,
	/* Renderer API		:: Everything regarding the low-level renderer */
	kTenshiLog_RendererAPI			= 0x00A0,
	/* Image API		:: Provides support for various image formats and pixel manipulation routines */
	kTenshiLog_ImageAPI				= 0x00A8,
	/* Basic2D API		:: The core 2D library */
	kTenshiLog_Basic2DAPI			= 0x00B0,
	/* Basic3D API		:: The core 3D library */
	kTenshiLog_Basic3DAPI			= 0x00B8,
	/* Terrain API		:: Extension to the core 3D library for terrain management */
	kTenshiLog_TerrainAPI			= 0x00C0,
	/* Particles API	:: Extension to the core 3D library for particle systems */
	kTenshiLog_ParticlesAPI			= 0x00C8,
	/* Physics API		:: Interface to the physics library */
	kTenshiLog_PhysicsAPI			= 0x00D0,
	/* Baker API		:: Provides CSG, lightmapping, and other processes primarily useful to editors */
	kTenshiLog_BakerAPI				= 0x00D8,
	/* VR API			:: Interfaces with virtual reality devices (head-mounted displays primarily) */
	kTenshiLog_VRAPI				= 0x00E0

	/*
	kTenshiLog__ReservedFacility35	= 0x00E8,
	kTenshiLog__ReservedFacility34	= 0x00F0,
	kTenshiLog__ReservedFacility33	= 0x00F8,
	kTenshiLog__ReservedFacility32	= 0x0100,
	kTenshiLog__ReservedFacility31	= 0x0108,
	kTenshiLog__ReservedFacility30	= 0x0110,
	kTenshiLog__ReservedFacility29	= 0x0118,
	kTenshiLog__ReservedFacility28	= 0x0120,
	kTenshiLog__ReservedFacility27	= 0x0128,
	kTenshiLog__ReservedFacility26	= 0x0130,
	kTenshiLog__ReservedFacility25	= 0x0138,
	kTenshiLog__ReservedFacility24	= 0x0140,
	kTenshiLog__ReservedFacility23	= 0x0148,
	kTenshiLog__ReservedFacility22	= 0x0150,
	kTenshiLog__ReservedFacility21	= 0x0158,
	kTenshiLog__ReservedFacility20	= 0x0160,
	kTenshiLog__ReservedFacility19	= 0x0168,
	kTenshiLog__ReservedFacility18	= 0x0170,
	kTenshiLog__ReservedFacility17	= 0x0178,
	kTenshiLog__ReservedFacility16	= 0x0180,
	kTenshiLog__ReservedFacility15	= 0x0188,
	kTenshiLog__ReservedFacility14	= 0x0190,
	kTenshiLog__ReservedFacility13	= 0x0198,
	kTenshiLog__ReservedFacility12	= 0x01A0,
	kTenshiLog__ReservedFacility11	= 0x01A8,
	kTenshiLog__ReservedFacility10	= 0x01B0,
	kTenshiLog__ReservedFacility09	= 0x01B8,
	kTenshiLog__ReservedFacility08	= 0x01C0,
	kTenshiLog__ReservedFacility07	= 0x01C8,
	kTenshiLog__ReservedFacility06	= 0x01D0,
	kTenshiLog__ReservedFacility05	= 0x01D8,
	kTenshiLog__ReservedFacility04	= 0x01E0,
	kTenshiLog__ReservedFacility03	= 0x01E8,
	kTenshiLog__ReservedFacility02	= 0x01F0,
	kTenshiLog__ReservedFacility01	= 0x01F8
	*/
} TenshiReportFacility_t;
#define TENSHI_LOG_FACILITY_MASK	0x01F8 /* 0000 0001 1111 1000 */

/* specifies what caused a report to be generated */
typedef enum TenshiReportCause_e
{
	/* the report was generated on purpose, by design or requirement of the app */
	kTenshiLog_Intentional			= 0x0000,
	/* report generated because the app initialized successfully */
	kTenshiLog_Init					= 0x0200,
	/* report generated because the app is closing down */
	kTenshiLog_Fini					= 0x0400,

	/* the report was generated as a result of processing an internal file */
	kTenshiLog_InternalFile			= 0x0600,
	/* the report was generated as a result of processing a user supplied (external) file */
	kTenshiLog_ExternalFile			= 0x0800,

	/* report generated for tracing purposes */
	kTenshiLog_Trace				= 0x0A00,
	/* report generated for some other development/debug purpose */
	kTenshiLog_Development			= 0x0C00,
	/* report generated for statistics tracking */
	kTenshiLog_Stats				= 0x0E00,

	/* ran out of memory */
	kTenshiLog_OutOfMemory			= 0x1000,
	/* a buffer overflow was detected */
	kTenshiLog_BufferOverflow		= 0x1200,
	/* a buffer underflow was detected */
	kTenshiLog_BufferUnderflow		= 0x1400,
	/* an internal check in the code failed */
	kTenshiLog_FailedCheck			= 0x1600,
	/* an internal check in the code failed -- a pointer was expected to be null */
	kTenshiLog_FailedCheck_IsNull	= 0x1800,
	/* an internal check in the code failed -- a pointer was expected to not be null*/
	kTenshiLog_FailedCheck_NotNull	= 0x1A00

	/*
	kTenshiLog__ReservedCause2		= 0x1C00,
	kTenshiLog__ReservedCause1		= 0x1E00
	*/
} TenshiReportCause_t;
#define TENSHI_LOG_CAUSE_MASK		0x1E00 /* 0001 1110 0000 0000 */

enum
{
	/* Log system information (process ID, thread ID, error code(s), time) */
	kTenshiLogF_SystemInfo			= 0x2000

	/*
	kTenshiLogF__Reserved2			= 0x4000,
	kTenshiLogF__Reserved1			= 0x8000
	*/
};

typedef void *( TENSHI_CALL *TenshiFnAlloc_t )( TenshiUIntPtr_t cBytes, int Memtag );
typedef void( TENSHI_CALL *TenshiFnDealloc_t )( void *pData );

typedef char *( TENSHI_CALL *TenshiFnString_t )( char *pOldStrObj, TenshiUIntPtr_t cStrBytes );
typedef char *( TENSHI_CALL *TenshiFnStrDup_t )( const char *pszText );

typedef void( TENSHI_CALL *TenshiFnAutoprint_t )( TenshiIntPtr_t iVersion, const char *pszText );
typedef int( TENSHI_CALL *TenshiFnSafeSync_t )( void );

typedef void( TENSHI_CALL *TenshiFnLogfv_t )( TenshiUInt16_t flags, const char *pszMod, const char *pszFile,
	TenshiUInt32_t Line, const char *pszFunc, const char *pszExpr, const char *pszFormat, va_list args );

typedef TenshiBoolean_t( TENSHI_CALL *TenshiFnInstanceInit_t )( TenshiType_t *pType, void *pInstance );
typedef void( TENSHI_CALL *TenshiFnInstanceFini_t )( TenshiType_t *pType, void *pInstance );
typedef void( TENSHI_CALL *TenshiFnInstanceCopy_t )( TenshiType_t *pType, void *pDstInstance, const void *pSrcInstance );
typedef void( TENSHI_CALL *TenshiFnInstanceMove_t )( TenshiType_t *pType, void *pDstInstance, void *pSrcInstance );

typedef void *( TENSHI_CALL *TenshiFnAllocObject_t )( void * );
typedef void( TENSHI_CALL *TenshiFnDeallocObject_t )( void * );

typedef TenshiObjectPool_t *( TENSHI_CALL *TenshiFnAllocEnginePool_t )( TenshiFnAllocObject_t, TenshiFnDeallocObject_t );
typedef TenshiBoolean_t( TENSHI_CALL *TenshiFnEngineObjectExists_t )( const TenshiObjectPool_t *, TenshiIndex_t );
typedef void( TENSHI_CALL *TenshiFnReserveEngineObjects_t )( TenshiObjectPool_t *, TenshiIndex_t uBegin, TenshiIndex_t uEnd );
typedef TenshiIndex_t( TENSHI_CALL *TenshiFnAllocEngineObject_t )( TenshiObjectPool_t *, TenshiIndex_t, void * );
typedef void( TENSHI_CALL *TenshiFnDeallocEngineObject_t )( TenshiObjectPool_t *, TenshiIndex_t );
typedef void *( TENSHI_CALL *TenshiFnUnwrapEngineObject_t )( TenshiObjectPool_t *, TenshiIndex_t );

typedef TenshiIndex_t( TENSHI_CALL *TenshiFnAllocMemblock_t )( TenshiUIntPtr_t cBytes );
typedef void( TENSHI_CALL *TenshiFnMakeMemblock_t )( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t cBytes );
typedef TenshiIndex_t( TENSHI_CALL *TenshiFnDeleteMemblock_t )( TenshiIndex_t MemblockNumber );
typedef TenshiBoolean_t( TENSHI_CALL *TenshiFnMemblockExist_t )( TenshiIndex_t MemblockNumber );
typedef void *( TENSHI_CALL *TenshiFnGetMemblockPtr_t )( TenshiIndex_t MemblockNumber );
typedef TenshiUIntPtr_t( TENSHI_CALL *TenshiFnGetMemblockSize_t )( TenshiIndex_t MemblockNumber );

typedef void( TENSHI_CALL *TenshiFnSubmitReport_t )( const TenshiReport_t * );
typedef void( TENSHI_CALL *TenshiFnRuntimeErrorCallback_t )( const TenshiReport_t *, TenshiUInt32_t ErrorId );

typedef void( TENSHI_CALL *TenshiFnRuntimeError_t )( TenshiReportFacility_t, const char *pszModName, TenshiUInt32_t ErrorId );

#ifndef TENSHI_MINREPORT_ENABLED
# define TENSHI_MINREPORT_ENABLED	1
#endif

#ifndef TENSHI_MODNAME
# define TENSHI_MODNAME				((const char *)0)
#endif
#ifndef TENSHI_FACILITY
# define TENSHI_FACILITY			kTenshiLog_ThirdParty
#endif

#if TENSHI_MINREPORT_ENABLED
# define TELOG_DBG					kTenshiLog_Debug
# define TELOG_DEBUG				kTenshiLog_Debug

# define TELOG_INFO					kTenshiLog_Info

# define TELOG_NOTE					kTenshiLog_Notice
# define TELOG_NOTICE				kTenshiLog_Notice

# define TELOG_WARN					kTenshiLog_Warning
# define TELOG_WARNING				kTenshiLog_Warning

# define TELOG_ERR					kTenshiLog_Error
# define TELOG_ERROR				kTenshiLog_Error

# define TELOG_CRIT					kTenshiLog_Critical
# define TELOG_CRITICAL				kTenshiLog_Critical

# define TELOG_ALERT				kTenshiLog_Alert

# define TELOG_EMERG				kTenshiLog_Panic
# define TELOG_PANIC				kTenshiLog_Panic

# define TELOG_F_USR				kTenshiLog_UserCode
# define TELOG_F_TPC				kTenshiLog_ThirdParty

# define TELOG_F_RT					kTenshiLog_CoreRT
# define TELOG_F_RT_MM				kTenshiLog_CoreRT_Memory
# define TELOG_F_RT_OBJ				kTenshiLog_CoreRT_Object
# define TELOG_F_RT_TY				kTenshiLog_CoreRT_Type
# define TELOG_F_RT_STR				kTenshiLog_CoreRT_String
# define TELOG_F_RT_ARR				kTenshiLog_CoreRT_Array
# define TELOG_F_RT_LS				kTenshiLog_CoreRT_List
# define TELOG_F_RT_BT				kTenshiLog_CoreRT_BTree

# define TELOG_F_API_MM				kTenshiLog_MemblockAPI
# define TELOG_F_API_CV				kTenshiLog_CVarAPI
# define TELOG_F_API_FS				kTenshiLog_FileAPI
# define TELOG_F_API_M				kTenshiLog_MathAPI
# define TELOG_F_API_TSK			kTenshiLog_AsyncAPI
# define TELOG_F_API_NET			kTenshiLog_NetworkAPI
# define TELOG_F_API_WND			kTenshiLog_WindowingAPI
# define TELOG_F_API_IN				kTenshiLog_InputAPI
# define TELOG_F_API_SND			kTenshiLog_AudioAPI
# define TELOG_F_API_R				kTenshiLog_RendererAPI
# define TELOG_F_API_IMG			kTenshiLog_ImageAPI
# define TELOG_F_API_B2				kTenshiLog_Basic2DAPI
# define TELOG_F_API_B3				kTenshiLog_Basic3DAPI
# define TELOG_F_API_TER			kTenshiLog_TerrainAPI
# define TELOG_F_API_FX				kTenshiLog_ParticlesAPI
# define TELOG_F_API_PHY			kTenshiLog_PhysicsAPI
# define TELOG_F_API_BKR			kTenshiLog_BakerAPI
# define TELOG_F_API_VR				kTenshiLog_VRAPI

# define TELOG_C_APP				kTenshiLog_Intentional
# define TELOG_C_INIT				kTenshiLog_Init
# define TELOG_C_FINI				kTenshiLog_Fini

# define TELOG_C_IO_INT				kTenshiLog_InternalFile
# define TELOG_C_IO_EXT				kTenshiLog_ExternalFile

# define TELOG_C_TRACE				kTenshiLog_Trace
# define TELOG_C_DEBUG				kTenshiLog_Development
# define TELOG_C_STATS				kTenshiLog_Stats

# define TELOG_C_NOMEM				kTenshiLog_OutOfMemory
# define TELOG_C_BUFOVER			kTenshiLog_BufferOverflow
# define TELOG_C_BUFUNDER			kTenshiLog_BufferUnderflow

# define TELOG_C_FAIL				kTenshiLog_FailedCheck
# define TELOG_C_FAIL_NULL			kTenshiLog_FailedCheck_IsNull
# define TELOG_C_FAIL_NOTNULL		kTenshiLog_FailedCheck_NotNull

# define TELOGF_SYSINFO				kTenshiLogF_SystemInfo
#endif

struct TenshiInt128Struct_s
{
	TenshiUInt64_t					V					[ 2 ];
};

struct TenshiRTTypeInfo_s
{
	TenshiUIntPtr_t					cTypes;
	const TenshiType_t *			pTypes;
};

struct TenshiObjectPool_s
{
	TenshiFnAllocObject_t			pfnAlloc;
	TenshiFnDeallocObject_t			pfnDealloc;

	void **							ppObjects;
	TenshiUIntPtr_t *				pAges;
	TenshiIndex_t					cCapacity;
};

#define TENSHI_MAX_ENGINE_TYPES		64
struct TenshiEngineTypes_s
{
	TenshiObjectPool_t				Pools				[ TENSHI_MAX_ENGINE_TYPES ];
	unsigned char					cTypes;

	TenshiFnAllocEnginePool_t		pfnAllocPool;
	TenshiFnEngineObjectExists_t	pfnObjectExists;
	TenshiFnReserveEngineObjects_t	pfnReserveObjects;
	TenshiFnAllocEngineObject_t		pfnAllocObject;
	TenshiFnDeallocEngineObject_t	pfnDeallocObject;
	TenshiFnUnwrapEngineObject_t	pfnUnwrapObject;
};

struct TenshiMemblockAPI_s
{
	TenshiObjectPool_t *			pMemblockPool;

	TenshiFnAllocMemblock_t			pfnAllocMemblock;
	TenshiFnMakeMemblock_t			pfnMakeMemblock;
	TenshiFnDeleteMemblock_t		pfnDeleteMemblock;
	TenshiFnMemblockExist_t			pfnMemblockExist;

	TenshiFnGetMemblockPtr_t		pfnGetMemblockPtr;
	TenshiFnGetMemblockSize_t		pfnGetMemblockSize;
};

/* An individual log report in expanded form */
struct TenshiReport_s
{
	TenshiReportPriority_t			Priority;
	TenshiReportFacility_t			Facility;
	TenshiReportCause_t				Cause;

	TenshiUInt32_t					ProcessID;
	TenshiUInt32_t					ThreadID;
	int								POSIXErrorCode;
	TenshiUInt32_t					SystemErrorCode;

	const char *					pszModuleNameUTF8;

	const char *					pszFilenameUTF8;
	TenshiUInt32_t					LineNumber;
	TenshiUInt32_t					Column;
	const char *					pszFunctionUTF8;
	const char *					pszExpressionUTF8;

	const char *					pszMessageUTF8;
};

struct TenshiLoggingAPI_s
{
	TenshiFnSubmitReport_t			pfnSubmitReport;

	TenshiReportPriority_t			MinimumPriority;
	TenshiUInt32_t					DefaultFlags;
};

#define TENSHI_RTGLOB_VERSION		150306
struct TenshiRuntimeGlob_s
{
	TenshiUInt32_t					uRuntimeVersion;

	int								argc;
	char **							argv;

	struct TenshiRTTypeInfo_s *		pTypeInfo;
	struct TenshiEngineTypes_s *	pEngineTypes;

	TenshiFnAlloc_t					pfnAlloc;
	TenshiFnDealloc_t				pfnDealloc;
	int								iCurrentMemtag;

	TenshiFnString_t				pfnString;
	TenshiFnStrDup_t				pfnStrDup;

	TenshiFnAutoprint_t				pfnAutoprintCallback;
	TenshiFnSafeSync_t				pfnSafeSyncCallback;
	TenshiFnRuntimeErrorCallback_t	pfnRuntimeErrorCallback;

	TenshiFnLogfv_t					pfnLogfv;
	TenshiFnRuntimeError_t			pfnRuntimeError;

	struct TenshiMemblockAPI_s *	pMemblockAPI;
	struct TenshiLoggingAPI_s *		pLoggingAPI;
};

#define TENSHI_TYPE_INT8			((TenshiType_t*)1)
#define TENSHI_TYPE_INT16			((TenshiType_t*)2)
#define TENSHI_TYPE_INT32			((TenshiType_t*)3)
#define TENSHI_TYPE_INT64			((TenshiType_t*)4)
#define TENSHI_TYPE_INT128			((TenshiType_t*)5)
#define TENSHI_TYPE_INTPTR			((TenshiType_t*)6)
#define TENSHI_TYPE_UINT8			((TenshiType_t*)7)
#define TENSHI_TYPE_UINT16			((TenshiType_t*)8)
#define TENSHI_TYPE_UINT32			((TenshiType_t*)9)
#define TENSHI_TYPE_UINT64			((TenshiType_t*)10)
#define TENSHI_TYPE_UINT128			((TenshiType_t*)11)
#define TENSHI_TYPE_UINTPTR			((TenshiType_t*)12)
#define TENSHI_TYPE_FLOAT16			((TenshiType_t*)13)
#define TENSHI_TYPE_FLOAT32			((TenshiType_t*)14)
#define TENSHI_TYPE_FLOAT64			((TenshiType_t*)15)
#define TENSHI_TYPE_BOOLEAN			((TenshiType_t*)16)
#define TENSHI_TYPE_STRING			((TenshiType_t*)17)
#define TENSHI_TYPE_ARRAY			((TenshiType_t*)18)
#define TENSHI_TYPE_LIST			((TenshiType_t*)19)
#define TENSHI_TYPE_BTREE			((TenshiType_t*)20)
struct TenshiType_s
{
	/* kTenshiTypeF_ flags */
	TenshiUInt32_t					Flags;
	TenshiUInt32_t					cBytes;

	char *							pszName;
	char *							pszPattern;

	TenshiFnInstanceInit_t			pfnInit;
	TenshiFnInstanceFini_t			pfnFini;
	TenshiFnInstanceCopy_t			pfnCopy;
	TenshiFnInstanceMove_t			pfnMove;
};
struct TenshiTypeObject_s
{
	TenshiUIntPtr_t					cReferences;
	TenshiType_t *					pType;
};

#define TENSHI_ARRAY_MAX_DIMENSIONS	9
#define TENSHI_ARRAY_INVALID_INDEX	( ~( TenshiUIntPtr_t )0 )

/*
 *	ARRAY [COLLECTION]
 *	=====
 *	This is the header for arrays in Tenshi.
 *
 *	All of the data follows immediately after the header. When an array is
 *	reallocated, so is the header. They are part of the same allocation.
 */
struct TenshiArray_s
{
	/* total number of dimensions */
	TenshiUIntPtr_t					cDimensions;
	/* elements per dimension */
	TenshiUIntPtr_t					uDimensions			[ TENSHI_ARRAY_MAX_DIMENSIONS ];
	/* total number of elements */
	TenshiUIntPtr_t					cItems;
	/* size of one element within the array */
	TenshiUIntPtr_t					cItemBytes;
	/* type used by all elements */
	TenshiType_t *					pItemType;
	/* current index */
	TenshiUIntPtr_t					uIndex;
};

/*
 *	LINKED LIST [COLLECTION]
 *	===========
 *	This is the data structure for the base of a linked list. It references list
 *	items, which it can manipulate.
 */
struct TenshiList_s
{
	/* points to the first item in the list */
	TenshiListItem_t *				pHead;
	/* points to the last item in the list */
	TenshiListItem_t *				pTail;
	/* points to the currently set item in the list */
	TenshiListItem_t *				pCurr;

	/* number of items stored in the list */
	TenshiUIntPtr_t					cItems;
	/* size of one item */
	TenshiUIntPtr_t					cItemBytes;
	/* type for each item */
	TenshiType_t *					pItemType;

	/* index number for the currently cached item */
	TenshiUIntPtr_t					uCachedIndex;
	/* pointer to the currently cached item */
	TenshiListItem_t *				pCachedIndex;
};
struct TenshiListItem_s
{
	TenshiListItem_t *				pPrev;
	TenshiListItem_t *				pNext;
};

/*
 *	BINARY TREE [COLLECTION]
 *	===========
 *	Header for an associative array, implemented via binary tree. (Also known as
 *	a map.)
 */
struct TenshiBTree_s
{
	TenshiBTreeNode_t *				pRoot;
	TenshiBTreeNode_t *				pHead;
	TenshiBTreeNode_t *				pTail;

	TenshiType_t *					pItemType;
	TenshiUIntPtr_t					cItemBytes;
};
struct TenshiBTreeNode_s
{
	TenshiBTreeNode_t *				pPrnt;
	TenshiBTreeNode_t *				pLeft;
	TenshiBTreeNode_t *				pRght;

	TenshiInt32_t					Key;

	TenshiBTree_t *					pBase;
	TenshiBTreeNode_t *				pPrev;
	TenshiBTreeNode_t *				pNext;
};

/*
 *	MEMORY BLOCK
 *	============
 *	Unstructured block of memory for user modification.
 */
struct TenshiMemblock_s
{
	TenshiUIntPtr_t					cBytes;
	TenshiUIntPtr_t					uPos;
};

/*
 *	CHECKLIST
 *	=========
 *	Structured memory that the user can query. Often used to enumerate values.
 */
struct TenshiChecklist_s
{
	TenshiUIntPtr_t					cEntries;
	TenshiChecklistItem_t *			pEntries;
};
struct TenshiChecklistItem_s
{
	char *							pszValue;
	TenshiUInt64_t					uValue[ 4 ];
	double							fValue[ 4 ];
};


#if TENSHI_STATIC_LINK_ENABLED


/*
 *	RUNTIME (CORE) FUNCTIONS
 */

TENSHI_FUNC TenshiRuntimeGlob_t *TENSHI_CALL teGetGlob( void );

TENSHI_FUNC void *TENSHI_CALL teAlloc( TenshiUIntPtr_t cBytes, int Memtag );
TENSHI_FUNC void TENSHI_CALL teDealloc( void *pData );

TENSHI_FUNC void TENSHI_CALL teAutoprint( const char *pszText );
TENSHI_FUNC int TENSHI_CALL teSafeSync( void );

TENSHI_FUNC const char *TENSHI_CALL teGetReportPriorityStr( TenshiReportPriority_t x );
TENSHI_FUNC const char *TENSHI_CALL teGetReportFacilityStr( TenshiReportFacility_t x );
TENSHI_FUNC const char *TENSHI_CALL teGetReportCauseStr( TenshiReportCause_t x );
TENSHI_FUNC void TENSHI_CALL teLogfv( TenshiUInt16_t flags, const char *pszMod,
	const char *pszFile, TenshiUInt32_t Line, const char *pszFunc,
	const char *pszExpr, const char *pszFormatMessage, va_list args );
TENSHI_FUNC void TENSHI_CALL teLogf( TenshiUInt16_t flags, const char *pszMod,
	const char *pszFile, TenshiUInt32_t Line, const char *pszFunc,
	const char *pszExpr, const char *pszFormatMessage, ... );

TENSHI_FUNC void TENSHI_CALL teRuntimeError( TenshiReportFacility_t, const char *pszModName, TenshiUInt32_t ErrorId );

TENSHI_FUNC TenshiObjectPool_t *TENSHI_CALL teAllocEnginePool( TenshiFnAllocObject_t, TenshiFnDeallocObject_t );
TENSHI_FUNC void TENSHI_CALL teFiniEnginePool( TenshiObjectPool_t * );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teAllocEngineIndex( TenshiObjectPool_t *, TenshiIndex_t );
TENSHI_FUNC void TENSHI_CALL teReserveIndexes( TenshiObjectPool_t *, TenshiIndex_t uBeginIndex, TenshiIndex_t uEndIndex );
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teFindEngineIndex( TenshiObjectPool_t * );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teEngineObjectExists( const TenshiObjectPool_t *, TenshiIndex_t );
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocEngineObject( TenshiObjectPool_t *, TenshiIndex_t, void * );
TENSHI_FUNC void TENSHI_CALL teDeallocEngineObject( TenshiObjectPool_t *, TenshiIndex_t );
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teWrapEngineObject( TenshiObjectPool_t *, TenshiIndex_t );
TENSHI_FUNC void *TENSHI_CALL teUnwrapEngineObject( TenshiObjectPool_t *, TenshiIndex_t );


/*
 *	STRING FUNCTIONS
 */

TENSHI_FUNC char *TENSHI_CALL teStrAlloc( char *p, TenshiUIntPtr_t n );
TENSHI_FUNC char *TENSHI_CALL teStrReclaim( char *s );

TENSHI_FUNC char *TENSHI_CALL teStrDup( const char *s );
TENSHI_FUNC char *TENSHI_CALL teStrConcat( const char *a, const char *b );
TENSHI_FUNC char *TENSHI_CALL teStrFindRm( const char *a, const char *b );
TENSHI_FUNC char *TENSHI_CALL teStrRepeat( const char *s, TenshiUIntPtr_t n );
TENSHI_FUNC char *TENSHI_CALL teStrCatDir( const char *a, const char *b );

TENSHI_FUNC char *TENSHI_CALL teCastInt8ToStr( TenshiInt8_t i );
TENSHI_FUNC char *TENSHI_CALL teCastInt16ToStr( TenshiInt16_t i );
TENSHI_FUNC char *TENSHI_CALL teCastInt32ToStr( TenshiInt32_t i );
TENSHI_FUNC char *TENSHI_CALL teCastInt64ToStr( TenshiInt64_t i );
TENSHI_FUNC char *TENSHI_CALL teCastInt128ToStr( TenshiInt128Struct_t i );

TENSHI_FUNC char *TENSHI_CALL teCastUInt8ToStr( TenshiUInt8_t i );
TENSHI_FUNC char *TENSHI_CALL teCastUInt16ToStr( TenshiUInt16_t i );
TENSHI_FUNC char *TENSHI_CALL teCastUInt32ToStr( TenshiUInt32_t i );
TENSHI_FUNC char *TENSHI_CALL teCastUInt64ToStr( TenshiUInt64_t i );
TENSHI_FUNC char *TENSHI_CALL teCastUInt128ToStr( TenshiInt128Struct_t i );

TENSHI_FUNC char *TENSHI_CALL teCastFloat32ToStr( float f );
TENSHI_FUNC char *TENSHI_CALL teCastFloat64ToStr( double f );

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrInstance_Init_f( TenshiType_t *pType, void *pInstance );
TENSHI_FUNC void TENSHI_CALL teStrInstance_Fini_f( TenshiType_t *pType, void *pInstance );
TENSHI_FUNC void TENSHI_CALL teStrInstance_Copy_f( TenshiType_t *pType, void *pDstInstance, const void *pSrcInstance );
TENSHI_FUNC void TENSHI_CALL teStrInstance_Move_f( TenshiType_t *pType, void *pDstInstance, void *pSrcInstance );

TENSHI_FUNC int TENSHI_CALL teStr_Asc( const char *s );
TENSHI_FUNC char *TENSHI_CALL teStr_Chr( TenshiUInt32_t utf32cp );

TENSHI_FUNC char *TENSHI_CALL teStr_Bin( TenshiUInt32_t x );
TENSHI_FUNC char *TENSHI_CALL teStr_Hex( TenshiUInt32_t x );
TENSHI_FUNC char *TENSHI_CALL teStr_Oct( TenshiUInt32_t x );

TENSHI_FUNC char *TENSHI_CALL teStr_Lower( const char *s );
TENSHI_FUNC char *TENSHI_CALL teStr_Upper( const char *s );

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teStr_Len( const char *s );
TENSHI_FUNC int TENSHI_CALL teStr_SortCmp( const char *a, const char *b );
TENSHI_FUNC int TENSHI_CALL teStr_SortCmpCase( const char *a, const char *b );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrEq( const char *a, const char *b );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStrEqCase( const char *a, const char *b );

TENSHI_FUNC char *TENSHI_CALL teStr_Left( const char *s, TenshiIntPtr_t n );
TENSHI_FUNC char *TENSHI_CALL teStr_Mid( const char *s, TenshiIntPtr_t pos );
TENSHI_FUNC char *TENSHI_CALL teStr_MidLen( const char *s, TenshiIntPtr_t pos, TenshiUIntPtr_t len );
TENSHI_FUNC char *TENSHI_CALL teStr_Right( const char *s, TenshiIntPtr_t n );
TENSHI_FUNC char *TENSHI_CALL teStr_Skip( const char *s, TenshiUIntPtr_t n );
TENSHI_FUNC char *TENSHI_CALL teStr_Drop( const char *s, TenshiUIntPtr_t n );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_HasPrefix( const char *s, const char *prefix );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_HasSuffix( const char *s, const char *suffix );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teStr_Contains( const char *s, const char *search );


/*
 *	TYPE FUNCTIONS
 */

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsTypeTrivial( const TenshiType_t *pType );

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teInitTypeInstance( TenshiType_t *pType, void *pInstance );
TENSHI_FUNC void TENSHI_CALL teFiniTypeInstance( TenshiType_t *pType, void *pInstance );
TENSHI_FUNC void TENSHI_CALL teCopyTypeInstance( TenshiType_t *pType, void *pDstInstance, const void *pSrcInstance );
TENSHI_FUNC void TENSHI_CALL teMoveTypeInstance( TenshiType_t *pType, void *pDstInstance, void *pSrcInstance );

TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teNewTypeObject( TenshiType_t *pType );
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teDeleteTypeObject( TenshiTypeObject_t *pObj );
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teKeepTypeObject( TenshiTypeObject_t *pObj );
TENSHI_FUNC TenshiTypeObject_t *TENSHI_CALL teCopyTypeObject( const TenshiTypeObject_t *pObj );
TENSHI_FUNC void *TENSHI_CALL teGetTypeObject( TenshiTypeObject_t *pObj );
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teGetTypeObjectSize( const TenshiTypeObject_t *pObj );

TENSHI_FUNC TenshiType_t *TENSHI_CALL teFixType( TenshiType_t *p );


/*
 *	ARRAY FUNCTIONS
 */

TENSHI_FUNC void *TENSHI_CALL teArrayUndim( void *pArrayData );
TENSHI_FUNC void *TENSHI_CALL teArrayDim( const TenshiUIntPtr_t *pDimensions, TenshiUIntPtr_t cDimensions, TenshiType_t *pItemType );
TENSHI_FUNC void *TENSHI_CALL teArrayRedim( void *pOldArrayData, const TenshiUIntPtr_t *pDimensions, TenshiUIntPtr_t cDimensions, TenshiType_t *pItemType );

TENSHI_FUNC void *TENSHI_CALL teArrayInsertElements( void *pArrayData, TenshiUIntPtr_t uBefore, const void *pItems, TenshiUIntPtr_t cItems );
TENSHI_FUNC void TENSHI_CALL teArrayDeleteElements( void *pArrayData, TenshiUIntPtr_t uFirst, TenshiUIntPtr_t cDeletes );

TENSHI_FUNC void TENSHI_CALL teArrayIndexToBottom( void *pArrayData );
TENSHI_FUNC void TENSHI_CALL teArrayIndexToTop( void *pArrayData );
TENSHI_FUNC void TENSHI_CALL teArrayIndexToNext( void *pArrayData );
TENSHI_FUNC void TENSHI_CALL teArrayIndexToPrevious( void *pArrayData );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teArrayIsIndexValid( const void *pArrayData );

TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtTop( void *pArrayData );
TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtBottom( void *pArrayData );
TENSHI_FUNC void *TENSHI_CALL teArrayInsertAtElement( void *pArrayData, TenshiUIntPtr_t uElement );

TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtTop( void *pArrayData, TenshiUIntPtr_t cItems );
TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtBottom( void *pArrayData, TenshiUIntPtr_t cItems );
TENSHI_FUNC void *TENSHI_CALL teArrayInsertQuantityAtElement( void *pArrayData, TenshiUIntPtr_t uElement, TenshiUIntPtr_t cItems );

TENSHI_FUNC void TENSHI_CALL teArrayDeleteElementAt( void *pArrayData, TenshiUIntPtr_t uIndex );
TENSHI_FUNC void TENSHI_CALL teArrayDeleteElement( void *pArrayData );

TENSHI_FUNC void TENSHI_CALL teEmptyArray( void *pArrayData );

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayLen( const void *pArrayData );
TENSHI_FUNC TenshiIntPtr_t TENSHI_CALL teArrayCount( const void *pArrayData );

TENSHI_FUNC void TENSHI_CALL teArrayIndexToStack( void *pArrayData );
TENSHI_FUNC void *TENSHI_CALL teAddToStack( void *pArrayData );
TENSHI_FUNC void TENSHI_CALL teRemoveFromStack( void *pArrayData );

TENSHI_FUNC void TENSHI_CALL teArrayIndexToQueue( void *pArrayData );
TENSHI_FUNC void *TENSHI_CALL teAddToQueue( void *pArrayData );
TENSHI_FUNC void TENSHI_CALL teRemoveFromQueue( void *pArrayData );

TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayCurrentIndex( const void *pArrayData );
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teArrayDimensionLen( const void *pArrayData, TenshiUIntPtr_t uDim );


/*
 *	LINKED LIST FUNCTIONS
 */

TENSHI_FUNC TenshiList_t *TENSHI_CALL teNewList( TenshiType_t *pItemType );
TENSHI_FUNC TenshiList_t *TENSHI_CALL teDeleteList( TenshiList_t *pList );

TENSHI_FUNC void *TENSHI_CALL teListNodeItem( TenshiListItem_t *pItem );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListNodeFromItem( void *pData );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListFrontNode( TenshiList_t *pList );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListBackNode( TenshiList_t *pList );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL tePreviousListNode( TenshiListItem_t *pNode );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teNextListNode( TenshiListItem_t *pNode );

TENSHI_FUNC void *TENSHI_CALL teListFront( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListBack( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListPrevious( void *pData );
TENSHI_FUNC void *TENSHI_CALL teListNext( void *pData );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertBeforeNode( TenshiList_t *pList, TenshiListItem_t *pAfterNode );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertAfterNode( TenshiList_t *pList, TenshiListItem_t *pBeforeNode );

TENSHI_FUNC void *TENSHI_CALL teListInsertBefore( TenshiList_t *pList, void *pAfter );
TENSHI_FUNC void *TENSHI_CALL teListInsertAfter( TenshiList_t *pList, void *pBefore );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListInsertNode( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListInsert( TenshiList_t *pList );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListAddNodeToFront( TenshiList_t *pList );
TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListAddNodeToBack( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListAddToFront( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListAddToBack( TenshiList_t *pList );

TENSHI_FUNC void TENSHI_CALL teListDeleteNode( TenshiList_t *pList, TenshiListItem_t *pItem );

TENSHI_FUNC void TENSHI_CALL teListDeleteFront( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListDeleteBack( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListDelete( TenshiList_t *pList );

TENSHI_FUNC void TENSHI_CALL teListToFront( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListToBack( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListToPrevious( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListToNext( TenshiList_t *pList );

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teListIsValid( const TenshiList_t *pList );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListCurrentNode( TenshiList_t *pList );
TENSHI_FUNC void *TENSHI_CALL teListCurrent( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListResetCurrent( TenshiList_t *pList );

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teListIsEmpty( const TenshiList_t *pList );
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teListLen( const TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teEmptyList( TenshiList_t *pList );

TENSHI_FUNC void TENSHI_CALL teListMoveToFront( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListMoveToBack( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListMoveToPrevious( TenshiList_t *pList );
TENSHI_FUNC void TENSHI_CALL teListMoveToNext( TenshiList_t *pList );

TENSHI_FUNC TenshiListItem_t *TENSHI_CALL teListNodeAt( TenshiList_t *pList, TenshiUIntPtr_t uIndex );
TENSHI_FUNC void *TENSHI_CALL teListAt( TenshiList_t *pList, TenshiUIntPtr_t uIndex );


/*
 *	BINARY TREE FUNCTIONS
 */

TENSHI_FUNC TenshiBTree_t *TENSHI_CALL teNewBTree( TenshiType_t *pItemType );
TENSHI_FUNC TenshiBTree_t *TENSHI_CALL teDeleteBTree( TenshiBTree_t *pBase );

TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teBTreeIsEmpty( const TenshiBTree_t *pBase );
TENSHI_FUNC void TENSHI_CALL teEmptyBTree( TenshiBTree_t *pBase );

TENSHI_FUNC void *TENSHI_CALL teBTreeItemFromNode( TenshiBTreeNode_t *pNode );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeNodeFromItem( void *pItem );

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeFindNode( TenshiBTree_t *pBase, TenshiInt32_t iKey );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeLookupNode( TenshiBTree_t *pBase, TenshiInt32_t iKey );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeInsertNode( TenshiBTree_t *pBase, TenshiInt32_t iKey );

TENSHI_FUNC void *TENSHI_CALL teBTreeFind( TenshiBTree_t *pBase, TenshiInt32_t iKey );
TENSHI_FUNC void *TENSHI_CALL teBTreeLookup( TenshiBTree_t *pBase, TenshiInt32_t iKey );
TENSHI_FUNC void *TENSHI_CALL teBTreeInsert( TenshiBTree_t *pBase, TenshiInt32_t iKey );

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeDeleteNode( TenshiBTree_t *pBase, TenshiBTreeNode_t *pNode );
TENSHI_FUNC void *TENSHI_CALL teBTreeDelete( TenshiBTree_t *pBase, void *pItem );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teBTreeNodeKey( const TenshiBTreeNode_t *pNode );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teBTreeKey( const void *pItem );

TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeFrontNode( TenshiBTree_t *pBase );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeBackNode( TenshiBTree_t *pBase );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreePreviousNode( TenshiBTreeNode_t *pNode );
TENSHI_FUNC TenshiBTreeNode_t *TENSHI_CALL teBTreeNextNode( TenshiBTreeNode_t *pNode );

TENSHI_FUNC void *TENSHI_CALL teBTreeFront( TenshiBTree_t *pBase );
TENSHI_FUNC void *TENSHI_CALL teBTreeBack( TenshiBTree_t *pBase );
TENSHI_FUNC void *TENSHI_CALL teBTreePrevious( void *pItem );
TENSHI_FUNC void *TENSHI_CALL teBTreeNext( void *pItem );

/*
 *	MEMBLOCK FUNCTIONS
 */

TENSHI_FUNC void *TENSHI_CALL teMemblockAlloc_f( void *pParm );
TENSHI_FUNC void TENSHI_CALL teMemblockDealloc_f( void *p );

TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocMemblock( TenshiUIntPtr_t cBytes );
TENSHI_FUNC void TENSHI_CALL teMakeMemblock( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t cBytes );
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teDeleteMemblock( TenshiIndex_t MemblockNumber );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teMemblockExist( TenshiIndex_t MemblockNumber );

TENSHI_FUNC void *TENSHI_CALL teGetMemblockPtr( TenshiIndex_t MemblockNumber );
TENSHI_FUNC TenshiUIntPtr_t TENSHI_CALL teGetMemblockSize( TenshiIndex_t MemblockNumber );

TENSHI_FUNC TenshiUInt8_t TENSHI_CALL teMemblockByte( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );
TENSHI_FUNC TenshiUInt16_t TENSHI_CALL teMemblockWord( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teMemblockDword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );
TENSHI_FUNC TenshiUInt64_t TENSHI_CALL teMemblockQword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );
TENSHI_FUNC float TENSHI_CALL teMemblockFloat( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );
TENSHI_FUNC double TENSHI_CALL teMemblockFloat64( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos );

TENSHI_FUNC void TENSHI_CALL teWriteMemblockByte( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt8_t Value );
TENSHI_FUNC void TENSHI_CALL teWriteMemblockWord( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt16_t Value );
TENSHI_FUNC void TENSHI_CALL teWriteMemblockDword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt32_t Value );
TENSHI_FUNC void TENSHI_CALL teWriteMemblockQword( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, TenshiUInt64_t Value );
TENSHI_FUNC void TENSHI_CALL teWriteMemblockFloat( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, float Value );
TENSHI_FUNC void TENSHI_CALL teWriteMemblockFloat64( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t uPos, double Value );

TENSHI_FUNC void TENSHI_CALL teCopyMemblock( TenshiIndex_t MemblockFrom, TenshiIndex_t MemblockTo, TenshiUIntPtr_t uPosFrom, TenshiUIntPtr_t uPosTo, TenshiUIntPtr_t cBytes );

/*
 *	MATH FUNCTIONS
 */

TENSHI_FUNC float TENSHI_CALL teUintBitsToFloat( TenshiUInt32_t x );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teFloatToUintBits( float x );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsNAN( float x );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsInf( float x );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teIsZero( float x );
TENSHI_FUNC float TENSHI_CALL teDegrees( float radians );
TENSHI_FUNC float TENSHI_CALL teRadians( float degrees );
TENSHI_FUNC float TENSHI_CALL teCos( float degrees );
TENSHI_FUNC float TENSHI_CALL teSin( float degrees );
TENSHI_FUNC float TENSHI_CALL teTan( float degrees );
TENSHI_FUNC float TENSHI_CALL teAsin( float x );
TENSHI_FUNC float TENSHI_CALL teAcos( float x );
TENSHI_FUNC float TENSHI_CALL teAtan( float x );
TENSHI_FUNC float TENSHI_CALL teAtanfull( float y, float x );
TENSHI_FUNC float TENSHI_CALL teHcos( float degrees );
TENSHI_FUNC float TENSHI_CALL teHsin( float degrees );
TENSHI_FUNC float TENSHI_CALL teHtan( float degrees );
TENSHI_FUNC float TENSHI_CALL teSq( float x );
TENSHI_FUNC float TENSHI_CALL teSqrt( float x );
TENSHI_FUNC float TENSHI_CALL teAbs( float x );
TENSHI_FUNC float TENSHI_CALL teExp( float x );
TENSHI_FUNC float TENSHI_CALL teFloor( float x );
TENSHI_FUNC float TENSHI_CALL teCeil( float x );
TENSHI_FUNC float TENSHI_CALL teRound( float x );
TENSHI_FUNC float TENSHI_CALL teFrac( float x );
TENSHI_FUNC float TENSHI_CALL teSign( float x );
TENSHI_FUNC float TENSHI_CALL teMinF( float a, float b );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teMinI( TenshiInt32_t a, TenshiInt32_t b );
TENSHI_FUNC float TENSHI_CALL teMaxF( float a, float b );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teMaxI( TenshiInt32_t a, TenshiInt32_t b );
TENSHI_FUNC float TENSHI_CALL teClamp( float x, float l, float h );
TENSHI_FUNC float TENSHI_CALL teSaturate( float x );
TENSHI_FUNC float TENSHI_CALL teSaturateSigned( float x );
TENSHI_FUNC float TENSHI_CALL teLerp( float x, float y, float t );
TENSHI_FUNC float TENSHI_CALL teCerp( float x, float y, float z, float w, float t );
TENSHI_FUNC float TENSHI_CALL teSlerp( float a, float b, float t );
TENSHI_FUNC float TENSHI_CALL teWrap360( float angle );
TENSHI_FUNC float TENSHI_CALL teWrap180( float angle );
TENSHI_FUNC float TENSHI_CALL teAngleDelta( float a, float b );
TENSHI_FUNC float TENSHI_CALL teCurveValue( float a, float da, float sp );
TENSHI_FUNC float TENSHI_CALL teWrapValue( float da );
TENSHI_FUNC float TENSHI_CALL teNewXValue( float x, float a, float b );
TENSHI_FUNC float TENSHI_CALL teNewYValue( float y, float a, float b );
TENSHI_FUNC float TENSHI_CALL teNewZValue( float z, float a, float b );
TENSHI_FUNC float TENSHI_CALL teCurveAngle( float a, float da, float sp );
TENSHI_FUNC float TENSHI_CALL teDot2D( float x1, float y1, float x2, float y2 );
TENSHI_FUNC float TENSHI_CALL teDot3D( float x1, float y1, float z1, float x2, float y2, float z2 );
TENSHI_FUNC float TENSHI_CALL teLengthSq2D( float x, float y );
TENSHI_FUNC float TENSHI_CALL teLengthSq3D( float x, float y, float z );
TENSHI_FUNC float TENSHI_CALL teLength2D( float x, float y );
TENSHI_FUNC float TENSHI_CALL teLength3D( float x, float y, float z );
TENSHI_FUNC float TENSHI_CALL teDistance2D( float x1, float y1, float x2, float y2 );
TENSHI_FUNC float TENSHI_CALL teDistance3D( float x1, float y1, float z1, float x2, float y2, float z2 );
TENSHI_FUNC float TENSHI_CALL tePercentF( float numerator, float denominator );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL tePercentI( TenshiInt32_t numerator, TenshiInt32_t denominator );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teArgb( TenshiUInt32_t r, TenshiUInt32_t g, TenshiUInt32_t b, TenshiUInt32_t a );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgb( TenshiUInt32_t r, TenshiUInt32_t g, TenshiUInt32_t b );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbA( TenshiUInt32_t argb );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbR( TenshiUInt32_t argb );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbG( TenshiUInt32_t argb );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbB( TenshiUInt32_t argb );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teArgbF( float r, float g, float b, float a );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRgbF( float r, float g, float b );
TENSHI_FUNC float TENSHI_CALL teRgbAF( TenshiUInt32_t argb );
TENSHI_FUNC float TENSHI_CALL teRgbRF( TenshiUInt32_t argb );
TENSHI_FUNC float TENSHI_CALL teRgbGF( TenshiUInt32_t argb );
TENSHI_FUNC float TENSHI_CALL teRgbBF( TenshiUInt32_t argb );

/*
 *	RANDOM NUMBER
 */

typedef struct TenshiPCGState_s {
	TenshiUInt64_t state; /* can be any value */
	TenshiUInt64_t inc;   /* must always be odd */
} TenshiPCGState_t;

TENSHI_FUNC void TENSHI_CALL tePCGSeed( TenshiPCGState_t *r, TenshiUInt64_t state, TenshiUInt64_t seq );
TENSHI_FUNC void TENSHI_CALL tePCGRandomize( TenshiPCGState_t *r, TenshiUInt32_t seedval );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL tePCGRnd( TenshiPCGState_t *r );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL tePCGBoundedRnd( TenshiPCGState_t *r, TenshiUInt32_t bound );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL tePCGRangedRnd( TenshiPCGState_t *r, TenshiInt32_t lowBound, TenshiInt32_t highBound );

TENSHI_FUNC void *TENSHI_CALL teRNGAlloc_f( void *pParm );
TENSHI_FUNC void TENSHI_CALL teRNGDealloc_f( void *p );

TENSHI_FUNC TenshiIndex_t TENSHI_CALL teAllocRNG( void );
TENSHI_FUNC void TENSHI_CALL teMakeRNG( TenshiIndex_t RNGNumber );
TENSHI_FUNC TenshiIndex_t TENSHI_CALL teDeleteRNG( TenshiIndex_t RNGNumber );
TENSHI_FUNC TenshiBoolean_t TENSHI_CALL teRNGExist( TenshiIndex_t RNGNumber );

TENSHI_FUNC void TENSHI_CALL teRandomizeRNG( TenshiIndex_t RNGNumber, TenshiUInt32_t seedval );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRNGGenerate( TenshiIndex_t RNGNumber );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRNGBoundedGenerate( TenshiIndex_t RNGNumber, TenshiUInt32_t bound );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teRNGRangedGenerate( TenshiIndex_t RNGNumber, TenshiInt32_t lowBound, TenshiInt32_t highBound );

TENSHI_FUNC void TENSHI_CALL teRandomize( TenshiUInt32_t seedval );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRnd( void );
TENSHI_FUNC TenshiUInt32_t TENSHI_CALL teRndBounded( TenshiUInt32_t bound );
TENSHI_FUNC TenshiInt32_t TENSHI_CALL teRndRanged( TenshiInt32_t lowBound, TenshiInt32_t highBound );

#endif /*TENSHI_STATIC_LINK_ENABLED*/


/*
===============================================================================

	C++ SPECIFIC STUFF

===============================================================================
*/

#ifdef __cplusplus
namespace Tenshi { struct PlcmntNw {}; }

inline void *operator new( TenshiUIntPtr_t, void *p, Tenshi::PlcmntNw )
{
	return p;
}
inline void operator delete( void *, void *, Tenshi::PlcmntNw )
{
}

namespace Tenshi
{

#ifndef TENSHI_SHAREDLINK
# ifdef _MSC_VER
#  define TENSHI_SHAREDLINK TENSHI_SELECTANY extern
# else
#  define TENSHI_SHAREDLINK TENSHI_SELECTANY
# endif
#endif

	TENSHI_SHAREDLINK TenshiRuntimeGlob_t *			g_pGlob						= 0;

# if !TENSHI_STATIC_LINK_ENABLED
	TENSHI_SHAREDLINK TenshiFnAlloc_t				g_pfnAlloc					= 0;
	TENSHI_SHAREDLINK TenshiFnDealloc_t				g_pfnDealloc				= 0;

	TENSHI_SHAREDLINK TenshiFnString_t				g_pfnString					= 0;
	TENSHI_SHAREDLINK TenshiFnStrDup_t				g_pfnStrDup					= 0;

	TENSHI_SHAREDLINK TenshiFnLogfv_t				g_pfnLogfv					= 0;
	TENSHI_SHAREDLINK TenshiFnRuntimeError_t		g_pfnRuntimeError			= 0;
	
	TENSHI_SHAREDLINK TenshiFnAllocEnginePool_t		g_pfnAllocEnginePool		= 0;
	TENSHI_SHAREDLINK TenshiFnEngineObjectExists_t	g_pfnEngineObjectExists		= 0;
	TENSHI_SHAREDLINK TenshiFnReserveEngineObjects_t	g_pfnReserveEngineObjects	= 0;
	TENSHI_SHAREDLINK TenshiFnAllocEngineObject_t	g_pfnAllocEngineObject		= 0;
	TENSHI_SHAREDLINK TenshiFnDeallocEngineObject_t	g_pfnDeallocEngineObject	= 0;
	TENSHI_SHAREDLINK TenshiFnUnwrapEngineObject_t	g_pfnUnwrapEngineObject		= 0;

	TENSHI_SHAREDLINK TenshiFnAllocMemblock_t		g_pfnAllocMemblock			= 0;
	TENSHI_SHAREDLINK TenshiFnMakeMemblock_t		g_pfnMakeMemblock			= 0;
	TENSHI_SHAREDLINK TenshiFnDeleteMemblock_t		g_pfnDeleteMemblock			= 0;
	TENSHI_SHAREDLINK TenshiFnMemblockExist_t		g_pfnMemblockExist			= 0;
	TENSHI_SHAREDLINK TenshiFnGetMemblockPtr_t		g_pfnGetMemblockPtr			= 0;
	TENSHI_SHAREDLINK TenshiFnGetMemblockSize_t		g_pfnGetMemblockSize		= 0;
# endif

	TENSHI_FORCEINLINE void *Alloc( TenshiUIntPtr_t cBytes, int iMemtag = TENSHI_MEMTAG )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teAlloc( cBytes, iMemtag );
# else
		return g_pfnAlloc( cBytes, iMemtag );
# endif
	}
	TENSHI_FORCEINLINE void *Dealloc( void *p )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teDealloc( p );
# else
		g_pfnDealloc( p );
# endif
		return ( void * )0;
	}

	TENSHI_FORCEINLINE char *MakeString( TenshiUIntPtr_t cNewStringBytes )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teStrAlloc( ( char * )0, cNewStringBytes );
# else
		return g_pfnString( ( char * )0, cNewStringBytes );
# endif
	}
	TENSHI_FORCEINLINE char *MakeString( char *pOldString, TenshiUIntPtr_t cNewStringBytes )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teStrAlloc( pOldString, cNewStringBytes );
# else
		return g_pfnString( pOldString, cNewStringBytes );
# endif
	}
	TENSHI_FORCEINLINE char *KillString( char *pOldString )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teStrAlloc( pOldString, 0 );
# else
		return g_pfnString( pOldString, 0 );
# endif
	}
	TENSHI_FORCEINLINE char *StrDup( const char *pszText )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teStrDup( pszText );
# else
		return g_pfnStrDup( pszText );
# endif
	}

	TENSHI_FORCEINLINE void Logfv( TenshiUInt16_t flags, const char *pszMod, const char *pszFile,
	TenshiUInt32_t Line, const char *pszFunc, const char *pszExpr, const char *pszFormat, va_list args )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teLogfv( flags, pszMod, pszFile, Line, pszFunc, pszExpr, pszFormat, args );
# else
		g_pfnLogfv( flags, pszMod, pszFile, Line, pszFunc, pszExpr, pszFormat, args );
# endif
	}
	inline void Logf( TenshiUInt16_t flags, const char *pszMod, const char *pszFile,
	TenshiUInt32_t Line, const char *pszFunc, const char *pszExpr, const char *pszFormat, ... )
	{
		va_list args;

		va_start( args, pszFormat );
# if TENSHI_STATIC_LINK_ENABLED
		teLogfv( flags, pszMod, pszFile, Line, pszFunc, pszExpr, pszFormat, args );
# else
		g_pfnLogfv( flags, pszMod, pszFile, Line, pszFunc, pszExpr, pszFormat, args );
# endif
		va_end( args );
	}
	TENSHI_NORETURN TENSHI_FORCEINLINE void RuntimeError( TenshiUInt32_t ErrorId, const char *pszModName = TENSHI_MODNAME, TenshiReportFacility_t Facility = TENSHI_FACILITY )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teRuntimeError( Facility, pszModName, ErrorId );
# else
		g_pfnRuntimeError( Facility, pszModName, ErrorId );
# endif
		abort();
	}

	TENSHI_FORCEINLINE TenshiObjectPool_t *AllocEnginePool( TenshiFnAllocObject_t pfnAlloc, TenshiFnDeallocObject_t pfnDealloc )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teAllocEnginePool( pfnAlloc, pfnDealloc );
# else
		return g_pfnAllocEnginePool( pfnAlloc, pfnDealloc );
# endif
	}
	TENSHI_FORCEINLINE bool EngineObjectExists( const TenshiObjectPool_t *p, TenshiIndex_t i )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teEngineObjectExists( p, i ) != TENSHI_FALSE;
# else
		return g_pfnEngineObjectExists( p, i ) != TENSHI_FALSE;
# endif
	}
	TENSHI_FORCEINLINE void ReserveEngineObjects( TenshiObjectPool_t *p, TenshiIndex_t uBeginIndex, TenshiIndex_t uEndIndex )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teReserveIndexes( p, uBeginIndex, uEndIndex );
# else
		g_pfnReserveEngineObjects( p, uBeginIndex, uEndIndex );
# endif
	}
	TENSHI_FORCEINLINE TenshiIndex_t AllocEngineObject( TenshiObjectPool_t *p, TenshiIndex_t i, void *pParm )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teAllocEngineObject( p, i, pParm );
# else
		return g_pfnAllocEngineObject( p, i, pParm );
# endif
	}
	TENSHI_FORCEINLINE void DeallocEngineObject( TenshiObjectPool_t *p, TenshiIndex_t i )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teDeallocEngineObject( p, i );
# else
		g_pfnDeallocEngineObject( p, i );
# endif
	}
	TENSHI_FORCEINLINE void *UnwrapEngineObject( TenshiObjectPool_t *p, TenshiIndex_t i )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teUnwrapEngineObject( p, i );
# else
		return g_pfnUnwrapEngineObject( p, i );
# endif
	}

	TENSHI_FORCEINLINE TenshiIndex_t MakeMemblock( TenshiUIntPtr_t cBytes )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teAllocMemblock( cBytes );
# else
		return g_pfnAllocMemblock( cBytes );
# endif
	}
	TENSHI_FORCEINLINE void MakeMemblock( TenshiIndex_t MemblockNumber, TenshiUIntPtr_t cBytes )
	{
# if TENSHI_STATIC_LINK_ENABLED
		teMakeMemblock( MemblockNumber, cBytes );
# else
		g_pfnMakeMemblock( MemblockNumber, cBytes );
# endif
	}
	TENSHI_FORCEINLINE TenshiIndex_t DeleteMemblock( TenshiIndex_t MemblockNumber )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teDeleteMemblock( MemblockNumber );
# else
		return g_pfnDeleteMemblock( MemblockNumber );
# endif
	}
	TENSHI_FORCEINLINE bool MemblockExist( TenshiIndex_t MemblockNumber )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teMemblockExist( MemblockNumber ) != TENSHI_FALSE;
# else
		return g_pfnMemblockExist( MemblockNumber ) != TENSHI_FALSE;
# endif
	}
	TENSHI_FORCEINLINE void *GetMemblockPtr( TenshiIndex_t MemblockNumber )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teGetMemblockPtr( MemblockNumber );
# else
		return g_pfnGetMemblockPtr( MemblockNumber );
# endif
	}
	TENSHI_FORCEINLINE TenshiUIntPtr_t GetMemblockSize( TenshiIndex_t MemblockNumber )
	{
# if TENSHI_STATIC_LINK_ENABLED
		return teGetMemblockSize( MemblockNumber );
# else
		return g_pfnGetMemblockSize( MemblockNumber );
# endif
	}

	struct StrRef
	{
		const char *s;
		const char *e;

		inline StrRef()
		: s( ( const char * )0 )
		, e( ( const char * )0 )
		{
		}
		inline StrRef( const char *p )
		: s( p )
		, e( !!p ? p + strlen( p ) : ( const char * )0 )
		{
		}
		inline StrRef( const char *s, const char *e )
		: s( s )
		, e( e )
		{
		}
		inline StrRef( const char *p, size_t n )
		: s( p )
		, e( p + n )
		{
		}
		inline StrRef( const StrRef &x )
		: s( x.s )
		, e( x.e )
		{
		}

		inline TenshiUIntPtr_t Len() const
		{
			return ( TenshiUIntPtr_t )( e - s );
		}

		inline StrRef Skip( TenshiUIntPtr_t n = 1 ) const
		{
			if( n >= Len() ) {
				return StrRef();
			}

			return StrRef( s + n, e );
		}
		inline StrRef Drop( TenshiUIntPtr_t n = 1 ) const
		{
			if( n >= Len() ) {
				return StrRef();
			}

			return StrRef( s, e - n );
		}

		inline StrRef Left( TenshiUIntPtr_t n = 1 ) const
		{
			if( n >= Len() ) { return StrRef( *this ); }
			return StrRef( s, s + n );
		}
		inline StrRef Right( TenshiUIntPtr_t n = 1 ) const
		{
			if( n >= Len() ) { return StrRef( *this ); }
			return StrRef( e - n, e );
		}
		inline StrRef Mid( TenshiUIntPtr_t off, TenshiUIntPtr_t n = 1 ) const
		{
			if( !n || off >= Len() ) { return StrRef(); }
			return StrRef( s + off, off + n > Len() ? e : s + off + n );
		}
		inline StrRef Substr( TenshiUIntPtr_t start, TenshiUIntPtr_t end ) const
		{
			const TenshiUIntPtr_t n = Len();

			if( end > n ) { end = n; }
			if( start >= n ) { return StrRef(); }
			if( end <= start ) { return StrRef(); }

			return StrRef( s + start, s + end );
		}

		inline bool operator==( const StrRef &x ) const
		{
			return
				Len() == x.Len() &&
				memcmp( ( const void * )s, ( const void * )x.s, Len() ) == 0;
		}
		inline bool operator!=( const StrRef &x ) const
		{
			return
				Len() != x.Len() ||
				memcmp( ( const void * )s, ( const void * )x.s, Len() ) != 0;
		}

		inline bool StartsWith( char x ) const
		{
			if( !Len() ) { return false; }
			return *s == x;
		}
		inline bool StartsWith( const StrRef &x ) const
		{
			const TenshiUIntPtr_t xn = x.Len();
			return Len() >= xn && Left( xn ) == x;
		}
		inline bool EndsWith( char x ) const
		{
			if( !Len() ) { return false; }
			return *( e - 1 ) == x;
		}
		inline bool EndsWith( const StrRef &x ) const
		{
			const TenshiUIntPtr_t xn = x.Len();
			return Len() >= xn && Right( xn ) == x;
		}

		inline StrRef Find( char x ) const
		{
			const TenshiUIntPtr_t n = Len();
			if( !n ) { return StrRef(); }

			const unsigned char y = x;

			const void *const p = memchr( ( const void * )s, +y, n );
			if( !p ) { return StrRef(); }

			return StrRef( ( const char * )p, e );
		}
		inline StrRef Find( const StrRef &x ) const
		{
			const TenshiUIntPtr_t n = Len();
			const TenshiUIntPtr_t xn = x.Len();

			if( !n || xn < n ) { return StrRef(); }

			const char ch = x[0];
			StrRef testset( *this );
			for(;;) {
				const StrRef t = testset.Find( ch );
				if( !t ) { break; }

				if( t.StartsWith( x ) ) {
					return t;
				}

				testset = t.Skip();
			}

			return StrRef();
		}

		inline StrRef FindPathSep() const
		{
#ifdef _WIN32
			const StrRef a = Find( '/' );
			const StrRef b = Find( '\\' );

			if( a && b ) {
				return a.s < b.s ? a : b;
			}

			return a ? a : b;
#else
			return Find( '/' );
#endif
		}

		inline StrRef FindLast( char x ) const
		{
			StrRef Last;
			StrRef Test( *this );
			for(;;) {
				const StrRef p = Test.Find( x );
				if( !p ) {
					break;
				}

				Last = p;
				Test = p.Skip();
			}
			return Last;
		}
		inline StrRef FindLast( const StrRef &x ) const
		{
			StrRef Last;
			StrRef Test( *this );
			for(;;) {
				const StrRef p = Test.Find( x );
				if( !p ) {
					break;
				}

				Last = p;
				Test = p.Skip();
			}
			return Last;
		}
		inline StrRef FindLastPathSep() const
		{
#ifdef _WIN32
			StrRef Last;
			StrRef Test( *this );
			for(;;) {
				const StrRef a = Test.Find( '/' );
				const StrRef b = Test.Find( '\\' );

				if( !a && !b ) {
					break;
				}

				Last = a.s > b.s ? a : b;
				Test = Last.Skip();
			}
			return Last;
#else
			return FindLast( '/' );
#endif
		}

		inline bool Contains( char x ) const
		{
			return Find( x );
		}
		inline bool Contains( const StrRef &x ) const
		{
			return Find( x );
		}

		inline char *Dup() const
		{
			const size_t n = Len();
			char *const p = MakeString( n );
			if( !p ) {
				return ( char * )0;
			}
			memcpy( ( void * )p, ( const void * )s, n );
			p[ n ] = '\0';
			return p;
		}

		inline operator bool() const
		{
			return !!s && !!e;
		}
		inline bool operator!() const
		{
			return !s || !e;
		}

		inline char operator[]( TenshiUIntPtr_t i ) const
		{
			if( i >= Len() ) {
				return '\0';
			}

			return s[ i ];
		}
	};

	template< typename tObject >
	class THandler
	{
	public:
		THandler()
		: m_pPool( ( TenshiObjectPool_t * )0 )
		{
		}
		~THandler()
		{
		}

		bool Init()
		{
			m_pPool = AllocEnginePool( &Alloc_f, &Dealloc_f );
			return m_pPool != 0;
		}

		TENSHI_FORCEINLINE bool Exists( TenshiIndex_t uIndex ) const
		{
			return EngineObjectExists( m_pPool, uIndex );
		}

		TENSHI_FORCEINLINE TenshiIndex_t Make()
		{
			return AllocEngineObject( m_pPool, 0, ( void * )0 );
		}
		TENSHI_FORCEINLINE TenshiIndex_t Make( TenshiIndex_t uIndex )
		{
			return AllocEngineObject( m_pPool, uIndex, ( void * )0 );
		}
		TENSHI_FORCEINLINE void Free( TenshiIndex_t uIndex )
		{
			DeallocEngineObject( m_pPool, uIndex );
		}

		TENSHI_FORCEINLINE tObject *Unwrap( TenshiIndex_t uIndex ) const
		{
			return
				reinterpret_cast< tObject * >
				(
					UnwrapEngineObject( m_pPool, uIndex )
				);
		}

	private:
		TenshiObjectPool_t *		m_pPool;
		
		static void *TENSHI_CALL Alloc_f( void * )
		{
			void *const p = Alloc( sizeof( tObject ), TENSHI_MEMTAG );
			if( !p ) {
				return 0;
			}

			new( p, PlcmntNw() ) tObject();
			return p;
		}
		static void TENSHI_CALL Dealloc_f( void *p )
		{
			if( !p ) {
				return;
			}

			( ( tObject * )p )->~tObject();

			Dealloc( p );
		}
	};

	inline void Init( TenshiRuntimeGlob_t *pGlob )
	{
		g_pGlob									= pGlob;

		TENSHI_ASSERT
		(
			(!pGlob || pGlob->uRuntimeVersion >= TENSHI_RTGLOB_VERSION)
			&& "Mismatched runtime/SDK (runtime is older than SDK)"
		);

		g_pfnAlloc								= 0;
		g_pfnDealloc							= 0;

		g_pfnString								= 0;
		g_pfnStrDup								= 0;

		g_pfnRuntimeError						= 0;

		g_pfnAllocEnginePool					= 0;
		g_pfnEngineObjectExists					= 0;
		g_pfnReserveEngineObjects				= 0;
		g_pfnAllocEngineObject					= 0;
		g_pfnDeallocEngineObject				= 0;
		g_pfnUnwrapEngineObject					= 0;

		g_pfnAllocMemblock						= 0;
		g_pfnMakeMemblock						= 0;
		g_pfnDeleteMemblock						= 0;
		g_pfnMemblockExist						= 0;
		g_pfnGetMemblockPtr						= 0;
		g_pfnGetMemblockSize					= 0;

		if( !pGlob ) {
			return;
		}

		g_pfnAlloc								= pGlob->pfnAlloc;
		g_pfnDealloc							= pGlob->pfnDealloc;

		g_pfnString								= pGlob->pfnString;
		g_pfnStrDup								= pGlob->pfnStrDup;

		g_pfnLogfv								= pGlob->pfnLogfv;
		g_pfnRuntimeError						= pGlob->pfnRuntimeError;

		struct TenshiEngineTypes_s *const p		= pGlob->pEngineTypes;

		if( !!p ) {
			g_pfnAllocEnginePool				= p->pfnAllocPool;
			g_pfnEngineObjectExists				= p->pfnObjectExists;
			g_pfnReserveEngineObjects			= p->pfnReserveObjects;
			g_pfnAllocEngineObject				= p->pfnAllocObject;
			g_pfnDeallocEngineObject			= p->pfnDeallocObject;
			g_pfnUnwrapEngineObject				= p->pfnUnwrapObject;
		}

		struct TenshiMemblockAPI_s *const mb	= pGlob->pMemblockAPI;

		if( !!mb ) {
			g_pfnAllocMemblock					= mb->pfnAllocMemblock;
			g_pfnMakeMemblock					= mb->pfnMakeMemblock;
			g_pfnDeleteMemblock					= mb->pfnDeleteMemblock;
			g_pfnMemblockExist					= mb->pfnMemblockExist;
			g_pfnGetMemblockPtr					= mb->pfnGetMemblockPtr;
			g_pfnGetMemblockSize				= mb->pfnGetMemblockSize;
		}
	}

}

# ifndef TENSHI_TRACE_ENABLED
#  ifdef _DEBUG
#   define TENSHI_TRACE_ENABLED		1
#  else
#   define TENSHI_TRACE_ENABLED		0
#  endif
# endif

# if TENSHI_TRACE_ENABLED
#  define TE_TRACE(...)				Tenshi::Logf(\
										TELOG_DEBUG | TENSHI_FACILITY | TELOG_C_TRACE,\
										(TENSHI_MODNAME),\
										__FILE__, __LINE__, TENSHI_CURRENT_FUNCTION,\
										( const char * )0,\
										__VA_ARGS__)
# else
#  define TE_TRACE(...)				((void)0)
# endif

# define TE_CHECK(RTErrCode_,Expr_)\
									do { if( !(Expr_) ) {\
										Tenshi::Logf(\
											TELOG_DEBUG | TENSHI_FACILITY | TELOG_C_FAIL,\
											(TENSHI_MODNAME),\
											__FILE__, __LINE__, TENSHI_CURRENT_FUNCTION,\
											( #Expr_ ),\
											"Runtime check failed");\
										Tenshi::RuntimeError((RTErrCode_), (TENSHI_MODNAME), (TENSHI_FACILITY));\
									} } while(false)
# define TE_CHECK_NOT_NULL(RTErrCode_,Expr_)\
									do { if( !(Expr_) ) {\
										Tenshi::Logf(\
											TELOG_DEBUG | TENSHI_FACILITY | TELOG_C_FAIL_NOTNULL,\
											(TENSHI_MODNAME),\
											__FILE__, __LINE__, TENSHI_CURRENT_FUNCTION,\
											( "'" #Expr_ "' is null" ),\
											"Runtime check failed; pointer is null");\
										Tenshi::RuntimeError((RTErrCode_), (TENSHI_MODNAME), (TENSHI_FACILITY));\
									} } while(false)
# define TE_CHECK_IS_NULL(RTErrCode_,Expr_)\
									do { if( !!(Expr_) ) {\
										Tenshi::Logf(\
											TELOG_DEBUG | TENSHI_FACILITY | TELOG_C_FAIL_NULL,\
											(TENSHI_MODNAME),\
											__FILE__, __LINE__, TENSHI_CURRENT_FUNCTION,\
											( "'" #Expr_ "' is not null" ),\
											"Runtime check failed; pointer is not null");\
										Tenshi::RuntimeError((RTErrCode_), (TENSHI_MODNAME), (TENSHI_FACILITY));\
									} } while(false)

#endif /*__cplusplus*/


/*
 *	ERROR CODES
 */

#define TE_ERR_SYSTEM				1
#define TE_ERR_EXISTS				2
#define TE_ERR_BADALLOC				3
#define TE_ERR_INVALID				4

#define TE_ERR_FS_DIRUNUSED			101
#define TE_ERR_FS_FILEUNUSED		102

#endif

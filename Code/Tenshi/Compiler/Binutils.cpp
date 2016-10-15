#include "_PCH.hpp"
#include "Binutils.hpp"
#include "Shell.hpp"
#include <sys/stat.h>

// TODO: Select between the debug and release version based on build mode

#ifndef TENSHIRUNTIME_O
# if 0
#  if AX_DEBUG_ENABLED
#   define TENSHIRUNTIME_O "TenshiRuntimeDbg.o"
#  else
#   define TENSHIRUNTIME_O "TenshiRuntime.o"
#  endif
# else
#  if AX_DEBUG_ENABLED
#   define TENSHIRUNTIME_O "libTenshiRuntimeDbg.a"
#  else
#   define TENSHIRUNTIME_O "libTenshiRuntime.a"
#  endif
# endif
#endif
#ifndef GNU_SYSROOT_DIR
# define GNU_SYSROOT_DIR "GNU"
#endif

namespace Tenshi { namespace Compiler {

	MBinutils &MBinutils::GetInstance()
	{
		static MBinutils instance;
		return instance;
	}

	MBinutils::MBinutils()
	: m_SysRoot()
	, m_LD()
	, m_LibDir()
	, m_IntLibDir()
	, m_Obj_CRT2()
	, m_Obj_CRTBegin()
	, m_Obj_CRTEnd()
	, m_Obj_TenshiRuntime()
	{
	}
	MBinutils::~MBinutils()
	{
	}

	static bool GetPath( char *szDstAbsPath, Ax::uintptr cMaxDstAbsPathBytes, const char *pszRelPath )
	{
		static char szAppDir[ PATH_MAX + 1 ] = { '\0' };

		AX_ASSERT_NOT_NULL( szDstAbsPath );
		AX_ASSERT( cMaxDstAbsPathBytes > 0 );
		AX_ASSERT_NOT_NULL( pszRelPath );

		if( !szAppDir[ 0 ] && !Ax::System::GetAppDir( szAppDir ) ) {
			return false;
		}

		char szBuff[ PATH_MAX + 1 ];
		szBuff[ sizeof( szBuff ) - 1 ] = '\0'; // suppress C6054

		Ax::StrCpy( szBuff, szAppDir );
		Ax::uintptr appendIndex = strlen( szBuff );

		if( appendIndex > 0 && ( szBuff[ appendIndex - 1 ] != '/' && szBuff[ appendIndex - 1 ] != '\\' ) ) {
			Ax::AppendString( szBuff, appendIndex, AX_DIRSEP );
		}

		Ax::AppendString( szBuff, appendIndex, pszRelPath );

		if( appendIndex + 1 >= sizeof( szBuff ) || appendIndex + 1 > cMaxDstAbsPathBytes ) {
			return false;
		}

		return Ax::GetAbsolutePath( szDstAbsPath, cMaxDstAbsPathBytes, szBuff ) != nullptr;
	}
	template< Ax::uintptr tMaxBytes >
	static bool GetPath( char( &szDstAbsPath )[ tMaxBytes ], const char *pszRelPath )
	{
		return GetPath( szDstAbsPath, tMaxBytes, pszRelPath );
	}
	
	bool MBinutils::Init()
	{
		char szBuff[ PATH_MAX + 1 ];

		if( !GetPath( szBuff, GNU_SYSROOT_DIR ) ) {
			Ax::BasicErrorf( "Could not obtain path to GNU sysroot" );
			return false;
		}

		AX_EXPECT_MSG( m_SysRoot.Assign( szBuff ), "Out of memory" );
		AX_DEBUG_LOG += "sysroot: " + m_SysRoot;

		// Find LD
		AX_EXPECT_MSG( m_LD.Assign( m_SysRoot ), "Out of memory" );
		AX_EXPECT_MSG( m_LD.AppendPath( "bin" AX_DIRSEP "ld" ), "Out of memory" );

		// Append '.exe' to all programs if we're in Windows
#ifdef _WIN32
		AX_EXPECT_MSG( m_LD.Append( ".exe" ), "Out of memory" );
#endif

		// Find the lib directories
		AX_EXPECT_MSG( m_LibDir.Assign( m_SysRoot ), "Out of memory" );
		AX_EXPECT_MSG( m_LibDir.AppendPath( "lib" ), "Out of memory" );

		AX_EXPECT_MSG( m_IntLibDir.Assign( m_LibDir ), "Out of memory" );

		// Find the objects that we need to link against
		AX_EXPECT_MSG( m_Obj_CRT2.Assign( m_LibDir ), "Out of memory" );
		AX_EXPECT_MSG( m_Obj_CRT2.AppendPath( "crt2.o" ), "Out of memory" );

		AX_EXPECT_MSG( m_Obj_CRTBegin.Assign( m_IntLibDir ), "Out of memory" );
		AX_EXPECT_MSG( m_Obj_CRTBegin.AppendPath( "crtbegin.o" ), "Out of memory" );

		AX_EXPECT_MSG( m_Obj_CRTEnd.Assign( m_IntLibDir ), "Out of memory" );
		AX_EXPECT_MSG( m_Obj_CRTEnd.AppendPath( "crtend.o" ), "Out of memory" );

		// Find TenshiRuntime.o
		if( !GetPath( szBuff, TENSHIRUNTIME_O ) ) {
			Ax::BasicErrorf( "Could not obtain path to TenshiRuntime.o" );
			return false;
		}

		AX_EXPECT_MSG( m_Obj_TenshiRuntime.Assign( szBuff ), "Out of memory" );

		AX_DEBUG_LOG += TENSHIRUNTIME_O ": " + m_Obj_TenshiRuntime;

		// Done
		return true;
	}

	int MBinutils::Link( const Ax::String &Output, const Ax::TArray< Ax::String > &InObjects, const SModule::IntrList &InMods ) const
	{
		static const Ax::uintptr kExtraReserved = 16;
		Ax::TArray< Ax::String > CommandLine;

		Ax::String OutDir;
		AX_EXPECT_MSG( OutDir.Assign( Output.ExtractDirectory() ), "Out of memory" );
		if( OutDir.IsEmpty() ) {
			AX_EXPECT_MSG( OutDir.Assign( Ax::System::GetDir() ), "Out of memory" );
		}

		AX_EXPECT_MSG( CommandLine.Reserve( InObjects.Num() + kExtraReserved ), "Out of memory" );

		AX_EXPECT_MSG( CommandLine.Append( m_LD ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-o" ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( Output ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( m_Obj_CRT2 ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( m_Obj_CRTBegin ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-L" ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( m_LibDir ), "Out of memory" );
		if( m_IntLibDir != m_LibDir ) {
			AX_EXPECT_MSG( CommandLine.Append( "-L" ), "Out of memory" );
			AX_EXPECT_MSG( CommandLine.Append( m_IntLibDir ), "Out of memory" );
		}
		AX_EXPECT_MSG( CommandLine.Append( InObjects ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( m_Obj_TenshiRuntime ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-lmingw32" ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-lgcc" ), "Out of memory" ); //only needed for stack checks
		AX_EXPECT_MSG( CommandLine.Append( "-lmingwex" ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-lmsvcrt" ), "Out of memory" );
		AX_EXPECT_MSG( CommandLine.Append( "-lkernel32" ), "Out of memory" );

		// Add modules
		for( const SModule::IntrLink *pModLink = InMods.HeadLink(); pModLink != nullptr; pModLink = pModLink->NextLink() ) {
			const SModule *const pMod = pModLink->Node();
			AX_ASSERT_NOT_NULL( pMod );

			const SModule &Mod = *pMod;

			// Skip the internal module if it was found here
			if( Mod.Type == EModule::Internal ) {
				continue;
			}

			// TODO: Distinguish between the debug and release filenames
			const Ax::String &ModFilename = Mod.DebugFilename;

			bool bCopyMod = false;

			struct stat srcmod, dstmod;
			if( stat( ModFilename, &srcmod ) != 0 ) {
				Ax::Errorf( ModFilename, "File not found" );
				return EXIT_FAILURE;
			}

			const Ax::String ModDstFilename = ( OutDir / ModFilename.ExtractFilename() ).CleanPathSlashes();
			if( stat( ModDstFilename, &dstmod ) == 0 ) {
				bCopyMod = srcmod.st_mtime > dstmod.st_mtime;
			} else {
				bCopyMod = true;
			}

			AX_DEBUG_LOG += "Dst: " + ModDstFilename;
			if( bCopyMod ) {
				// FIXME: Use Ax file-system routine when it's implemented
#ifdef _WIN32
				if( !CopyFileW( ( LPCWSTR )ModFilename.AsUTF16().Pointer(), ( LPCWSTR )ModDstFilename.AsUTF16().Pointer(), FALSE ) ) {
					Ax::Warnf( ModDstFilename, "Failed to create copy" );
				}
#else
				AX_DEBUG_LOG += "TODO: Copy [" + ModFilename + "] -> [" + ModDstFilename + "]";
#endif
			}

			AX_EXPECT_MSG( CommandLine.Append( ModFilename ), "Out of memory" );
		}
		AX_EXPECT_MSG( CommandLine.Append( m_Obj_CRTEnd ), "Out of memory" );

		return Shell->Run( CommandLine );
	}

}}

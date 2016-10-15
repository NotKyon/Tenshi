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

		AX_EXPECT_MEMORY( m_SysRoot.Assign( szBuff ) );
		AX_DEBUG_LOG += "sysroot: " + m_SysRoot;

		// Find LD
		AX_EXPECT_MEMORY( m_LD.Assign( m_SysRoot ) );
		AX_EXPECT_MEMORY( m_LD.AppendPath( "bin" AX_DIRSEP "ld" ) );

		// Append '.exe' to all programs if we're in Windows
#ifdef _WIN32
		AX_EXPECT_MEMORY( m_LD.Append( ".exe" ) );
#endif

		// Find the lib directories
		AX_EXPECT_MEMORY( m_LibDir.Assign( m_SysRoot ) );
		AX_EXPECT_MEMORY( m_LibDir.AppendPath( "lib" ) );

		AX_EXPECT_MEMORY( m_IntLibDir.Assign( m_LibDir ) );

		// Find the objects that we need to link against
		AX_EXPECT_MEMORY( m_Obj_CRT2.Assign( m_LibDir ) );
		AX_EXPECT_MEMORY( m_Obj_CRT2.AppendPath( "crt2.o" ) );

		AX_EXPECT_MEMORY( m_Obj_CRTBegin.Assign( m_IntLibDir ) );
		AX_EXPECT_MEMORY( m_Obj_CRTBegin.AppendPath( "crtbegin.o" ) );

		AX_EXPECT_MEMORY( m_Obj_CRTEnd.Assign( m_IntLibDir ) );
		AX_EXPECT_MEMORY( m_Obj_CRTEnd.AppendPath( "crtend.o" ) );

		// Find TenshiRuntime.o
		if( !GetPath( szBuff, TENSHIRUNTIME_O ) ) {
			Ax::BasicErrorf( "Could not obtain path to \"" TENSHIRUNTIME_O "\"" );
			return false;
		}

		AX_EXPECT_MEMORY( m_Obj_TenshiRuntime.Assign( szBuff ) );

		AX_DEBUG_LOG += TENSHIRUNTIME_O ": " + m_Obj_TenshiRuntime;

		// Done
		return true;
	}

	int MBinutils::Link( const Ax::String &Output, const Ax::TArray< Ax::String > &InObjects, const SModule::IntrList &InMods ) const
	{
		static const Ax::uintptr kExtraReserved = 16;
		Ax::TArray< Ax::String > CommandLine;

		Ax::String OutDir;
		AX_EXPECT_MEMORY( OutDir.Assign( Output.ExtractDirectory() ) );
		if( OutDir.IsEmpty() ) {
			AX_EXPECT_MEMORY( OutDir.Assign( Ax::System::GetDir() ) );
		}

		AX_EXPECT_MEMORY( CommandLine.Reserve( InObjects.Num() + kExtraReserved ) );

		AX_EXPECT_MEMORY( CommandLine.Append( m_LD ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-o" ) );
		AX_EXPECT_MEMORY( CommandLine.Append( Output ) );
		AX_EXPECT_MEMORY( CommandLine.Append( m_Obj_CRT2 ) );
		AX_EXPECT_MEMORY( CommandLine.Append( m_Obj_CRTBegin ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-L" ) );
		AX_EXPECT_MEMORY( CommandLine.Append( m_LibDir ) );
		if( m_IntLibDir != m_LibDir ) {
			AX_EXPECT_MEMORY( CommandLine.Append( "-L" ) );
			AX_EXPECT_MEMORY( CommandLine.Append( m_IntLibDir ) );
		}
		AX_EXPECT_MEMORY( CommandLine.Append( InObjects ) );
		AX_EXPECT_MEMORY( CommandLine.Append( m_Obj_TenshiRuntime ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-lmingw32" ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-lgcc" ) ); //only needed for stack checks
		AX_EXPECT_MEMORY( CommandLine.Append( "-lmingwex" ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-lmsvcrt" ) );
		AX_EXPECT_MEMORY( CommandLine.Append( "-lkernel32" ) );

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

			AX_EXPECT_MEMORY( CommandLine.Append( ModFilename ) );
		}
		AX_EXPECT_MEMORY( CommandLine.Append( m_Obj_CRTEnd ) );

		return Shell->Run( CommandLine );
	}

}}

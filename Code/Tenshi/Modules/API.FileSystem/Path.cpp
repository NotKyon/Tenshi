#include "_PCH.hpp"
#include "Path.hpp"

#ifdef _WIN32
# include <shellapi.h>
# include <ShlObj.h>
#endif

TE_FUNC const char *TE_CALL FS_AppDir()
{
	static bool bDidInit = false;
	static char szAppDir[ PATH_MAX + 1 ] = { '\0' };

	if( !bDidInit ) {
		bDidInit |= !!Ax::System::GetAppDir( szAppDir );
	}

	return szAppDir;
}
TE_FUNC const char *TE_CALL FS_HomeDir()
{
	static bool bDidInit = false;
	static Ax::String HomeDir;

#ifdef _WIN32
	if( !bDidInit ) {
		wchar_t wszHomeDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_PROFILE, NULL, 0, wszHomeDir );
		bDidInit |= HomeDir.AssignUTF16( wszHomeDir );
	}

	return HomeDir;
#else
# error TODO: Please implement this function.
#endif
}
TE_FUNC const char *TE_CALL FS_WinDir()
{
#ifdef _WIN32
	static bool bDidInit = false;
	static Ax::String WinDir;

	if( !bDidInit ) {
		wchar_t wszWinDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_WINDOWS, NULL, 0, wszWinDir );
		bDidInit |= WinDir.AssignUTF16( wszWinDir );
	}

	return WinDir;
#else
	return nullptr;
#endif
}
TE_FUNC const char *TE_CALL FS_MyDocDir()
{
	static bool bDidInit = false;
	static Ax::String DocDir;

	if( !bDidInit ) {
#ifdef _WIN32
		wchar_t wszDocDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_MYDOCUMENTS, NULL, 0, wszDocDir );
		bDidInit |= DocDir.AssignUTF16( wszDocDir );
#else
		DocDir.Assign( FS_HomeDir() );
		bDidInit |= DocDir.AppendPath( "Documents" );
#endif
	}

	return DocDir;
}
TE_FUNC const char *TE_CALL FS_MusicDir()
{
	static bool bDidInit = false;
	static Ax::String MusicDir;

	if( !bDidInit ) {
#ifdef _WIN32
		wchar_t wszMusicDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_MYMUSIC, NULL, 0, wszMusicDir );
		bDidInit |= MusicDir.AssignUTF16( wszMusicDir );
#else
		MusicDir.Assign( FS_HomeDir() );
		bDidInit |= MusicDir.AppendPath( "Music" );
#endif
	}

	return MusicDir;
}
TE_FUNC const char *TE_CALL FS_VideosDir()
{
	static bool bDidInit = false;
	static Ax::String VideoDir;

	if( !bDidInit ) {
#ifdef _WIN32
		wchar_t wszVideoDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_MYVIDEO, NULL, 0, wszVideoDir );
		bDidInit |= VideoDir.AssignUTF16( wszVideoDir );
#else
		VideoDir.Assign( FS_HomeDir() );
		bDidInit |= VideoDir.AppendPath( "Videos" );
#endif
	}

	return VideoDir;
}
TE_FUNC const char *TE_CALL FS_PicturesDir()
{
	static bool bDidInit = false;
	static Ax::String PicDir;

	if( !bDidInit ) {
#ifdef _WIN32
		wchar_t wszPicDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_MYPICTURES, NULL, 0, wszPicDir );
		bDidInit |= PicDir.AssignUTF16( wszPicDir );
#else
		PicDir.Assign( FS_HomeDir() );
		bDidInit |= PicDir.AppendPath( "Pictures" );
#endif
	}

	return PicDir;
}
TE_FUNC const char *TE_CALL FS_AppDataDir()
{
	static bool bDidInit = false;
	static Ax::String AppDataDir;

	if( !bDidInit ) {
#ifdef _WIN32
		wchar_t wszAppDataDir[ PATH_MAX + 1 ] = { L'\0' };
		SHGetFolderPathW( NULL, CSIDL_APPDATA, NULL, 0, wszAppDataDir );
		bDidInit |= AppDataDir.AssignUTF16( wszAppDataDir );
#else
		bDidInit |= AppDataDir.Assign( FS_HomeDir() );
#endif
	}

	return AppDataDir;
}
TE_FUNC const char *TE_CALL FS_TempDir()
{
	static bool bDidInit = false;
	static Ax::String TempDir;

	if( !bDidInit ) {
#ifdef _WIN32
		bDidInit |= TempDir.Assign( FS_AppDataDir() ) ? TempDir.AppendPath( "Temp" ) : false;
#else
# error TODO: Please implement this function.
#endif
	}

	return TempDir;
}

TE_FUNC char *TE_CALL FS_AbsolutePath( const char *pszRelPath )
{
	char szAbsPath[ PATH_MAX*4 + 1 ];

	if( !pszRelPath || !Ax::GetAbsolutePath( szAbsPath, pszRelPath ) ) {
		return nullptr;
	}

	return Tenshi::StrDup( szAbsPath );
}
TE_FUNC char *TE_CALL FS_AbsolutePathBase( const char *pszRelPath, const char *pszBase )
{
	AX_ASSERT_MSG( false, "Unimplemented" );
	return nullptr;
}
TE_FUNC char *TE_CALL FS_RelativePath( const char *pszAbsPath )
{
	char szRelPath[ PATH_MAX*4 + 1 ];
	char szCurPath[ PATH_MAX*4 + 1 ];

	if( !Ax::System::GetDir( szCurPath ) ) {
		return nullptr;
	}

	if( !pszAbsPath || !Ax::GetRelativePath( szRelPath, szCurPath, pszAbsPath ) ) {
		return nullptr;
	}

	return Tenshi::StrDup( szRelPath );
}
TE_FUNC char *TE_CALL FS_RelativePathBase( const char *pszAbsPath, const char *pszBase )
{
	char szRelPath[ PATH_MAX*4 + 1 ];

	if( !pszAbsPath || !pszBase || !Ax::GetRelativePath( szRelPath, pszBase, pszAbsPath ) ) {
		return nullptr;
	}

	return Tenshi::StrDup( szRelPath );
}

TE_FUNC int TE_CALL FS_PathIsAbsolute( const char *pszPath )
{
	if( !pszPath || *pszPath == '\0' ) {
		return 0;
	}

	return FS_PathHasRootname( pszPath );
}
TE_FUNC int TE_CALL FS_PathIsRelative( const char *pszPath )
{
	if( !pszPath || *pszPath == '\0' ) {
		return 0;
	}

	return !FS_PathHasRootname( pszPath );
}

static StrRef PathRootname( StrRef InPath )
{
#ifdef _WIN32
	if( InPath.Skip().StartsWith( ':' ) ) {
		return InPath.Left( 2 );
	}

	if( InPath.StartsWith( "\\\\" ) ) {
		// "Raw path"
		if( InPath.Skip(2).StartsWith( "?\\" ) ) {
			const StrRef x = InPath.Skip(4).Find( '\\' );
			if( x.StartsWith( "\\UNC\\" ) ) {
				const StrRef y = x.Skip(5).Find( '\\' );
				if( y ) {
					return StrRef( InPath.s, y.s );
				}
			} else if( !x ) {
				return InPath;
			}

			return StrRef( InPath.s, x.s );
		}

		// Devices namespace
		if( InPath.Skip(2).StartsWith( ".\\" ) ) {
			const StrRef x = InPath.Skip(4).Find( '\\' );
			if( x ) {
				return StrRef( InPath.s, x.s );
			}

			return InPath;
		}

		// Network path
		const StrRef x = InPath.Skip( 2 ).Find( '\\' );
		if( x ) {
			return StrRef( InPath.s, x.s );
		}

		return InPath;
	}
#else
	if( InPath.StartsWith( '/' ) ) {
		return InPath.Left( 1 );
	}
#endif

	return StrRef();
}
static StrRef PathRootdir( StrRef InPath )
{
	const StrRef Rootname = PathRootname( InPath );
	const StrRef Path = InPath.Skip( Rootname ? Rootname.Len() + 1 : 0 );

	const StrRef NextSep = Path.FindPathSep();
	if( NextSep ) {
		return StrRef( InPath.s, NextSep.s );
	}

	return Path;
}
static StrRef PathDirectory( StrRef InPath )
{
	const StrRef Last = InPath.FindLastPathSep();
	if( !Last ) {
		return StrRef();
	}

	return InPath.Drop( Last.Len() - 1 );
}
static StrRef PathFilename( StrRef InPath )
{
	const StrRef Dir = PathDirectory( InPath );
	if( !Dir ) {
		return InPath;
	}

	return InPath.Skip( Dir.Len() );
}
static StrRef PathBasename( StrRef InPath )
{
	const StrRef Filename = PathFilename( InPath );
	const StrRef Dot = Filename.FindLast( '.' );

	return Dot ? StrRef( Filename.s, Dot.s ) : Filename;
}
static StrRef PathExtension( StrRef InPath )
{
	return PathFilename( InPath ).FindLast( '.' );
}
static StrRef ParentPath( StrRef InPath )
{
	const StrRef MainDir = PathDirectory( InPath );
	if( !MainDir ) { return StrRef(); }

	const StrRef LastSep = MainDir.Drop().FindLastPathSep();
	if( !LastSep ) { return StrRef(); }

	return StrRef( InPath.s, LastSep.s );
}

TE_FUNC char *TE_CALL FS_GetPathRootname( const char *pszPath )
{
	return PathRootname( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetPathRootdir( const char *pszPath )
{
	return PathRootdir( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetPathDirectory( const char *pszPath )
{
	return PathDirectory( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetPathFilename( const char *pszPath )
{
	return PathFilename( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetPathBasename( const char *pszPath )
{
	return PathBasename( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetPathExtension( const char *pszPath )
{
	return PathExtension( pszPath ).Dup();
}
TE_FUNC char *TE_CALL FS_GetParentPath( const char *pszPath )
{
	return ParentPath( pszPath ).Dup();
}

TE_FUNC int TE_CALL FS_PathHasRootname( const char *pszPath )
{
	return +!!PathRootname( pszPath );
}
TE_FUNC int TE_CALL FS_PathHasRootdir( const char *pszPath )
{
	return +!!PathRootdir( pszPath );
}
TE_FUNC int TE_CALL FS_PathHasDirectory( const char *pszPath )
{
	return +!!PathDirectory( pszPath );
}
TE_FUNC int TE_CALL FS_PathHasFilename( const char *pszPath )
{
	return +!!PathFilename( pszPath );
}
TE_FUNC int TE_CALL FS_PathHasBasename( const char *pszPath )
{
	return +!!PathBasename( pszPath );
}
TE_FUNC int TE_CALL FS_PathHasExtension( const char *pszPath )
{
	return +!!PathExtension( pszPath );
}

TE_FUNC char *TE_CALL FS_SetPathFilename( const char *pszPath, const char *pszFilename )
{
	const StrRef InPath = StrRef( pszPath );
	const StrRef PathDir = PathDirectory( InPath );

	const StrRef InName = StrRef( pszFilename );

	char *const p = Tenshi::MakeString( PathDir.Len() + InName.Len() );
	if( !p ) {
		return nullptr;
	}

	if( PathDir ) {
		memcpy( ( void * )p, ( const void * )PathDir.s, PathDir.Len() );
	}
	if( InName ) {
		memcpy( ( void * )( p + PathDir.Len() ), ( const void * )InName.s, InName.Len() );
	}
	p[ PathDir.Len() + InName.Len() ] = '\0';

	return p;
}
TE_FUNC char *TE_CALL FS_SetPathBasename( const char *pszPath, const char *pszBasename )
{
	const StrRef InPath = StrRef( pszPath );
	const StrRef InBase = StrRef( pszBasename );

	const StrRef PathDir = PathDirectory( InPath );
	const StrRef PathExt = StrRef( PathDir ? PathDir.e : InPath.s, InPath.e ).FindLast( '.' );

	const TenshiUIntPtr_t n = PathDir.Len() + InBase.Len() + PathExt.Len();
	char *const p = Tenshi::MakeString( n );
	if( !p ) {
		return nullptr;
	}

	if( PathDir ) {
		memcpy( ( void * )p, ( const void * )PathDir.s, PathDir.Len() );
	}
	if( InBase ) {
		memcpy( ( void * )( p + PathDir.Len() ), ( const void * )InBase.s, InBase.Len() );
	}
	if( PathExt ) {
		memcpy( ( void * )( p + PathDir.Len() + InBase.Len() ), ( const void * )PathExt.s, PathExt.Len() );
	}
	p[ n ] = '\0';

	return p;
}
TE_FUNC char *TE_CALL FS_SetPathExtension( const char *pszPath, const char *pszExtension )
{
	const StrRef InPath = StrRef( pszPath );
	const StrRef InExt = StrRef( pszExtension );

	const StrRef PathExt = PathExtension( InPath );
	const StrRef AllPath = PathExt ? StrRef( InPath.s, PathExt.s ) : InPath;

	char *const p = Tenshi::MakeString( AllPath.Len() + InExt.Len() );
	if( !p ) {
		return nullptr;
	}

	if( AllPath ) {
		memcpy( ( void * )p, ( const void * )AllPath.s, AllPath.Len() );
	}
	if( InExt ) {
		memcpy( ( void * )( p + AllPath.Len() ), ( const void * )InExt.s, InExt.Len() );
	}
	p[ AllPath.Len() + InExt.Len() ] = '\0';

	return p;
}

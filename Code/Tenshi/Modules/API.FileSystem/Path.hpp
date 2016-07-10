#pragma once

#include "../APICommon.hpp"

TE_FUNC const char *TE_CALL FS_AppDir();
TE_FUNC const char *TE_CALL FS_HomeDir();
TE_FUNC const char *TE_CALL FS_WinDir();
TE_FUNC const char *TE_CALL FS_MyDocDir();
TE_FUNC const char *TE_CALL FS_MusicDir();
TE_FUNC const char *TE_CALL FS_VideosDir();
TE_FUNC const char *TE_CALL FS_PicturesDir();
TE_FUNC const char *TE_CALL FS_AppDataDir();
TE_FUNC const char *TE_CALL FS_TempDir();

TE_FUNC char *TE_CALL FS_AbsolutePath( const char *pszRelPath );
TE_FUNC char *TE_CALL FS_AbsolutePathBase( const char *pszRelPath, const char *pszBase );
TE_FUNC char *TE_CALL FS_RelativePath( const char *pszAbsPath );
TE_FUNC char *TE_CALL FS_RelativePathBase( const char *pszAbsPath, const char *pszBase );

TE_FUNC int TE_CALL FS_PathIsAbsolute( const char *pszPath );
TE_FUNC int TE_CALL FS_PathIsRelative( const char *pszPath );

TE_FUNC char *TE_CALL FS_GetPathRootname( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetPathRootdir( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetPathDirectory( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetPathFilename( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetPathBasename( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetPathExtension( const char *pszPath );
TE_FUNC char *TE_CALL FS_GetParentPath( const char *pszPath );

TE_FUNC int TE_CALL FS_PathHasRootname( const char *pszPath );
TE_FUNC int TE_CALL FS_PathHasRootdir( const char *pszPath );
TE_FUNC int TE_CALL FS_PathHasDirectory( const char *pszPath );
TE_FUNC int TE_CALL FS_PathHasFilename( const char *pszPath );
TE_FUNC int TE_CALL FS_PathHasBasename( const char *pszPath );
TE_FUNC int TE_CALL FS_PathHasExtension( const char *pszPath );

TE_FUNC char *TE_CALL FS_SetPathFilename( const char *pszPath, const char *pszFilename );
TE_FUNC char *TE_CALL FS_SetPathBasename( const char *pszPath, const char *pszBasename );
TE_FUNC char *TE_CALL FS_SetPathExtension( const char *pszPath, const char *pszExtension );

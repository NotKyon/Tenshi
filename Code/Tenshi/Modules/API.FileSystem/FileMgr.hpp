#pragma once

#include "../APICommon.hpp"

TE_FUNC void TE_CALL FS_SetDir( const char *pszDirname );
TE_FUNC bool TE_CALL FS_SetDirE( const char *pszDirname, ErrorCode &EC );

TE_FUNC void TE_CALL FS_EnterDir( const char *pszDirname );
TE_FUNC bool TE_CALL FS_EnterDirE( const char *pszDirname, ErrorCode &EC );

TE_FUNC void TE_CALL FS_LeaveDir();
TE_FUNC bool TE_CALL FS_LeaveDirE( ErrorCode &EC );

TE_FUNC char *TE_CALL FS_GetDir();

TE_FUNC Ax::uint64 TE_CALL FS_GetFileUniqueID( const char *pszPath );

TE_FUNC bool TE_CALL FS_FileExist( const char *pszPath );
TE_FUNC bool TE_CALL FS_PathExist( const char *pszPath );

TE_FUNC bool TE_CALL FS_IsRegularFile( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsDirectory( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsDevice( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsCharacterDevice( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsBlockDevice( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsPipe( const char *pszPath );
TE_FUNC bool TE_CALL FS_IsSocket( const char *pszPath );

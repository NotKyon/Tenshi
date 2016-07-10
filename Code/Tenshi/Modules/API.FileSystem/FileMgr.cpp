#include "_PCH.hpp"
#include "FileMgr.hpp"

void LogError( ErrorCode &EC );

TE_FUNC void TE_CALL FS_SetDir( const char *pszDirname )
{
	if( !Ax::System::SetDir( pszDirname ) ) {
		SystemError( ErrorCode().SetErrno() );
	}
}
TE_FUNC bool TE_CALL FS_SetDirE( const char *pszDirname, ErrorCode &EC )
{
	if( !Ax::System::SetDir( pszDirname ) ) {
		EC.SetErrno();
		return false;
	}

	EC.Value = 0;
	return true;
}

TE_FUNC void TE_CALL FS_EnterDir( const char *pszDirname )
{
	if( !Ax::System::PushDir( pszDirname ) ) {
		SystemError( ErrorCode().SetErrno() );
	}
}
TE_FUNC bool TE_CALL FS_EnterDirE( const char *pszDirname, ErrorCode &EC )
{
	if( !Ax::System::PushDir( pszDirname ) ) {
		EC.SetErrno();
		return false;
	}

	EC.Value = 0;
	return true;
}

TE_FUNC void TE_CALL FS_LeaveDir()
{
	errno = 0;
	Ax::System::PopDir();
	if( errno ) {
		SystemError( ErrorCode().SetErrno() );
	}
}
TE_FUNC bool TE_CALL FS_LeaveDirE( ErrorCode &EC )
{
	errno = 0;
	Ax::System::PopDir();
	if( errno ) {
		EC.SetErrno();
		return false;
	}

	return true;
}

TE_FUNC char *TE_CALL FS_GetDir()
{
	char szCurDir[ PATH_MAX + 1 ];

	if( !Ax::System::GetDir( szCurDir ) ) {
		return nullptr;
	}

	return Tenshi::StrDup( szCurDir );
}

TE_FUNC Ax::uint64 TE_CALL FS_GetFileUniqueID( const char *pszPath )
{
	return Ax::System::GetUniqueFileId( pszPath );
}

TE_FUNC bool TE_CALL FS_FileExist( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0 && ( s.st_mode & ( _S_IFREG | _S_IFIFO ) ) != 0;
}
TE_FUNC bool TE_CALL FS_PathExist( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0;
}

TE_FUNC bool TE_CALL FS_IsRegularFile( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0 && ( s.st_mode & _S_IFREG ) != 0;
}
TE_FUNC bool TE_CALL FS_IsDirectory( const char *pszPath )
{
	struct stat s;

	return stat( pszPath, &s ) == 0 && ( s.st_mode & _S_IFDIR ) != 0;
}
TE_FUNC bool TE_CALL FS_IsDevice( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0 && ( s.st_mode & _S_IFCHR ) != 0;
}
TE_FUNC bool TE_CALL FS_IsCharacterDevice( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0 && ( s.st_mode & _S_IFCHR ) != 0;
}
TE_FUNC bool TE_CALL FS_IsBlockDevice( const char *pszPath )
{
	/* TODO: block devices */
	( void )pszPath;
	UNIMPLEMENTED();
	return false;
}
TE_FUNC bool TE_CALL FS_IsPipe( const char *pszPath )
{
	struct _stat64i32 s;
	wchar_t wszPath[ PATH_MAX + 1 ];

	return _wstat( Ax::ToWStr( wszPath, pszPath ), &s ) == 0 && ( s.st_mode & _S_IFIFO ) != 0;
}
TE_FUNC bool TE_CALL FS_IsSocket( const char *pszPath )
{
	/* TODO: sockets */
	( void )pszPath;
	UNIMPLEMENTED();
	return false;
}

#if 0
MAKE FILE Filename$
MAKE FILE( Filename$, Ref ErrorCode ) -> Boolean
Creates a blank file.

DELETE FILE Filename$
Removes the given file.

COPY FILE SourceFilename$, DestinationFilename$ [, Flags]
COPY FILE( SourceFilename$, DestinationFilename$ [, Flags], Ref ErrorCode ) -> Boolean
Copy SourceFilename$ to DestinationFilename$.

MOVE FILE SourceFilename$, DestinationFilename$ [, Flags]
MOVE FILE( SourceFilename$, DestinationFilename$ [, Flags], Ref ErrorCode ) -> Boolean
Move SourceFilename$ to DestinationFilename$.

RENAME FILE SourceFilename$, DestinationFilename$ [, Flags]
RENAME FILE( SourceFilename$, DestinationFilename$ [, Flags ], Ref ErrorCode ) -> Boolean
Rename SourceFilename$ to DestinationFilename$. This is a move operation with
slightly different semantics:

* If DestinationFilename$ is a relative path, then it is considered relative to
` SourceFilename$.
#endif

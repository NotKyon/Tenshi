#pragma once

#include "../APICommon.hpp"

enum class EOpenDir
{
	Directly,
	Recursively
};

enum EFileType : Ax::int32
{
	kFT_Invalid = -1,
	kFT_File,
	kFT_Directory,
	kFT_Device,
	kFT_Special
};

class CDirectory
{
public:
	static bool Init();
	static void Fini();

	CDirectory();
	~CDirectory();

	bool Open( const StrRef &InPath, EOpenDir Mode, ErrorCode &EC );
	void Close();

	bool Read( ErrorCode &EC );
	bool IsEnd() const;

	const char *EntryName() const;
	Ax::uint64 EntrySize() const;
	EFileType EntryType() const;
	Ax::uint64 EntryCreationTime() const;
	Ax::uint64 EntryAccessedTime() const;
	Ax::uint64 EntryModifiedTime() const;

	const char *EntryCreationDateStr();
	const char *EntryAccessedDateStr();
	const char *EntryModifiedDateStr();
	const char *EntryCreationTimeStr();
	const char *EntryAccessedTimeStr();
	const char *EntryModifiedTimeStr();

private:
	Ax::String						m_OpenPath;
	EOpenDir						m_OpenMode;

	CDirectory *					m_pSubdir;

#ifdef _WIN32
	HANDLE							m_hFind;
	WIN32_FIND_DATAW				m_FindData;
	Ax::String						m_FindName;
	bool							m_bIsFirst;
	bool							m_bIsEnd;
#elif defined(__APPLE__)
#else
# error TODO: Cross platform directory enumeration
#endif
	char							m_szDateTime[ 64 ];


	const char *SetDateStr( Ax::uint64 uFileTime );
	const char *SetTimeStr( Ax::uint64 uFileTime );
};

//----------------------------------------------------------------------------//

TE_FUNC void TE_CALL FS_MakeDirectory( const char *pszDirname );
TE_FUNC bool TE_CALL FS_MakeDirectoryEC( const char *pszDirname, ErrorCode &EC );

TE_FUNC void TE_CALL FS_DeleteDirectory( const char *pszDirname );
TE_FUNC bool TE_CALL FS_DeleteDirectoryEC( const char *pszDirname, unsigned &EC );

TE_FUNC void TE_CALL FS_OpenDirectory( TenshiIndex_t DirNumber, const char *pszDirname );
TE_FUNC bool TE_CALL FS_OpenDirectoryEC( TenshiIndex_t DirNumber, const char *pszDirname, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryAlloc( const char *pszDirname );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryAllocEC( const char *pszDirname, ErrorCode &EC );

TE_FUNC void TE_CALL FS_OpenDirectoryTree( TenshiIndex_t DirNumber, const char *pszDirname );
TE_FUNC bool TE_CALL FS_OpenDirectoryTreeEC( TenshiIndex_t DirNumber, const char *pszDirname, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryTreeAlloc( const char *pszDirname );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryTreeAllocEC( const char *pszDirname, ErrorCode &EC );

TE_FUNC TenshiIndex_t TE_CALL FS_CloseDirectory( TenshiIndex_t DirNumber );

TE_FUNC bool TE_CALL FS_IsDirectoryOpen( TenshiIndex_t DirNumber );

TE_FUNC void TE_CALL FS_SetDirectoryFilter( TenshiIndex_t DirNumber, const char *pszExtension );
TE_FUNC const char *TE_CALL FS_GetDirectoryFilter( TenshiIndex_t DirNumber );

TE_FUNC void TE_CALL FS_ResetDirectory( TenshiIndex_t DirNumber );

TE_FUNC bool TE_CALL FS_DirectoryEnd( TenshiIndex_t DirNumber );
TE_FUNC bool TE_CALL FS_ReadDirectory( TenshiIndex_t DirNumber );
TE_FUNC bool TE_CALL FS_ReadDirectoryEC( TenshiIndex_t DirNumber, ErrorCode &EC );

TE_FUNC const char *TE_CALL FS_GetEntryFileName( TenshiIndex_t DirNumber );
TE_FUNC EFileType TE_CALL FS_GetEntryFileType( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileDateStr( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileTimeStr( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileCreationDateStr( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileCreationTimeStr( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileAccessedDateStr( TenshiIndex_t DirNumber );
TE_FUNC const char *TE_CALL FS_GetEntryFileAccessedTimeStr( TenshiIndex_t DirNumber );

TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileModifiedTimestamp( TenshiIndex_t DirNumber );
TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileCreationTimestamp( TenshiIndex_t DirNumber );
TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileAccessedTimestamp( TenshiIndex_t DirNumber );

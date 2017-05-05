#include "_PCH.hpp"
#include "Directory.hpp"

static Tenshi::THandler<CDirectory> g_Directories;

bool CDirectory::Init()
{
	return g_Directories.Init();
}
void CDirectory::Fini()
{
}

CDirectory::CDirectory()
: m_OpenPath()
, m_OpenMode( EOpenDir::Directly )
, m_pSubdir( nullptr )
#ifdef _WIN32
, m_hFind( INVALID_HANDLE_VALUE )
, m_FindName()
, m_bIsFirst( false )
, m_bIsEnd( false )
#endif
{
}
CDirectory::~CDirectory()
{
	Close();
}

bool CDirectory::Open( const StrRef &InPath, EOpenDir Mode, ErrorCode &EC )
{
	AX_EXPECT_MSG( m_OpenPath.Assign( InPath.s, InPath.e ), "Out of memory" );
	m_OpenMode = Mode;

#ifdef _WIN32
	Ax::TArray< Ax::uint16 > RealPathUTF16;
	Ax::String RealPath;

	AX_EXPECT_MSG( RealPath.Assign( InPath.s, InPath.e ), "Out of memory" );
	AX_EXPECT_MSG( RealPath.AppendPath( "*" ), "Out of memory" );

	fprintf( stderr, "CDirectory::Open: \"%s\"\n", RealPath.CString() );

	RealPathUTF16 = RealPath.AsUTF16();
	AX_EXPECT_MSG( !RealPathUTF16.IsEmpty(), "Out of memory" );

	m_hFind = FindFirstFileW( ( LPCWSTR )RealPathUTF16.Pointer(), &m_FindData );
	if( m_hFind == INVALID_HANDLE_VALUE ) {
		DWORD dwLastErr = GetLastError();
		if( dwLastErr == ERROR_NO_MORE_FILES ) {
			dwLastErr = 0;
		}
		EC.SetSystemError_MSWin( dwLastErr );
		return false;
	}

	( void )m_FindName.Reserve( 128 );
	m_bIsFirst = true;
	m_bIsEnd = false;
#endif

	return true;
}
void CDirectory::Close()
{
#ifdef _WIN32
	if( m_hFind == INVALID_HANDLE_VALUE ) {
		return;
	}

	( void )FindClose( m_hFind );
	m_hFind = INVALID_HANDLE_VALUE;
#endif
}

bool CDirectory::Read( ErrorCode &EC )
{
#ifdef _WIN32
	m_FindName.Clear();
	if( !m_bIsFirst ) {
		memset( &m_FindData, 0, sizeof( m_FindData ) );
	}
#endif

	if( m_pSubdir != nullptr ) {
		if( m_pSubdir->Read( EC ) ) {
#ifdef _WIN32
			AX_EXPECT_MSG( m_FindName.AssignUTF16( m_FindData.cFileName ), "Out of memory" );
			AX_EXPECT_MSG( m_FindName.AppendPath( m_pSubdir->EntryName() ), "Out of memory" );
#endif
			return true;
		}

		if( EC ) {
			return false;
		}

		delete m_pSubdir;
		m_pSubdir = nullptr;
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	if( !m_bIsFirst && !FindNextFileW( m_hFind, &m_FindData ) ) {
		DWORD dwLastErr = GetLastError();
		if( dwLastErr == ERROR_NO_MORE_FILES ) {
			dwLastErr = 0;
			m_bIsEnd = true;
		}
		EC.SetSystemError_MSWin( dwLastErr );
		return false;
	}

	m_bIsFirst = false;

	AX_EXPECT_MSG( m_FindName.Assign( m_OpenPath ), "Out of memory" );
	AX_EXPECT_MSG( m_FindName.AppendDirectorySeparator(), "Out of memory" );
	AX_EXPECT_MSG( m_FindName.AppendUTF16( m_FindData.cFileName ), "Out of memory" );

	if( ( m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && ( m_OpenMode == EOpenDir::Recursively ) &&
	wcscmp( m_FindData.cFileName, L"." ) != 0 && wcscmp( m_FindData.cFileName, L".." ) != 0 ) {
		m_pSubdir = new CDirectory();
		if( !m_pSubdir ) {
			return false;
		}

		if( !m_pSubdir->Open( Tenshi::StrRef( m_FindName.CString(), m_FindName.Len() ), EOpenDir::Recursively, EC ) ) {
			delete m_pSubdir;
			m_pSubdir = nullptr;
			return false;
		}

		return Read( EC );
	}

	return true;
#else
	return false;
#endif
}
bool CDirectory::IsEnd() const
{
#ifdef _WIN32
	return m_bIsEnd;
#else
	return true;
#endif
}

inline Ax::uint64 Make64( Ax::uint32 uHigh, Ax::uint32 uLow )
{
	return ( Ax::uint64( uHigh )<<32 ) | Ax::uint64( uLow );
}
#ifdef _WIN32
static inline Ax::uint64 Make64( const FILETIME &ft )
{
	return Make64( ft.dwHighDateTime, ft.dwLowDateTime );
}
#endif

const char *CDirectory::EntryName() const
{
#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	return m_FindName;
#elif defined(__APPLE__)
	return nullptr;
#endif
}
Ax::uint64 CDirectory::EntrySize() const
{
	if( m_pSubdir != nullptr ) {
		return m_pSubdir->EntrySize();
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	return Make64( m_FindData.nFileSizeHigh, m_FindData.nFileSizeLow );
#elif defined(__APPLE__)
	return 0;
#endif
}
EFileType CDirectory::EntryType() const
{
	if( m_pSubdir != nullptr ) {
		return m_pSubdir->EntryType();
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	if( m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
		return kFT_Directory;
	}

	if( m_FindData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ) {
		return kFT_File;
	}

	if( m_FindData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ) {
		return kFT_Device;
	}

	return kFT_Special;
#elif defined(__APPLE__)
	return kFT_Invalid;
#endif
}
Ax::uint64 CDirectory::EntryCreationTime() const
{
	if( m_pSubdir != nullptr ) {
		return m_pSubdir->EntryCreationTime();
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	return Make64( m_FindData.ftCreationTime );
#elif defined(__APPLE__)
	return 0;
#endif
}
Ax::uint64 CDirectory::EntryAccessedTime() const
{
	if( m_pSubdir != nullptr ) {
		return m_pSubdir->EntryAccessedTime();
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	return Make64( m_FindData.ftLastAccessTime );
#elif defined(__APPLE__)
	return 0;
#endif
}
Ax::uint64 CDirectory::EntryModifiedTime() const
{
	if( m_pSubdir != nullptr ) {
		return m_pSubdir->EntryModifiedTime();
	}

#ifdef _WIN32
	AX_ASSERT( m_hFind != INVALID_HANDLE_VALUE );

	return Make64( m_FindData.ftLastWriteTime );
#elif defined(__APPLE__)
	return 0;
#endif
}

const char *CDirectory::EntryCreationDateStr()
{
	return SetDateStr( EntryCreationTime() );
}
const char *CDirectory::EntryAccessedDateStr()
{
	return SetDateStr( EntryAccessedTime() );
}
const char *CDirectory::EntryModifiedDateStr()
{
	return SetDateStr( EntryModifiedTime() );
}
const char *CDirectory::EntryCreationTimeStr()
{
	return SetTimeStr( EntryCreationTime() );
}
const char *CDirectory::EntryAccessedTimeStr()
{
	return SetTimeStr( EntryAccessedTime() );
}
const char *CDirectory::EntryModifiedTimeStr()
{
	return SetTimeStr( EntryModifiedTime() );
}

const char *CDirectory::SetDateStr( Ax::uint64 uFileTime )
{
#ifdef _WIN32
	FILETIME ft; ft.dwHighDateTime = Ax::uint32( uFileTime>>32 ); ft.dwLowDateTime = Ax::uint32( uFileTime );
	FILETIME lft;
	SYSTEMTIME st;

	if( !FileTimeToLocalFileTime( &ft, &lft ) || !FileTimeToSystemTime( &ft, &st ) ) {
		return nullptr;
	}

# ifdef _MSC_VER
	sprintf_s( m_szDateTime, "%.4u-%.2u-%.2u",
		( unsigned )st.wYear, ( unsigned )st.wMonth, ( unsigned )st.wDay );
# else
	snprintf( m_szDateTime, sizeof( m_szDateTime ) - 1, "%.4u-%.2u-%.2u",
		( unsigned )st.wYear, ( unsigned )st.wMonth, ( unsigned )st.wDay );
	m_szDateTime[ sizeof( m_szDateTime ) - 1 ] = '\0';
# endif
#endif

	return m_szDateTime;
}
const char *CDirectory::SetTimeStr( Ax::uint64 uFileTime )
{
#ifdef _WIN32
	FILETIME ft; ft.dwHighDateTime = Ax::uint32( uFileTime>>32 ); ft.dwLowDateTime = Ax::uint32( uFileTime );
	FILETIME lft;
	SYSTEMTIME st;

	if( !FileTimeToLocalFileTime( &ft, &lft ) || !FileTimeToSystemTime( &lft, &st ) ) {
		return nullptr;
	}

# ifdef _MSC_VER
	sprintf_s( m_szDateTime, "%.2u:%.2u:%.2u",
		( unsigned )st.wHour, ( unsigned )st.wMinute, ( unsigned )st.wSecond );
# else
	snprintf( m_szDateTime, sizeof( m_szDateTime ) - 1, "%.2u:%.2u:%.2u",
		( unsigned )st.wHour, ( unsigned )st.wMinute, ( unsigned )st.wSecond );
	m_szDateTime[ sizeof( m_szDateTime ) - 1 ] = '\0';
# endif
#endif

	return m_szDateTime;
}

//----------------------------------------------------------------------------//

namespace {
CDirectory *Directory( TenshiIndex_t uIndex )
{
	CDirectory *const pDir = g_Directories.Unwrap( uIndex );
	TE_CHECK( TE_ERR_FS_DIRUNUSED, pDir != nullptr );

	return pDir;
}
TenshiIndex_t OpenDir( TenshiIndex_t DirNumber, const char *pszDirname, EOpenDir Mode, ErrorCode *pEC )
{
	TenshiIndex_t uDir = g_Directories.Make( DirNumber );
	if( !uDir ) {
		TE_TRACE( "OpenDir( DirNumber=%u, pszDirname=\"%s\", Mode=%u, pEC=%p )@Make failed",
			DirNumber, pszDirname, ( unsigned int )Mode, ( const void * )pEC );

		TE_CHECK( TE_ERR_BADALLOC, DirNumber != 0 );

		Tenshi::RuntimeError( TE_ERR_EXISTS );
	}

	CDirectory *const pDir = g_Directories.Unwrap( uDir );
	AX_ASSERT_NOT_NULL( pDir );

	ErrorCode LocalEC;
	if( !pDir->Open( pszDirname, Mode, LocalEC ) ) {
		TE_TRACE( "OpenDir( DirNumber=%u, pszDirname=\"%s\", Mode=%u, pEC=%p )@Open failed",
			DirNumber, pszDirname, ( unsigned int )Mode, ( const void * )pEC );

		if( !pEC ) {
			SystemError( LocalEC );
		}

		*pEC = LocalEC;
		g_Directories.Free( uDir );
		return 0;
	}

	if( !!pEC ) {
		pEC->Value = 0;
	}
	return uDir;
}
} // anonymous namespace

TE_FUNC void TE_CALL FS_MakeDirectory( const char *pszDirname )
{
	errno = 0;
	Ax::System::MakeDirs( pszDirname );
	if( errno ) {
		SystemError( ErrorCode().SetErrno() );
	}
}
TE_FUNC bool TE_CALL FS_MakeDirectoryEC( const char *pszDirname, ErrorCode &EC )
{
	errno = 0;
	Ax::System::MakeDirs( pszDirname );
	if( errno ) {
		TE_TRACE( "Ax::System::MakeDirs() failed for \"%s\"", pszDirname );
		EC.SetErrno();
		return false;
	}

	return true;
}

TE_FUNC void TE_CALL FS_DeleteDirectory( const char *pszDirname )
{
	( void )pszDirname;
	UNIMPLEMENTED();
}
TE_FUNC bool TE_CALL FS_DeleteDirectoryEC( const char *pszDirname, unsigned &EC )
{
	( void )pszDirname;
	( void )EC;
	UNIMPLEMENTED();
	return false;
}

TE_FUNC void TE_CALL FS_OpenDirectory( TenshiIndex_t DirNumber, const char *pszDirname )
{
	TE_CHECK( TE_ERR_INVALID, DirNumber != 0 );
	OpenDir( DirNumber, pszDirname, EOpenDir::Directly, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenDirectoryEC( TenshiIndex_t DirNumber, const char *pszDirname, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, DirNumber != 0 );
	return !!OpenDir( DirNumber, pszDirname, EOpenDir::Directly, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryAlloc( const char *pszDirname )
{
	return OpenDir( 0, pszDirname, EOpenDir::Directly, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryAllocEC( const char *pszDirname, ErrorCode &EC )
{
	return OpenDir( 0, pszDirname, EOpenDir::Directly, &EC );
}

TE_FUNC void TE_CALL FS_OpenDirectoryTree( TenshiIndex_t DirNumber, const char *pszDirname )
{
	TE_CHECK( TE_ERR_INVALID, DirNumber != 0 );
	OpenDir( DirNumber, pszDirname, EOpenDir::Recursively, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenDirectoryTreeEC( TenshiIndex_t DirNumber, const char *pszDirname, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, DirNumber != 0 );
	return !!OpenDir( DirNumber, pszDirname, EOpenDir::Recursively, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryTreeAlloc( const char *pszDirname )
{
	return OpenDir( 0, pszDirname, EOpenDir::Recursively, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenDirectoryTreeAllocEC( const char *pszDirname, ErrorCode &EC )
{
	return OpenDir( 0, pszDirname, EOpenDir::Recursively, &EC );
}

TE_FUNC TenshiIndex_t TE_CALL FS_CloseDirectory( TenshiIndex_t DirNumber )
{
	g_Directories.Free( DirNumber );
	return 0;
}

TE_FUNC bool TE_CALL FS_IsDirectoryOpen( TenshiIndex_t DirNumber )
{
	return g_Directories.Exists( DirNumber );
}

TE_FUNC void TE_CALL FS_SetDirectoryFilter( TenshiIndex_t DirNumber, const char *pszExtension )
{
	( void )DirNumber;
	( void )pszExtension;
	UNIMPLEMENTED();
}
TE_FUNC const char *TE_CALL FS_GetDirectoryFilter( TenshiIndex_t DirNumber )
{
	( void )DirNumber;
	UNIMPLEMENTED();
	return nullptr;
}

TE_FUNC void TE_CALL FS_ResetDirectory( TenshiIndex_t DirNumber )
{
	( void )DirNumber;
	UNIMPLEMENTED();
}

TE_FUNC bool TE_CALL FS_DirectoryEnd( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->IsEnd();
}
TE_FUNC bool TE_CALL FS_ReadDirectory( TenshiIndex_t DirNumber )
{
	ErrorCode EC;

	if( !Directory( DirNumber )->Read( EC ) ) {
		if( EC ) {
			SystemError( EC );
		}

		return false;
	}

	return true;
}
TE_FUNC bool TE_CALL FS_ReadDirectoryEC( TenshiIndex_t DirNumber, ErrorCode &EC )
{
	return Directory( DirNumber )->Read( EC );
}

TE_FUNC const char *TE_CALL FS_GetEntryFileName( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryName();
}
TE_FUNC EFileType TE_CALL FS_GetEntryFileType( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryType();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileDateStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryModifiedDateStr();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileTimeStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryModifiedTimeStr();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileCreationDateStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryCreationDateStr();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileCreationTimeStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryCreationTimeStr();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileAccessedDateStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryAccessedDateStr();
}
TE_FUNC const char *TE_CALL FS_GetEntryFileAccessedTimeStr( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryAccessedTimeStr();
}

TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileModifiedTimestamp( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryModifiedTime();
}
TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileCreationTimestamp( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryCreationTime();
}
TE_FUNC Ax::uint64 TE_CALL FS_GetEntryFileAccessedTimestamp( TenshiIndex_t DirNumber )
{
	return Directory( DirNumber )->EntryAccessedTime();
}

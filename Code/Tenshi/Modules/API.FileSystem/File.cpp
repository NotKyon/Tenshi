#include "_PCH.hpp"
#include "File.hpp"

static Tenshi::THandler<CFile>		g_Files;

bool CFile::Init()
{
	return g_Files.Init();
}
void CFile::Fini()
{
}

CFile::CFile()
#ifdef _WIN32
: m_hFile( INVALID_HANDLE_VALUE )
#endif
{
}
CFile::~CFile()
{
	Close();
}

#ifdef _WIN32
inline DWORD GetDesiredAccess( Ax::uint32 M )
{
	return ( CanRead( M ) ? GENERIC_READ : 0 ) | ( CanWrite( M ) ? GENERIC_WRITE : 0 );
}
inline DWORD GetShareMode( Ax::uint32 M )
{
	return
		( SharingReads( M ) ? FILE_SHARE_READ : 0 ) |
		( SharingWrites( M ) ? FILE_SHARE_WRITE : 0 ) |
		( ( M & OPENF_SHARE_DELETE ) ? FILE_SHARE_DELETE : 0 );
}
inline DWORD GetCreationDisposition( Ax::uint32 M )
{
	switch( GetEffectiveDisposition( M ) )
	{
	case OPENF_CREATE_NEW:
		return CREATE_NEW;
	case OPENF_CREATE_ALWAYS:
		return CREATE_ALWAYS;
	case OPENF_OPEN_EXISTING:
		return OPEN_EXISTING;
	case OPENF_OPEN_ALWAYS:
		return OPEN_ALWAYS;
	case OPENF_TRUNCATE_EXISTING:
		return TRUNCATE_EXISTING;
	}

	return 0;
}
inline DWORD GetFlagsAndAttributes( Ax::uint32 M )
{
	DWORD r = 0;

	if( M & OPENF_DELETE_ON_CLOSE ) {
		r |= FILE_FLAG_DELETE_ON_CLOSE;
	}
	if( M & OPENF_NO_BUFFERING ) {
		r |= FILE_FLAG_NO_BUFFERING;
	}
	if( M & OPENF_SEQUENTIAL_ACCESS ) {
		r |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	if( M & OPENF_RANDOM_ACCESS ) {
		r |= FILE_FLAG_RANDOM_ACCESS;
	}
	if( M & OPENF_ASYNC ) {
		r |= FILE_FLAG_OVERLAPPED;
	}
	if( M & OPENF_WRITE_THROUGH ) {
		r |= FILE_FLAG_WRITE_THROUGH;
	}

	if( M & OPENF_HIDDEN ) {
		r |= FILE_ATTRIBUTE_HIDDEN;
	}
	if( M & OPENF_TEMPORARY ) {
		r |= FILE_ATTRIBUTE_TEMPORARY;
	} else {
		r |= FILE_ATTRIBUTE_NORMAL;
	}

	return r;
}
#endif

bool CFile::Open( const char *pszPath, Ax::uint32 Mode, ErrorCode &EC )
{
#ifdef _WIN32
	wchar_t wszPath[ PATH_MAX + 1 ];

	if( !Ax::ToWStr( wszPath, pszPath ) ) {
		EC.SetSystemError_MSWin( ERROR_INVALID_PARAMETER );
		return false;
	}

	wszPath[ PATH_MAX ] = L'\0';

	const DWORD dwDesiredAccess = GetDesiredAccess( Mode );
	const DWORD dwShareMode = GetShareMode( Mode );
	const DWORD dwCreationDisposition = GetCreationDisposition( Mode );
	const DWORD dwFlagsAndAttributes = GetFlagsAndAttributes( Mode );

	m_hFile = CreateFileW( wszPath, dwDesiredAccess, dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes, NULL );
	if( m_hFile == INVALID_HANDLE_VALUE ) {
		EC.SetLastError_MSWin();
		return false;
	}

	EC.Value = 0;
	return true;
#else
	return false;
#endif
}
void CFile::Close()
{
#ifdef _WIN32
	if( m_hFile != INVALID_HANDLE_VALUE ) {
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}
#endif
}

Ax::uintptr CFile::Read( void *pDstBuf, Ax::uintptr cMaxDstBuf, ErrorCode &EC )
{
#ifdef _WIN32
	if( !pDstBuf || cMaxDstBuf >= 0x7FFFFFFF || !cMaxDstBuf ) {
		EC.SetSystemError_MSWin( ERROR_INVALID_PARAMETER );
		return 0;
	}

	DWORD dwNumRead = 0;
	if( !ReadFile( m_hFile, pDstBuf, ( DWORD )cMaxDstBuf, &dwNumRead, nullptr ) ) {
		EC.SetLastError_MSWin();
		return 0;
	}

	EC.Value = 0;
	return ( Ax::uintptr )dwNumRead;
#else
	errno = ENOTSUP;
	EC.SetErrno();
	return 0;
#endif
}
Ax::uintptr CFile::Write( const void *pSrcBuf, Ax::uintptr cSrcBytes, ErrorCode &EC )
{
#ifdef _WIN32
	if( !cSrcBytes ) {
		EC.Value = 0;
		return 0;
	}
	if( !pSrcBuf || cSrcBytes >= 0x7FFFFFFF ) {
		EC.SetSystemError_MSWin( ERROR_INVALID_PARAMETER );
		return 0;
	}

	DWORD dwNumWritten = 0;
	if( !WriteFile( m_hFile, pSrcBuf, ( DWORD )cSrcBytes, &dwNumWritten, nullptr ) ) {
		EC.SetLastError_MSWin();
		return 0;
	}

	EC.Value = 0;
	return ( Ax::uintptr )dwNumWritten;
#else
	errno = ENOTSUP;
	EC.SetErrno();
	return 0;
#endif
}

Ax::uint64 CFile::GetSize()
{
#ifdef _WIN32
	LARGE_INTEGER Size;

	if( !GetFileSizeEx( m_hFile, &Size ) ) {
		return 0;
	}

	return ( Ax::uint64 )Size.QuadPart;
#else
	return 0;
#endif
}
Ax::uint64 CFile::GetPos()
{
#ifdef _WIN32
	LARGE_INTEGER Dist, CurPos;

	Dist.QuadPart = 0;
	if( !SetFilePointerEx( m_hFile, Dist, &CurPos, FILE_CURRENT ) ) {
		return 0;
	}

	return ( Ax::uint64 )CurPos.QuadPart;
#else
	return 0;
#endif
}
bool CFile::SetPos( Ax::uint64 uByteOffset )
{
#ifdef _WIN32
	LARGE_INTEGER Off;

	Off.QuadPart = uByteOffset;
	if( !SetFilePointerEx( m_hFile, Off, nullptr, FILE_BEGIN ) ) {
		return false;
	}

	return true;
#else
	return false;
#endif
}
bool CFile::Skip( Ax::int64 cBytes )
{
#ifdef _WIN32
	LARGE_INTEGER Off;

	Off.QuadPart = cBytes;
	if( !SetFilePointerEx( m_hFile, Off, nullptr, FILE_CURRENT ) ) {
		return false;
	}

	return true;
#else
	return false;
#endif
}
bool CFile::SkipEnd( Ax::int64 cBytes )
{
#ifdef _WIN32
	LARGE_INTEGER Off;

	Off.QuadPart = cBytes;
	if( !SetFilePointerEx( m_hFile, Off, nullptr, FILE_END ) ) {
		return false;
	}

	return true;
#else
	return false;
#endif
}

bool CFile::Flush()
{
#ifdef _WIN32
	return !!FlushFileBuffers( m_hFile );
#else
	return false;
#endif
}

bool CFile::IsEnd()
{
#ifdef _WIN32
	return GetPos() >= GetSize();
#else
	return true;
#endif
}

//----------------------------------------------------------------------------//

void LogError( ErrorCode &EC );

namespace {
CFile *File( TenshiIndex_t uIndex )
{
	CFile *const pFile = g_Files.Unwrap( uIndex );
	TE_CHECK( TE_ERR_FS_FILEUNUSED, pFile != nullptr );

	return pFile;
}

TenshiIndex_t OpenFile( TenshiIndex_t FileNumber, const char *pszFilename, Ax::uint32 Mode, ErrorCode *pEC )
{
	TenshiIndex_t uFile = g_Files.Make( FileNumber );
	if( !uFile ) {
		TE_TRACE( "OpenFile( FileNumber=%u, pszFilename=\"%s\", Mode=0x%.8X, pEC=%p )@Make failed",
			FileNumber, pszFilename, ( unsigned int )Mode, ( const void * )pEC );
		TE_CHECK( TE_ERR_BADALLOC, FileNumber != 0 );

		Tenshi::RuntimeError( TE_ERR_EXISTS );
	}

	CFile *const pFile = g_Files.Unwrap( uFile );
	AX_ASSERT_NOT_NULL( pFile );

	ErrorCode LocalEC;
	if( !pFile->Open( pszFilename, Mode, LocalEC ) ) {
		if( !pEC ) {
			SystemError( LocalEC );
		}

		*pEC = LocalEC;
		g_Files.Free( uFile );
		return 0;
	}

	if( !!pEC ) {
		pEC->Value = 0;
	}
	return uFile;
}
} // anonymous namespace

TE_FUNC void TE_CALL FS_OpenToRead( TenshiIndex_t FileNumber, const char *pszFilename )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	OpenFile( FileNumber, pszFilename, OPENF_R, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenToReadE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	return !!OpenFile( FileNumber, pszFilename, OPENF_R, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToReadA( const char *pszFilename )
{
	return OpenFile( 0, pszFilename, OPENF_R, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToReadEA( const char *pszFilename, ErrorCode &EC )
{
	return OpenFile( 0, pszFilename, OPENF_R, &EC );
}

TE_FUNC void TE_CALL FS_OpenToWrite( TenshiIndex_t FileNumber, const char *pszFilename )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	OpenFile( FileNumber, pszFilename, OPENF_W, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenToWriteE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	return !!OpenFile( FileNumber, pszFilename, OPENF_W, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToWriteA( const char *pszFilename )
{
	return OpenFile( 0, pszFilename, OPENF_W, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToWriteEA( const char *pszFilename, ErrorCode &EC )
{
	return OpenFile( 0, pszFilename, OPENF_W, &EC );
}

TE_FUNC void TE_CALL FS_OpenToAppend( TenshiIndex_t FileNumber, const char *pszFilename )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	OpenFile( FileNumber, pszFilename, OPENF_APPEND, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenToAppendE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	return !!OpenFile( FileNumber, pszFilename, OPENF_APPEND, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToAppendA( const char *pszFilename )
{
	return OpenFile( 0, pszFilename, OPENF_APPEND, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToAppendEA( const char *pszFilename, ErrorCode &EC )
{
	return OpenFile( 0, pszFilename, OPENF_APPEND, &EC );
}

TE_FUNC void TE_CALL FS_OpenFile( TenshiIndex_t FileNumber, const char *pszFilename )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	OpenFile( FileNumber, pszFilename, OPENF_RW, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenFileE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	return !!OpenFile( FileNumber, pszFilename, OPENF_RW, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileA( const char *pszFilename )
{
	return OpenFile( 0, pszFilename, OPENF_RW, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileEA( const char *pszFilename, ErrorCode &EC )
{
	return OpenFile( 0, pszFilename, OPENF_RW, &EC );
}
TE_FUNC void TE_CALL FS_OpenFileF( TenshiIndex_t FileNumber, const char *pszFilename, Ax::uint32 Mode )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	OpenFile( FileNumber, pszFilename, Mode, nullptr );
}
TE_FUNC bool TE_CALL FS_OpenFileFE( TenshiIndex_t FileNumber, const char *pszFilename, Ax::uint32 Mode, ErrorCode &EC )
{
	TE_CHECK( TE_ERR_INVALID, FileNumber != 0 );
	return !!OpenFile( FileNumber, pszFilename, Mode, &EC );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileFA( const char *pszFilename, Ax::uint32 Mode )
{
	return OpenFile( 0, pszFilename, Mode, nullptr );
}
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileFEA( const char *pszFilename, Ax::uint32 Mode, ErrorCode &EC )
{
	return OpenFile( 0, pszFilename, Mode, &EC );
}

TE_FUNC TenshiIndex_t TE_CALL FS_CloseFile( TenshiIndex_t FileNumber )
{
	g_Files.Free( FileNumber );
	return 0;
}

TE_FUNC Ax::uintptr TE_CALL FS_WriteFromPtr( TenshiIndex_t FileNumber, const void *pSrcBuf, Ax::uintptr cSrcBytes )
{
	ErrorCode EC;
	const Ax::uintptr cBytesWritten = File( FileNumber )->Write( pSrcBuf, cSrcBytes, EC );
	if( !cBytesWritten && EC ) {
		SystemError( EC );
	}

	return cBytesWritten;
}
TE_FUNC Ax::uintptr TE_CALL FS_WriteFromPtrE( TenshiIndex_t FileNumber, const void *pSrcBuf, Ax::uintptr cSrcBytes, ErrorCode &EC )
{
	return File( FileNumber )->Write( pSrcBuf, cSrcBytes, EC );
}

TE_FUNC Ax::uintptr TE_CALL FS_ReadToPtr( TenshiIndex_t FileNumber, void *pDstBuf, Ax::uintptr cDstBytes )
{
	ErrorCode EC;
	const Ax::uintptr cBytesRead = File( FileNumber )->Read( pDstBuf, cDstBytes, EC );
	if( !cBytesRead && EC ) {
		SystemError( EC );
	}

	return cBytesRead;
}
TE_FUNC Ax::uintptr TE_CALL FS_ReadToPtrE( TenshiIndex_t FileNumber, void *pDstBuf, Ax::uintptr cDstBytes, ErrorCode &EC )
{
	return File( FileNumber )->Read( pDstBuf, cDstBytes, EC );
}

TE_FUNC void TE_CALL FS_Write8( TenshiIndex_t FileNumber, Ax::uint8 Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 1, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Write8E( TenshiIndex_t FileNumber, Ax::uint8 Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 1, EC ) == 1;
}
TE_FUNC void TE_CALL FS_Write16( TenshiIndex_t FileNumber, Ax::uint16 Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 2, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Write16E( TenshiIndex_t FileNumber, Ax::uint16 Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 2, EC ) == 2;
}
TE_FUNC void TE_CALL FS_Write32( TenshiIndex_t FileNumber, Ax::uint32 Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 4, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Write32E( TenshiIndex_t FileNumber, Ax::uint32 Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 4, EC ) == 4;
}
TE_FUNC void TE_CALL FS_Write64( TenshiIndex_t FileNumber, Ax::uint64 Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 8, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Write64E( TenshiIndex_t FileNumber, Ax::uint64 Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 8, EC ) == 8;
}
TE_FUNC void TE_CALL FS_WriteF32( TenshiIndex_t FileNumber, float Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 4, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_WriteF32E( TenshiIndex_t FileNumber, float Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 4, EC ) == 4;
}
TE_FUNC void TE_CALL FS_WriteF64( TenshiIndex_t FileNumber, double Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Write( &Value, 8, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_WriteF64E( TenshiIndex_t FileNumber, double Value, ErrorCode &EC )
{
	return File( FileNumber )->Write( &Value, 8, EC ) == 8;
}



static Ax::uint16 BE( Ax::uint16 x )
{
	union {
		Ax::uint8 a[2];
		Ax::uint16 v;
	} u;

	u.a[0] = Ax::uint8( ( x&0xFF00 )>>8 );
	u.a[1] = Ax::uint8( ( x&0x00FF )>>0 );

	return u.v;
}
static Ax::uint32 BE( Ax::uint32 x )
{
	union {
		Ax::uint8 a[4];
		Ax::uint32 v;
	} u;

	u.a[0] = Ax::uint8( (x&0xFF000000)>>24 );
	u.a[1] = Ax::uint8( (x&0x00FF0000)>>16 );
	u.a[2] = Ax::uint8( (x&0x0000FF00)>> 8 );
	u.a[3] = Ax::uint8( (x&0x000000FF)>> 0 );

	return u.v;
}
static Ax::uint64 BE( Ax::uint64 x )
{
	union {
		Ax::uint8 a[8];
		Ax::uint64 v;
	} u;

	u.a[0] = Ax::uint8( (x&0xFF00000000000000ULL)>>56 );
	u.a[1] = Ax::uint8( (x&0x00FF000000000000ULL)>>48 );
	u.a[2] = Ax::uint8( (x&0x0000FF0000000000ULL)>>40 );
	u.a[3] = Ax::uint8( (x&0x000000FF00000000ULL)>>32 );
	u.a[4] = Ax::uint8( (x&0x00000000FF000000ULL)>>24 );
	u.a[5] = Ax::uint8( (x&0x0000000000FF0000ULL)>>16 );
	u.a[6] = Ax::uint8( (x&0x000000000000FF00ULL)>> 8 );
	u.a[7] = Ax::uint8( (x&0x00000000000000FFULL)>> 0 );

	return u.v;
}
static Ax::uint32 BE( float x )
{
	union {
		Ax::uint32 v;
		float f;
	} u;

	u.f = x;
	return BE( u.v );
}
static Ax::uint64 BE( double x )
{
	union {
		Ax::uint64 v;
		double f;
	} u;

	u.f = x;
	return BE( u.v );
}

TE_FUNC void TE_CALL FS_WriteBE16( TenshiIndex_t FileNumber, Ax::uint16 Value )
{
	FS_Write16( FileNumber, BE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteBE16E( TenshiIndex_t FileNumber, Ax::uint16 Value, ErrorCode &EC )
{
	return FS_Write16E( FileNumber, BE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteBE32( TenshiIndex_t FileNumber, Ax::uint32 Value )
{
	FS_Write32( FileNumber, BE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteBE32E( TenshiIndex_t FileNumber, Ax::uint32 Value, ErrorCode &EC )
{
	return FS_Write32E( FileNumber, BE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteBE64( TenshiIndex_t FileNumber, Ax::uint64 Value )
{
	FS_Write64( FileNumber, BE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteBE64E( TenshiIndex_t FileNumber, Ax::uint64 Value, ErrorCode &EC )
{
	return FS_Write64E( FileNumber, BE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteBEF32( TenshiIndex_t FileNumber, float Value )
{
	FS_Write32( FileNumber, BE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteBEF32E( TenshiIndex_t FileNumber, float Value, ErrorCode &EC )
{
	return FS_Write32E( FileNumber, BE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteBEF64( TenshiIndex_t FileNumber, double Value )
{
	FS_Write64( FileNumber, BE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteBEF64E( TenshiIndex_t FileNumber, double Value, ErrorCode &EC )
{
	return FS_Write64E( FileNumber, BE( Value ), EC );
}



static Ax::uint16 LE( Ax::uint16 x )
{
	union {
		Ax::uint8 a[2];
		Ax::uint16 v;
	} u;

	u.a[0] = Ax::uint8( ( x&0x00FF )>>0 );
	u.a[1] = Ax::uint8( ( x&0xFF00 )>>8 );

	return u.v;
}
static Ax::uint32 LE( Ax::uint32 x )
{
	union {
		Ax::uint8 a[4];
		Ax::uint32 v;
	} u;

	u.a[0] = Ax::uint8( (x&0x000000FF)>> 0 );
	u.a[1] = Ax::uint8( (x&0x0000FF00)>> 8 );
	u.a[2] = Ax::uint8( (x&0x00FF0000)>>16 );
	u.a[3] = Ax::uint8( (x&0xFF000000)>>24 );

	return u.v;
}
static Ax::uint64 LE( Ax::uint64 x )
{
	union {
		Ax::uint8 a[8];
		Ax::uint64 v;
	} u;

	u.a[0] = Ax::uint8( (x&0x00000000000000FFULL)>> 0 );
	u.a[1] = Ax::uint8( (x&0x000000000000FF00ULL)>> 8 );
	u.a[2] = Ax::uint8( (x&0x0000000000FF0000ULL)>>16 );
	u.a[3] = Ax::uint8( (x&0x00000000FF000000ULL)>>24 );
	u.a[4] = Ax::uint8( (x&0x000000FF00000000ULL)>>32 );
	u.a[5] = Ax::uint8( (x&0x0000FF0000000000ULL)>>40 );
	u.a[6] = Ax::uint8( (x&0x00FF000000000000ULL)>>48 );
	u.a[7] = Ax::uint8( (x&0xFF00000000000000ULL)>>56 );

	return u.v;
}
static Ax::uint32 LE( float x )
{
	union {
		Ax::uint32 v;
		float f;
	} u;

	u.f = x;
	return LE( u.v );
}
static Ax::uint64 LE( double x )
{
	union {
		Ax::uint64 v;
		double f;
	} u;

	u.f = x;
	return LE( u.v );
}

TE_FUNC void TE_CALL FS_WriteLE16( TenshiIndex_t FileNumLEr, Ax::uint16 Value )
{
	FS_Write16( FileNumLEr, LE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteLE16E( TenshiIndex_t FileNumLEr, Ax::uint16 Value, ErrorCode &EC )
{
	return FS_Write16E( FileNumLEr, LE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteLE32( TenshiIndex_t FileNumLEr, Ax::uint32 Value )
{
	FS_Write32( FileNumLEr, LE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteLE32E( TenshiIndex_t FileNumLEr, Ax::uint32 Value, ErrorCode &EC )
{
	return FS_Write32E( FileNumLEr, LE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteLE64( TenshiIndex_t FileNumLEr, Ax::uint64 Value )
{
	FS_Write64( FileNumLEr, LE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteLE64E( TenshiIndex_t FileNumLEr, Ax::uint64 Value, ErrorCode &EC )
{
	return FS_Write64E( FileNumLEr, LE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteLEF32( TenshiIndex_t FileNumLEr, float Value )
{
	FS_Write32( FileNumLEr, LE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteLEF32E( TenshiIndex_t FileNumLEr, float Value, ErrorCode &EC )
{
	return FS_Write32E( FileNumLEr, LE( Value ), EC );
}
TE_FUNC void TE_CALL FS_WriteLEF64( TenshiIndex_t FileNumLEr, double Value )
{
	FS_Write64( FileNumLEr, LE( Value ) );
}
TE_FUNC bool TE_CALL FS_WriteLEF64E( TenshiIndex_t FileNumLEr, double Value, ErrorCode &EC )
{
	return FS_Write64E( FileNumLEr, LE( Value ), EC );
}




TE_FUNC void TE_CALL FS_Read8( TenshiIndex_t FileNumber, Ax::uint8 &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 1, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Read8E( TenshiIndex_t FileNumber, Ax::uint8 &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 1, EC ) == 1;
}
TE_FUNC void TE_CALL FS_Read16( TenshiIndex_t FileNumber, Ax::uint16 &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 2, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Read16E( TenshiIndex_t FileNumber, Ax::uint16 &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 2, EC ) == 2;
}
TE_FUNC void TE_CALL FS_Read32( TenshiIndex_t FileNumber, Ax::uint32 &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 4, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Read32E( TenshiIndex_t FileNumber, Ax::uint32 &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 4, EC ) == 4;
}
TE_FUNC void TE_CALL FS_Read64( TenshiIndex_t FileNumber, Ax::uint64 &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 8, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_Read64E( TenshiIndex_t FileNumber, Ax::uint64 &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 8, EC ) == 8;
}
TE_FUNC void TE_CALL FS_ReadF32( TenshiIndex_t FileNumber, float &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 4, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_ReadF32E( TenshiIndex_t FileNumber, float &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 4, EC ) == 4;
}
TE_FUNC void TE_CALL FS_ReadF64( TenshiIndex_t FileNumber, double &Value )
{
	ErrorCode EC;
	if( !File( FileNumber )->Read( &Value, 8, EC ) && EC ) {
		SystemError( EC );
	}
}
TE_FUNC bool TE_CALL FS_ReadF64E( TenshiIndex_t FileNumber, double &Value, ErrorCode &EC )
{
	return File( FileNumber )->Read( &Value, 8, EC ) == 8;
}




static Ax::uint16 FromBE( Ax::uint16 x )
{
	union {
		Ax::uint8 a[ 2 ];
		Ax::uint16 v;
	} u;

	u.v = x;

	return
		( Ax::uint16( u.a[0] )<<8 ) |
		( Ax::uint16( u.a[1] )<<0 );
}
static Ax::uint32 FromBE( Ax::uint32 x )
{
	union {
		Ax::uint8 a[ 4 ];
		Ax::uint32 v;
	} u;

	u.v = x;

	return
		( Ax::uint32( u.a[0] )<<24 ) |
		( Ax::uint32( u.a[1] )<<16 ) |
		( Ax::uint32( u.a[2] )<< 8 ) |
		( Ax::uint32( u.a[3] )<< 0 );
}
static Ax::uint64 FromBE( Ax::uint64 x )
{
	union {
		Ax::uint8 a[ 8 ];
		Ax::uint64 v;
	} u;

	u.v = x;

	return
		( Ax::uint64( u.a[0] )<<56 ) |
		( Ax::uint64( u.a[1] )<<48 ) |
		( Ax::uint64( u.a[2] )<<40 ) |
		( Ax::uint64( u.a[3] )<<32 ) |
		( Ax::uint64( u.a[4] )<<24 ) |
		( Ax::uint64( u.a[5] )<<16 ) |
		( Ax::uint64( u.a[6] )<< 8 ) |
		( Ax::uint64( u.a[7] )<< 0 );
}
static float FromBE( float x )
{
	union {
		float f;
		Ax::uint32 v;
	} u;

	u.f = x;
	u.v = FromBE( u.v );
	return u.f;
}
static double FromBE( double x )
{
	union {
		double f;
		Ax::uint64 v;
	} u;

	u.f = x;
	u.v = FromBE( u.v );
	return u.f;
}

TE_FUNC void TE_CALL FS_ReadBE16( TenshiIndex_t FileNumber, Ax::uint16 &Value )
{
	Ax::uint16 v;
	FS_Read16( FileNumber, v );
	Value = FromBE( v );
}
TE_FUNC bool TE_CALL FS_ReadBE16E( TenshiIndex_t FileNumber, Ax::uint16 &Value, ErrorCode &EC )
{
	Ax::uint16 v;
	if( !FS_Read16E( FileNumber, v, EC ) ) {
		return false;
	}
	Value = FromBE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadBE32( TenshiIndex_t FileNumber, Ax::uint32 &Value )
{
	Ax::uint32 v;
	FS_Read32( FileNumber, v );
	Value = FromBE( v );
}
TE_FUNC bool TE_CALL FS_ReadBE32E( TenshiIndex_t FileNumber, Ax::uint32 &Value, ErrorCode &EC )
{
	Ax::uint32 v;
	if( !FS_Read32E( FileNumber, v, EC ) ) {
		return false;
	}
	Value = FromBE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadBE64( TenshiIndex_t FileNumber, Ax::uint64 &Value )
{
	Ax::uint64 v;
	FS_Read64( FileNumber, v );
	Value = FromBE( v );
}
TE_FUNC bool TE_CALL FS_ReadBE64E( TenshiIndex_t FileNumber, Ax::uint64 &Value, ErrorCode &EC )
{
	Ax::uint64 v;
	if( !FS_Read64E( FileNumber, v, EC ) ) {
		return false;
	}
	Value = FromBE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadBEF32( TenshiIndex_t FileNumber, float &Value )
{
	float v;
	FS_ReadF32( FileNumber, v );
	Value = FromBE( v );
}
TE_FUNC bool TE_CALL FS_ReadBEF32E( TenshiIndex_t FileNumber, float &Value, ErrorCode &EC )
{
	float v;
	if( !FS_ReadF32E( FileNumber, v, EC ) ) {
		return false;
	}
	Value = FromBE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadBEF64( TenshiIndex_t FileNumber, double &Value )
{
	double v;
	FS_ReadF64( FileNumber, v );
	Value = FromBE( v );
}
TE_FUNC bool TE_CALL FS_ReadBEF64E( TenshiIndex_t FileNumber, double &Value, ErrorCode &EC )
{
	double v;
	if( !FS_ReadF64E( FileNumber, v, EC ) ) {
		return false;
	}
	Value = FromBE( v );
	return true;
}




static Ax::uint16 FromLE( Ax::uint16 x )
{
	union {
		Ax::uint8 a[ 2 ];
		Ax::uint16 v;
	} u;

	u.v = x;

	return
		( Ax::uint16( u.a[1] )<<8 ) |
		( Ax::uint16( u.a[0] )<<0 );
}
static Ax::uint32 FromLE( Ax::uint32 x )
{
	union {
		Ax::uint8 a[ 4 ];
		Ax::uint32 v;
	} u;

	u.v = x;

	return
		( Ax::uint32( u.a[3] )<<24 ) |
		( Ax::uint32( u.a[2] )<<16 ) |
		( Ax::uint32( u.a[1] )<< 8 ) |
		( Ax::uint32( u.a[0] )<< 0 );
}
static Ax::uint64 FromLE( Ax::uint64 x )
{
	union {
		Ax::uint8 a[ 8 ];
		Ax::uint64 v;
	} u;

	u.v = x;

	return
		( Ax::uint64( u.a[7] )<<56 ) |
		( Ax::uint64( u.a[6] )<<48 ) |
		( Ax::uint64( u.a[5] )<<40 ) |
		( Ax::uint64( u.a[4] )<<32 ) |
		( Ax::uint64( u.a[3] )<<24 ) |
		( Ax::uint64( u.a[2] )<<16 ) |
		( Ax::uint64( u.a[1] )<< 8 ) |
		( Ax::uint64( u.a[0] )<< 0 );
}
static float FromLE( float x )
{
	union {
		float f;
		Ax::uint32 v;
	} u;

	u.f = x;
	u.v = FromLE( u.v );
	return u.f;
}
static double FromLE( double x )
{
	union {
		double f;
		Ax::uint64 v;
	} u;

	u.f = x;
	u.v = FromLE( u.v );
	return u.f;
}

TE_FUNC void TE_CALL FS_ReadLE16( TenshiIndex_t FileNumLEr, Ax::uint16 &Value )
{
	Ax::uint16 v;
	FS_Read16( FileNumLEr, v );
	Value = FromLE( v );
}
TE_FUNC bool TE_CALL FS_ReadLE16E( TenshiIndex_t FileNumLEr, Ax::uint16 &Value, ErrorCode &EC )
{
	Ax::uint16 v;
	if( !FS_Read16E( FileNumLEr, v, EC ) ) {
		return false;
	}
	Value = FromLE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadLE32( TenshiIndex_t FileNumLEr, Ax::uint32 &Value )
{
	Ax::uint32 v;
	FS_Read32( FileNumLEr, v );
	Value = FromLE( v );
}
TE_FUNC bool TE_CALL FS_ReadLE32E( TenshiIndex_t FileNumLEr, Ax::uint32 &Value, ErrorCode &EC )
{
	Ax::uint32 v;
	if( !FS_Read32E( FileNumLEr, v, EC ) ) {
		return false;
	}
	Value = FromLE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadLE64( TenshiIndex_t FileNumLEr, Ax::uint64 &Value )
{
	Ax::uint64 v;
	FS_Read64( FileNumLEr, v );
	Value = FromLE( v );
}
TE_FUNC bool TE_CALL FS_ReadLE64E( TenshiIndex_t FileNumLEr, Ax::uint64 &Value, ErrorCode &EC )
{
	Ax::uint64 v;
	if( !FS_Read64E( FileNumLEr, v, EC ) ) {
		return false;
	}
	Value = FromLE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadLEF32( TenshiIndex_t FileNumLEr, float &Value )
{
	float v;
	FS_ReadF32( FileNumLEr, v );
	Value = FromLE( v );
}
TE_FUNC bool TE_CALL FS_ReadLEF32E( TenshiIndex_t FileNumLEr, float &Value, ErrorCode &EC )
{
	float v;
	if( !FS_ReadF32E( FileNumLEr, v, EC ) ) {
		return false;
	}
	Value = FromLE( v );
	return true;
}
TE_FUNC void TE_CALL FS_ReadLEF64( TenshiIndex_t FileNumLEr, double &Value )
{
	double v;
	FS_ReadF64( FileNumLEr, v );
	Value = FromLE( v );
}
TE_FUNC bool TE_CALL FS_ReadLEF64E( TenshiIndex_t FileNumLEr, double &Value, ErrorCode &EC )
{
	double v;
	if( !FS_ReadF64E( FileNumLEr, v, EC ) ) {
		return false;
	}
	Value = FromLE( v );
	return true;
}

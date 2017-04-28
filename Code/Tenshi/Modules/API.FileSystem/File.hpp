#pragma once

#include "../APICommon.hpp"

#define OPENF_R						( 0x00000 )
#define OPENF_W						( 0x00001 )
#define OPENF_RW					( 0x00002 )
#define OPENF_APPEND				( 0x00003 )
#define OPENM_ACCESS				( 0x00007 )

#define OPENF_SHARE_DEFAULT			( 0x00000 )
#define OPENF_SHARE_R				( 0x00010 )
#define OPENF_SHARE_W				( 0x00020 )
#define OPENF_SHARE_RW				( 0x00030 )
#define OPENF_NSHARE_R				( 0x00040 )
#define OPENF_NSHARE_W				( 0x00050 )
#define OPENF_NSHARE_RW				( 0x00060 )
#define OPENF_SHARE_DELETE			( 0x00080 )
#define OPENF_NSHARE_DELETE			( 0x00000 )
#define OPENM_SHARE					( 0x000F0 )
#define OPENM_SHAREMODE				( 0x00070 )

#define OPENF_DISPOSITION_DEFAULT	( 0x00000 )
#define OPENF_CREATE_NEW			( 0x00100 )
#define OPENF_CREATE_ALWAYS			( 0x00200 )
#define OPENF_OPEN_EXISTING			( 0x00300 )
#define OPENF_OPEN_ALWAYS			( 0x00400 )
#define OPENF_TRUNCATE_EXISTING		( 0x00500 )
#define OPENM_DISPOSITION			( 0x00700 )

#define OPENF_DELETE_ON_CLOSE		( 0x01000 )
#define OPENF_NO_BUFFERING			( 0x02000 )
#define OPENF_SEQUENTIAL_ACCESS		( 0x04000 )
#define OPENF_RANDOM_ACCESS			( 0x08000 )
#define OPENF_ASYNC					( 0x10000 )
#define OPENF_WRITE_THROUGH			( 0x20000 )
#define OPENF_HIDDEN				( 0x40000 )
#define OPENF_TEMPORARY				( 0x80000 )
#define OPENM_FLAGS					( 0xFF000 )

inline bool CanRead( Ax::uint32 M )
{
	switch( M & OPENM_ACCESS )
	{
	case OPENF_R:
	case OPENF_RW:
		return true;
	}

	return false;
}
inline bool CanWrite( Ax::uint32 M )
{
	switch( M & OPENM_ACCESS )
	{
	case OPENF_W:
	case OPENF_RW:
	case OPENF_APPEND:
		return true;
	}

	return false;
}

inline bool SharingReads( Ax::uint32 M )
{
	switch( M & OPENM_SHAREMODE )
	{
	case OPENF_SHARE_R:
	case OPENF_SHARE_RW:
		return true;

	case OPENF_NSHARE_R:
	case OPENF_NSHARE_RW:
		return false;

	default:
		break;
	}

	if( CanWrite( M ) ) {
		return false;
	}

	return true;
}
inline bool SharingWrites( Ax::uint32 M )
{
	switch( M & OPENM_SHAREMODE )
	{
	case OPENF_SHARE_W:
	case OPENF_SHARE_RW:
		return true;

	case OPENF_NSHARE_W:
	case OPENF_NSHARE_RW:
		return false;

	default:
		break;
	}

	if( ( M & OPENM_ACCESS ) == OPENF_APPEND ) {
		return true;
	}

	return false;
}

inline Ax::uint32 GetEffectiveDisposition( Ax::uint32 M )
{
	if( ( M & OPENM_DISPOSITION ) != 0 ) {
		return M;
	}

	switch( M & OPENM_ACCESS )
	{
	case OPENF_R:
		return OPENF_OPEN_EXISTING;

	case OPENF_W:
		return OPENF_CREATE_ALWAYS;

	case OPENF_RW:
	case OPENF_APPEND:
		return OPENF_OPEN_ALWAYS;
	}

	return 0;
}

class CFile
{
public:
	static bool Init();
	static void Fini();

	CFile();
	~CFile();

	bool Open( const char *pszPath, Ax::uint32 Mode, ErrorCode &EC );
	void Close();

	Ax::uintptr Read( void *pDstBuf, Ax::uintptr cMaxDstBuf, ErrorCode &EC );
	Ax::uintptr Write( const void *pSrcBuf, Ax::uintptr cSrcBytes, ErrorCode &EC );

	Ax::uint64 GetSize();
	Ax::uint64 GetPos();
	bool SetPos( Ax::uint64 uByteOffset );
	bool Skip( Ax::int64 cBytes );
	bool SkipEnd( Ax::int64 cBytes );

	bool Flush();

	bool IsEnd();

private:
#ifdef _WIN32
	HANDLE							m_hFile;
#elif defined(__APPLE__)
#else
# error Please implement this
#endif
};

//----------------------------------------------------------------------------//

TE_FUNC void TE_CALL FS_OpenToRead( TenshiIndex_t FileNumber, const char *pszFilename );
TE_FUNC bool TE_CALL FS_OpenToReadE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToReadA( const char *pszFilename );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToReadEA( const char *pszFilename, ErrorCode &EC );

TE_FUNC void TE_CALL FS_OpenToWrite( TenshiIndex_t FileNumber, const char *pszFilename );
TE_FUNC bool TE_CALL FS_OpenToWriteE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToWriteA( const char *pszFilename );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToWriteEA( const char *pszFilename, ErrorCode &EC );

TE_FUNC void TE_CALL FS_OpenToAppend( TenshiIndex_t FileNumber, const char *pszFilename );
TE_FUNC bool TE_CALL FS_OpenToAppendE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToAppendA( const char *pszFilename );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenToAppendEA( const char *pszFilename, ErrorCode &EC );

TE_FUNC void TE_CALL FS_OpenFile( TenshiIndex_t FileNumber, const char *pszFilename );
TE_FUNC bool TE_CALL FS_OpenFileE( TenshiIndex_t FileNumber, const char *pszFilename, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileA( const char *pszFilename );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileEA( const char *pszFilename, ErrorCode &EC );
TE_FUNC void TE_CALL FS_OpenFileF( TenshiIndex_t FileNumber, const char *pszFilename, Ax::uint32 Mode );
TE_FUNC bool TE_CALL FS_OpenFileFE( TenshiIndex_t FileNumber, const char *pszFilename, Ax::uint32 Mode, ErrorCode &EC );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileFA( const char *pszFilename, Ax::uint32 Mode );
TE_FUNC TenshiIndex_t TE_CALL FS_OpenFileFEA( const char *pszFilename, Ax::uint32 Mode, ErrorCode &EC );

TE_FUNC TenshiIndex_t TE_CALL FS_CloseFile( TenshiIndex_t FileNumber );

TE_FUNC Ax::uintptr TE_CALL FS_WriteFromPtr( TenshiIndex_t FileNumber, const void *pSrcBuf, Ax::uintptr cSrcBytes );
TE_FUNC Ax::uintptr TE_CALL FS_WriteFromPtrE( TenshiIndex_t FileNumber, const void *pSrcBuf, Ax::uintptr cSrcBytes, ErrorCode &EC );

TE_FUNC Ax::uintptr TE_CALL FS_ReadToPtr( TenshiIndex_t FileNumber, void *pDstBuf, Ax::uintptr cDstBytes );
TE_FUNC Ax::uintptr TE_CALL FS_ReadToPtrE( TenshiIndex_t FileNumber, void *pDstBuf, Ax::uintptr cDstBytes, ErrorCode &EC );

TE_FUNC void TE_CALL FS_Write8( TenshiIndex_t FileNumber, Ax::uint8 Value );
TE_FUNC bool TE_CALL FS_Write8E( TenshiIndex_t FileNumber, Ax::uint8 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Write16( TenshiIndex_t FileNumber, Ax::uint16 Value );
TE_FUNC bool TE_CALL FS_Write16E( TenshiIndex_t FileNumber, Ax::uint16 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Write32( TenshiIndex_t FileNumber, Ax::uint32 Value );
TE_FUNC bool TE_CALL FS_Write32E( TenshiIndex_t FileNumber, Ax::uint32 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Write64( TenshiIndex_t FileNumber, Ax::uint64 Value );
TE_FUNC bool TE_CALL FS_Write64E( TenshiIndex_t FileNumber, Ax::uint64 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteF32( TenshiIndex_t FileNumber, float Value );
TE_FUNC bool TE_CALL FS_WriteF32E( TenshiIndex_t FileNumber, float Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteF64( TenshiIndex_t FileNumber, double Value );
TE_FUNC bool TE_CALL FS_WriteF64E( TenshiIndex_t FileNumber, double Value, ErrorCode &EC );

TE_FUNC void TE_CALL FS_WriteBE16( TenshiIndex_t FileNumber, Ax::uint16 Value );
TE_FUNC bool TE_CALL FS_WriteBE16E( TenshiIndex_t FileNumber, Ax::uint16 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteBE32( TenshiIndex_t FileNumber, Ax::uint32 Value );
TE_FUNC bool TE_CALL FS_WriteBE32E( TenshiIndex_t FileNumber, Ax::uint32 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteBE64( TenshiIndex_t FileNumber, Ax::uint64 Value );
TE_FUNC bool TE_CALL FS_WriteBE64E( TenshiIndex_t FileNumber, Ax::uint64 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteBEF32( TenshiIndex_t FileNumber, float Value );
TE_FUNC bool TE_CALL FS_WriteBEF32E( TenshiIndex_t FileNumber, float Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteBEF64( TenshiIndex_t FileNumber, double Value );
TE_FUNC bool TE_CALL FS_WriteBEF64E( TenshiIndex_t FileNumber, double Value, ErrorCode &EC );

TE_FUNC void TE_CALL FS_WriteLE16( TenshiIndex_t FileNumber, Ax::uint16 Value );
TE_FUNC bool TE_CALL FS_WriteLE16E( TenshiIndex_t FileNumber, Ax::uint16 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteLE32( TenshiIndex_t FileNumber, Ax::uint32 Value );
TE_FUNC bool TE_CALL FS_WriteLE32E( TenshiIndex_t FileNumber, Ax::uint32 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteLE64( TenshiIndex_t FileNumber, Ax::uint64 Value );
TE_FUNC bool TE_CALL FS_WriteLE64E( TenshiIndex_t FileNumber, Ax::uint64 Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteLEF32( TenshiIndex_t FileNumber, float Value );
TE_FUNC bool TE_CALL FS_WriteLEF32E( TenshiIndex_t FileNumber, float Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_WriteLEF64( TenshiIndex_t FileNumber, double Value );
TE_FUNC bool TE_CALL FS_WriteLEF64E( TenshiIndex_t FileNumber, double Value, ErrorCode &EC );

TE_FUNC void TE_CALL FS_Read8( TenshiIndex_t FileNumber, Ax::uint8 &Value );
TE_FUNC bool TE_CALL FS_Read8E( TenshiIndex_t FileNumber, Ax::uint8 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Read16( TenshiIndex_t FileNumber, Ax::uint16 &Value );
TE_FUNC bool TE_CALL FS_Read16E( TenshiIndex_t FileNumber, Ax::uint16 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Read32( TenshiIndex_t FileNumber, Ax::uint32 &Value );
TE_FUNC bool TE_CALL FS_Read32E( TenshiIndex_t FileNumber, Ax::uint32 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_Read64( TenshiIndex_t FileNumber, Ax::uint64 &Value );
TE_FUNC bool TE_CALL FS_Read64E( TenshiIndex_t FileNumber, Ax::uint64 &Value, ErrorCode &EC );

TE_FUNC void TE_CALL FS_ReadBE16( TenshiIndex_t FileNumber, Ax::uint16 &Value );
TE_FUNC bool TE_CALL FS_ReadBE16E( TenshiIndex_t FileNumber, Ax::uint16 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadBE32( TenshiIndex_t FileNumber, Ax::uint32 &Value );
TE_FUNC bool TE_CALL FS_ReadBE32E( TenshiIndex_t FileNumber, Ax::uint32 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadBE64( TenshiIndex_t FileNumber, Ax::uint64 &Value );
TE_FUNC bool TE_CALL FS_ReadBE64E( TenshiIndex_t FileNumber, Ax::uint64 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadBEF32( TenshiIndex_t FileNumber, float &Value );
TE_FUNC bool TE_CALL FS_ReadBEF32E( TenshiIndex_t FileNumber, float &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadBEF64( TenshiIndex_t FileNumber, double &Value );
TE_FUNC bool TE_CALL FS_ReadBEF64E( TenshiIndex_t FileNumber, double &Value, ErrorCode &EC );

TE_FUNC void TE_CALL FS_ReadLE16( TenshiIndex_t FileNumLEr, Ax::uint16 &Value );
TE_FUNC bool TE_CALL FS_ReadLE16E( TenshiIndex_t FileNumLEr, Ax::uint16 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadLE32( TenshiIndex_t FileNumLEr, Ax::uint32 &Value );
TE_FUNC bool TE_CALL FS_ReadLE32E( TenshiIndex_t FileNumLEr, Ax::uint32 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadLE64( TenshiIndex_t FileNumLEr, Ax::uint64 &Value );
TE_FUNC bool TE_CALL FS_ReadLE64E( TenshiIndex_t FileNumLEr, Ax::uint64 &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadLEF32( TenshiIndex_t FileNumLEr, float &Value );
TE_FUNC bool TE_CALL FS_ReadLEF32E( TenshiIndex_t FileNumLEr, float &Value, ErrorCode &EC );
TE_FUNC void TE_CALL FS_ReadLEF64( TenshiIndex_t FileNumLEr, double &Value );
TE_FUNC bool TE_CALL FS_ReadLEF64E( TenshiIndex_t FileNumLEr, double &Value, ErrorCode &EC );

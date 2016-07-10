#include "_PCH.hpp"

struct SFindFiles
{
	static SFindFiles &GetInstance();

	Ax::System::CFileList			Files;
	Ax::uintptr						uIndex;

	bool							bValidStat;
	struct stat						StatInfo;
	char							szDate[ 64 ];
	char							szTime[ 64 ];

	bool Next()
	{
		bValidStat = false;
		++uIndex;
		if( uIndex >= Files.Num() ) {
			uIndex = Files.Num();
			return false;
		}

		return true;
	}
	const char *Current() const
	{
		if( uIndex >= Files.Num() ) {
			return nullptr;
		}

		return Files.GetFile( uIndex );
	}

	struct stat *DoStat()
	{
		if( bValidStat ) {
			return &StatInfo;
		}

		const char *const pszFilename = Files.GetFile( uIndex );
		if( !pszFilename ) {
			return nullptr;
		}

		if( stat( pszFilename, &StatInfo ) != 0 ) {
			return nullptr;
		}

		struct tm t;
#ifdef _MSC_VER
		if( localtime_s( &t, &StatInfo.st_mtime ) != 0 ) {
			return nullptr;
		}
#else
		struct tm *const pt = localtime( &StatInfo.st_mtime );
		if( !pt ) {
			return nullptr;
		}

		t = *pt;
#endif

		strftime( szDate, sizeof( szDate ), "%Y-%m-%d", &t );
		strftime( szTime, sizeof( szTime ), "%H:%M:%S", &t );

		szDate[ sizeof( szDate ) - 1 ] = '\0';
		szTime[ sizeof( szTime ) - 1 ] = '\0';

		bValidStat = true;
		return &StatInfo;
	}

private:
	SFindFiles()
	: Files()
	, uIndex( 0 )
	, bValidStat( false )
	, StatInfo()
	{
	}
	~SFindFiles()
	{
	}

	AX_DELETE_COPYFUNCS(SFindFiles);
};
static Ax::TManager<SFindFiles>		FindFiles;

SFindFiles &SFindFiles::GetInstance()
{
	static SFindFiles instance;
	return instance;
}

TE_FUNC bool TE_CALL FS_FindFirst()
{
	if( !Ax::System::EnumFileTree( FindFiles->Files, "" ) || FindFiles->Files.IsEmpty() ) {
		return false;
	}

	FindFiles->uIndex = 0;
	FindFiles->bValidStat = false;

	return true;
}
TE_FUNC bool TE_CALL FS_FindNext()
{
	return FindFiles->Next();
}

TE_FUNC const char *TE_CALL FS_GetFileDateStr()
{
	if( !FindFiles->DoStat() ) {
		return nullptr;
	}

	return FindFiles->szDate;
}
TE_FUNC const char *TE_CALL FS_GetFileTimeStr()
{
	if( !FindFiles->DoStat() ) {
		return nullptr;
	}

	return FindFiles->szTime;
}
TE_FUNC const char *TE_CALL FS_GetFilenameStr()
{
	return FindFiles->Current();
}

TE_FUNC Ax::int32 TE_CALL FS_GetFileType()
{
	const struct stat *const s = FindFiles->DoStat();
	if( !s ) {
		return -1;
	}

	if( s->st_mode & S_IFDIR ) {
		return 1;
	}

	return 0;
}

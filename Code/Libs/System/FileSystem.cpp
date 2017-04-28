#include "FileSystem.hpp"
#ifdef _MSC_VER
# include "Detail/vc_dirent.h"
#else
# include <dirent.h>
#endif
#include <sys/stat.h>
#include "../Core/Logger.hpp"
#include "../Collections/List.hpp"

#ifdef _MSC_VER
# include <io.h>
# include <direct.h>
#else
# include <unistd.h>
#endif

#include <errno.h>

#ifndef S_IFREG
# define S_IFREG 0100000
#endif
#ifndef S_IFDIR
# define S_IFDIR 0040000
#endif

namespace Ax
{

// get the next extension delimiter
static const char *NextExtensionDelim( const char *exts )
{
	const char *e = strchr( exts, ';' );
	if( !e ) {
		return strchr( exts, '\0' );
	}

	return e;
}
// exts is a semicolon delimited list of extensions (e.g., ".cpp;.h")
static bool HasExtension( const char *src, const char *exts )
{
	const char *srcExt = strrchr( src, '.' );
	if( !srcExt ) {
		return false;
	}

	const size_t srcExtN = strlen( srcExt );

	const char *next;
	for( const char *ext = exts; *ext != '\0'; ext = next ) {
		const char *const delim = NextExtensionDelim( ext );

		next = delim;
		while( *next == ';' ) {
			++next;
		}

		const size_t extN = delim - ext;

		if( srcExtN != extN ) {
			continue;
		}

		if( CaseCmp( srcExt, ext, extN ) ) {
			return true;
		}
	}

	return false;
}

}

void Ax::System::CFileList::FilterExtension( const char *ext )
{
	size_t i = 0;
	while( i < m_Files.Num() ) {
		if( !HasExtension( m_Files[ i ], ext ) ) {
			size_t j = i + 1;
			while( j < m_Files.Num() && !HasExtension( m_Files[ j ], ext ) ) {
				++j;
			}

			m_Files.Remove( i, j - i );
			continue;
		}

		++i;
	}
}

bool Ax::System::IsDir( const char *filename )
{
	struct stat s;

	if( stat( filename, &s ) != 0 ) {
		return false;
	}

	return ( s.st_mode & S_IFDIR ) != 0;
}
time_t Ax::System::GetModifiedTime( const char *path )
{
	struct stat s;

	if( stat( path, &s ) != 0 ) {
		return -1;
	}

	return s.st_mtime;
}
void Ax::System::MakeDirs( const char *dirs )
{
	//
	//	! This is old code !
	//	Just ignore bad practices, mmkay?
	//

	const char *p;
	char buf[ PATH_MAX ], *path;

	// ignore the root directory
	if( dirs[ 0 ] == '/' ) {
		buf[ 0 ] = dirs[ 0 ];
		path = &buf[ 1 ];
		p = &dirs[ 1 ];
	} else if( dirs[ 1 ] == ':' && dirs[ 2 ] == '/' ) {
		buf[ 0 ] = dirs[ 0 ];
		buf[ 1 ] = dirs[ 1 ];
		buf[ 2 ] = dirs[ 2 ];
		path = &buf[ 3 ];
		p = &dirs[ 3 ];
	} else {
		path = &buf[ 0 ];
		p = &dirs[ 0 ];
	}

	errno = 0;

	// make each directory, one by one
	while( 1 ) {
		if( *p == '/' || *p == 0 ) {
			*path = 0;

#ifdef _WIN32
# ifdef _MSC_VER
			if( _mkdir( buf ) != 0 ) {
				if( errno == 0 ) {
					errno = EACCES;
				}
			}
# else
			mkdir( buf );
# endif
#else
			mkdir( buf, 0740 );
#endif
			if( errno && errno != EEXIST ) {
				BasicErrorf( "Couldn't create directory: %s", buf );
				break;
			}

			errno = 0;

			if( !( *path++ = *p++ ) ) {
				return;
			} else if( *p == '\0' ) {
				// handle a '/' ending
				return;
			}
		} else {
			*path++ = *p++;
		}

		if( path == &buf[ sizeof( buf )-1 ] ) {
			BasicErrorf( "path is too long" );
		}
	}
}

bool Ax::System::SetDir( const char *path )
{
	AX_ASSERT_NOT_NULL( path );

#ifdef _WIN32
	static const int maxwinpath = MAX_PATH + 1;
	uint16 winpath[ maxwinpath ];

	String inpath;
	if( !AX_VERIFY( inpath.Assign( path ) ) ) {
		return false;
	}

	if( inpath.EndsWith( "\\" ) || inpath.EndsWith( "/" ) ) {
		inpath.Remove( -1, 1 );
	}

	if( !AX_VERIFY( inpath.ConvertUTF16( winpath, maxwinpath ) == true ) ) {
		return false;
	}

	if( !SetCurrentDirectoryW( ( const wchar_t * )winpath ) ) {
		return false;
	}

	return true;
#else
	if( !AX_VERIFY( chdir( path ) == 0 ) ) {
		return false;
	}

	return true;
#endif
}
bool Ax::System::GetDir( char *path, size_t maxpath )
{
	AX_ASSERT_NOT_NULL( path );
	AX_ASSERT( maxpath > 1 );

#ifdef _WIN32
	static const int maxwinpath = MAX_PATH + 1;
	wchar_t winpath[ maxwinpath ];

	if( !AX_VERIFY( GetCurrentDirectoryW( maxwinpath, winpath ) != 0 ) ) {
		return false;
	}

	String temp;
	temp.AssignUTF16( winpath );
	temp.BackSlashesToForwardSlashes();
	temp.AppendDirectorySeparator();

	StrCpy( path, maxpath, temp.CString() );
	return true;
#else
	return AX_VERIFY( getcwd( path, maxpath ) != nullptr );
#endif
}

namespace
{

	struct DirStack
	{
		Ax::TList< Ax::String > dirs;

		static DirStack &GetInstance()
		{
			static DirStack instance;
			return instance;
		}
		~DirStack()
		{
			using namespace Ax::System;
			if( !dirs.IsEmpty() ) {
				AX_VERIFY( SetDir( *dirs.begin() ) == true );
				dirs.Clear();
			}
		}

	private:
		DirStack()
		{
		}
	};

	inline DirStack &DS()
	{
		return DirStack::GetInstance();
	}

}

bool Ax::System::PushDir( const char *path )
{
	char curdir[ MAX_PATH + 1 ];
	if( !GetDir( curdir, sizeof( curdir ) ) ) {
		return false;
	}

	auto iter = DS().dirs.AddTail();
	if( !iter->Assign( curdir ) ) {
		DS().dirs.Remove( iter );
		return false;
	}

	if( !SetDir( path ) ) {
		DS().dirs.Remove( iter );
		return false;
	}

	return true;
}
void Ax::System::PopDir()
{
	auto iter = DS().dirs.Last();
	if( iter == DS().dirs.end() ) {
		return;
	}

	AX_VERIFY( SetDir( *iter ) == true );
}

static bool EnumFileTree_r( Ax::System::CFileList &fl, const Ax::String &basePath, size_t curDepth )
{
	using namespace Ax;
	using namespace Ax::System;

	static const size_t maxDepth = 64;
	if( !AX_VERIFY( curDepth < maxDepth ) ) {
		return false;
	}

	String inpath;
	if( !AX_VERIFY( inpath.Assign( basePath ) == true ) ) {
		return false;
	}

	if( inpath.Len() > 0 ) {
#ifdef _WIN32
		inpath.ForwardSlashesToBackSlashes();
		if( inpath.EndsWith( "\\" ) ) {
			inpath.Remove( -1, 1 );
		}
#else
		if( inpath.EndsWith( "/" ) ) {
			inpath.Remove( -1, 1 );
		}
#endif
	} else {
		inpath = ".";
	}

	String tempPath;
	if( inpath != "." ) {
		if( !AX_VERIFY( tempPath.Assign( inpath ) == true ) ) {
			return false;
		}
		if( !AX_VERIFY( tempPath.AppendDirectorySeparator() == true ) ) {
			return false;
		}

#ifdef _WIN32
		tempPath.BackSlashesToForwardSlashes();
#endif
	}

	String newBasePath;
	if( !AX_VERIFY( newBasePath.Reserve( MAX_PATH ) == true ) ) {
		return false;
	}

	// Open the directory
	DIR *d = opendir( inpath );
	if( !d ) {
		Errorf( basePath, "Could not open directory" );
		return false;
	}

	// Enumerate the files in the directory
	bool r = true;
	for(;;) {
		// Read next directory entry
		struct dirent *dp = readdir( d );
		if( !dp ) {
			break;
		}

		// Skip '.' and '..'
		if( dp->d_name[ 0 ] == '.' && ( dp->d_name[ 1 ] == '\0' || ( dp->d_name[ 1 ] == '.' && dp->d_name[ 2 ] == '\0' ) ) ) {
			continue;
		}

		// Set the new base path
		newBasePath.Assign( tempPath );
		newBasePath.Append( dp->d_name );

		// Handle files
		if( !IsDir( newBasePath ) ) {
			fl.AppendFile( newBasePath );
			continue;
		}

		// Don't enter directory if at maximum depth
		if( curDepth == maxDepth ) {
			Warnf( newBasePath, "Not entering path because max depth (%i) has been reached", ( int )maxDepth );
			continue;
		}

		// Add files from this directory
		if( !EnumFileTree_r( fl, newBasePath, curDepth + 1 ) ) {
			r = false;
			break;
		}
	}

	// Close the directory
	closedir( d );
	d = nullptr;

	// Done!
	return r;
}
bool Ax::System::EnumFileTree( CFileList &fl, const char *basePath )
{
	return EnumFileTree_r( fl, basePath, 0 );
}
bool Ax::System::EnumFileTree( CFileGraph &fg, const char *basePath )
{
	const char *const inpath = basePath != nullptr && *basePath != '\0' ? basePath : ".";

	// Open the directory
	DIR *d = opendir( inpath );
	if( !d ) {
		Errorf( inpath, "Could not open directory" );
		return false;
	}

	// Read each entry
	String newPath;
	bool r = true;
	for(;;) {
		// Read the next entry
		struct dirent *const dp = readdir( d );
		if( !dp ) {
			break;
		}
		
		// Skip '.' and '..'
		if( dp->d_name[ 0 ] == '.' && ( dp->d_name[ 1 ] == '\0' || ( dp->d_name[ 1 ] == '.' && dp->d_name[ 2 ] == '\0' ) ) ) {
			continue;
		}

		// Construct the new path
		newPath.Empty();
		if( inpath == basePath ) {
			newPath.Assign( basePath );
		}
		newPath.AppendPath( dp->d_name );

		// Adding files is just addition of string
		if( !IsDir( newPath ) ) {
			if( !fg.AddFile( dp->d_name ) ) {
				r = false;
				break;
			}

			continue;
		}

		// Add a whole directory
		CFileGraph *const graph = fg.AddDirectory( dp->d_name );
		if( !graph || !EnumFileTree( *graph, newPath ) ) {
			r = false;
			break;
		}

		// Ignore the directory if it has nothing in it
		if( !( graph->NumDirectories() + graph->NumFiles() ) ) {
			fg.RemoveLastDirectory( graph );
		}
	}

	// Close the directory
	closedir( d );
	d = nullptr;

	// Done
	return true;
}

// Retrieve a unique identifier for a file
Ax::uint64 Ax::System::GetUniqueFileId( const char *filename )
{
#if defined( _WIN32 )
	// Convert UTF-8 to UTF-16
	Ax::uint16 path[ MAX_PATH ];
	if( !String( filename ).ConvertUTF16( path, MAX_PATH ) ) {
		return 0;
	}

	// Open the file (just the attributes, no data)
	HANDLE h = CreateFileW( ( const wchar_t * )path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if( !h || h == INVALID_HANDLE_VALUE ) {
		return 0;
	}

	// Get information about the file (contains unique identifier)
	BY_HANDLE_FILE_INFORMATION info;
	if( !GetFileInformationByHandle( h, &info ) ) {
		CloseHandle( h );
		return 0;
	}

	// Store the identifier and close the file
	const uint64 index = ( ( ( uint64 )info.nFileIndexHigh )<<32 ) | ( uint64 )info.nFileIndexLow;
	CloseHandle( h );

	// Done
	return index;
#elif ( defined( __linux__ ) || defined( __linux ) || defined( linux ) ) && 0 // ← remove "&& 0" when implementation added
	//
	//	TODO: Add a Linux-specific unique-file-identifier retrieval function here
	//
#elif defined( __APPLE__ ) && 0 // ← remove "&& 0" when implementation added
	//
	//	TODO: Add a Mac OS X-specific unique-file-identifier retrieval function here
	//
#else
	//
	//	Unknown or unhandled platform. Create a hash of the file's absolute path
	//

	char path[ PATH_MAX + 1 ];
	if( GetAbsolutePath( path, sizeof( path ), filename ) == nullptr ) {
		return 0;
	}

	uint64 hash = 0xFACEBEEF;
	uint64 n = 0;

	for( size_t i = 0; path[ i ] != '\0'; ++i ) {
		uint64_t x = uint64_t( path[i] );

		hash  -= n;
		hash <<= 1;
		hash  ^= x<<4 | x>>2 | x<<2;
		hash   = hash<<4 | ( hash ^ hash>>4 );
	}

	return hash;
#endif
}

// Read from a file
bool Ax::System::ReadFile( String &dst, const char *filename, EEncoding InEncoding )
{
#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	FILE *fp = NULL;
	if( fopen_s( &fp, filename, "rb" ) != 0 ) {
		Errorf( filename, "Could not open to read" );
		return false;
	}
#else
	FILE *const fp = fopen( filename, "rb" );
	if( !fp ) {
		Errorf( filename, "Could not open to read" );
		return false;
	}
#endif

	AX_ASSERT_NOT_NULL( fp );

	fseek( fp, 0, SEEK_END );
#ifdef _WIN32
# ifdef _WIN64
	const size_t n = ( size_t )_ftelli64( fp );
# else
	const long long tempn = _ftelli64( fp );
	if( tempn < 0 || tempn > 0xFFFFFFFF ) {
		fclose( fp );
		Errorf( filename, "File is too large" );
		return false;
	}

	const size_t n = ( size_t )tempn;
# endif
#else
	const size_t n = ( size_t )ftell( fp );
#endif

	fseek( fp, 0, SEEK_SET );

	TArray< uint8 > bytes;

	if( !bytes.Reserve( n + 4 - n%4 ) ) {
		fclose( fp );
		Errorf( filename, "Could not reserve enough memory to fit files contents (%i byte%s)", n, n == 1 ? "" : "s" );
		return false;
	}

	bytes.Resize( n );
	uint8 *const buf = bytes.Pointer();

	size_t r = 0;
	while( r < n ) {
		const size_t x = fread( &buf[ r ], 1, n - r, fp );
		if( !x ) {
			break;
		}

		r += x;
	}

	fclose( fp );

	String tmp;
	if( !tmp.ConvertFromEncoding( bytes, InEncoding ) ) {
		Errorf( filename, "Could not convert encoding" );
		return false;
	}

	if( !dst.Append( tmp ) ) {
		Errorf( filename, "Not enough memory" );
		return false;
	}

	return true;
}
// Write to a file
bool Ax::System::WriteFile( const char *filename, const char *text )
{
#if defined( _MSC_VER ) && defined( __STDC_WANT_SECURE_LIB__ )
	FILE *fp = NULL;
	if( fopen_s( &fp, filename, "wb" ) != 0 || !fp ) {
		Errorf( filename, "Could not open to read" );
		return false;
	}
#else
	FILE *const fp = fopen( filename, "wb" );
	if( !fp ) {
		Errorf( filename, "Could not open to read" );
		return false;
	}
#endif

	if( text != nullptr ) {
		fprintf( fp, "%s", text ); //TODO: check for an error here
	}

	fclose( fp );
	return true;
}

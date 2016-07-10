#pragma once

#include "../Core/Assert.hpp"
#include "../Core/String.hpp"
#include "../Collections/Array.hpp"

#include <time.h>

namespace Ax { namespace System {

	class CFileList
	{
	public:
		inline CFileList()
		: m_Files()
		{
		}
		inline CFileList( const CFileList &fl )
		: m_Files( fl.m_Files )
		{
		}
		inline ~CFileList()
		{
		}

		inline void Clear()
		{
			m_Files.Clear();
		}
		inline bool IsEmpty() const
		{
			return m_Files.IsEmpty();
		}

		inline size_t Num() const
		{
			return m_Files.Num();
		}
		inline size_t GetNumFiles() const
		{
			return m_Files.Num();
		}
		inline const char *GetFile( size_t index ) const
		{
			if( index >= m_Files.Num() ) {
				return NULL;
			}

			return m_Files[ index ];
		}

		inline void AppendFile( const char *filename )
		{
			m_Files.Append( filename );
		}
		inline void AppendList( const CFileList &files )
		{
			for( size_t i = 0; i < files.Num(); ++i ) {
				m_Files.Append( files.GetFile( i ) );
			}
		}

		inline void BackSlashesToForwardSlashes()
		{
			const size_t n = Num();
			for( size_t i = 0; i < n; ++i ) {
				m_Files[ i ].BackSlashesToForwardSlashes();
			}
		}

		inline const char *operator[]( int i ) const
		{
			return GetFile( i );
		}

		inline CFileList *operator->()
		{
			return this;
		}
		inline const CFileList *operator->() const
		{
			return this;
		}

		void FilterExtension( const char *ext );

	private:
		TArray< String >			m_Files;
	};
	
	class CFileGraph
	{
	public:
		inline CFileGraph()
		: m_pParent( nullptr )
		, m_Name()
		, m_Directories()
		, m_Files()
		{
		}
		inline CFileGraph( const CFileGraph &x )
		: m_pParent( nullptr )
		, m_Name()
		, m_Directories()
		, m_Files()
		{
			Copy( x );
		}
		inline CFileGraph &operator=( const CFileGraph &x )
		{
			Copy( x );
			return *this;
		}
		inline ~CFileGraph()
		{
			Clear();
		}

		inline CFileGraph *Parent()
		{
			return m_pParent;
		}
		inline const CFileGraph *Parent() const
		{
			return m_pParent;
		}

		inline void Clear()
		{
			for( size_t i = 0; i < m_Directories.Num(); ++i ) {
				delete m_Directories[ i ];
				m_Directories[ i ] = nullptr;
			}
			m_Directories.Clear();

			m_Files.Clear();

			if( m_pParent != nullptr ) {
				for( size_t i = 0; i < m_pParent->m_Directories.Num(); ++i ) {
					if( m_pParent->m_Directories[ i ] == this ) {
						m_pParent->m_Directories.Remove( i, 1 );
						break;
					}
				}

				m_pParent = nullptr;
			}
		}

		inline void Copy( const CFileGraph &x )
		{
			// Clear out existing data
			Clear();

#if 0
			// Need to copy the parents too
			CFileGraph *newParent = nullptr;
			CFileGraph **ppParent = &m_pParent;
			const CFileGraph *prnt = x.m_pParent;
			while( prnt != nullptr ) {
				CFileGraph *newParent = new CFileGraph();
				if( !AX_VERIFY_NOT_NULL( newParent ) ) {
					return;
				}

				newParent->CopySingle( *prnt );

				AX_ASSERT_NOT_NULL( ppParent );
				*ppParent = newParent;
				ppParent = &newParent->m_pParent;

				prnt = prnt->m_pParent;
			}
#else
			// Just copy the parent pointer because otherwise memory will go missing
			m_pParent = x.m_pParent;
#endif
			CopySingle( x );

			for( int i = 0; i < x.NumDirectories(); ++i ) {
				const CFileGraph *const srcdir = x.Directory( i );
				AX_ASSERT_NOT_NULL( srcdir );

				AX_STATIC_SUPPRESS( 6011 )
				CFileGraph *const dir = AddDirectory( srcdir->Name() );
				if( !AX_VERIFY_NOT_NULL( dir ) ) {
					break;
				}

				dir->Copy( *srcdir );
			}

			for( int i = 0; i < x.NumFiles(); ++i ) {
				AddFile( x.File( i ) );
			}
		}
		inline void CopySingle( const CFileGraph &x )
		{
			m_Name = x.m_Name;
			m_Directories = x.m_Directories;
			m_Files = x.m_Files;
		}

		inline const String &Name() const
		{
			return m_Name;
		}
		inline String FullName() const
		{
			String temp;

			if( m_pParent != nullptr ) {
				temp = m_pParent->FullName();
			}

			temp.AppendPath( m_Name );
			return temp;
		}

		inline int NumDirectories() const
		{
			return ( int )m_Directories.Num();
		}
		inline int NumFiles() const
		{
			return ( int )m_Files.Num();
		}

		inline CFileGraph *Directory( int i )
		{
			AX_ASSERT( i >= 0 && i < NumDirectories() );
			return m_Directories[ i ];
		}
		inline const CFileGraph *Directory( int i ) const
		{
			AX_ASSERT( i >= 0 && i < NumDirectories() );
			return m_Directories[ i ];
		}

		inline String &File( int i )
		{
			AX_ASSERT( i >= 0 && i < NumFiles() );
			return m_Files[ i ];
		}
		inline const String &File( int i ) const
		{
			AX_ASSERT( i >= 0 && i < NumFiles() );
			return m_Files[ i ];
		}

		inline String FilePath( int i ) const
		{
			AX_ASSERT( i >= 0 && i < NumFiles() );
			String temp = FullName();
			temp.AppendPath( m_Files[ i ] );
			return temp;
		}

		inline CFileGraph *AddDirectory( const char *name )
		{
			CFileGraph *graph = new CFileGraph();
			if( !AX_VERIFY_NOT_NULL( graph ) ) {
				return nullptr;
			}

			graph->m_pParent = this;
			graph->m_Name = name;

			if( !AX_VERIFY( m_Directories.Append( graph ) == true ) ) {
				delete graph;
				return nullptr;
			}

			return graph;
		}
		inline bool AddFile( const char *name )
		{
			return AX_VERIFY( m_Files.Append( name ) == true );
		}

		inline void RemoveLastDirectory( CFileGraph *lastDirectory )
		{
			AX_ASSERT( NumDirectories() > 0 );

			const int i = ( int )( m_Directories.Num() - 1 );
			AX_ASSERT( Directory( i ) == lastDirectory );

			delete m_Directories[ i ];
			m_Directories[ i ] = nullptr;

			m_Directories.Resize( i );
		}

	private:
		CFileGraph *				m_pParent;
		String						m_Name;
		TArray< CFileGraph * >		m_Directories;
		TArray< String >			m_Files;
	};


	bool IsDir( const char *filename );
	time_t GetModifiedTime( const char *path );
	void MakeDirs( const char *dirs );
	bool EnumFileTree( CFileList &fl, const char *basePath );
	bool EnumFileTree( CFileGraph &fg, const char *basePath );
	
	bool SetDir( const char *path );
	bool GetDir( char *path, size_t maxpath );

	template< size_t tMaxPath >
	inline bool GetDir( char( &path )[ tMaxPath ] )
	{
		return GetDir( path, tMaxPath );
	}
	inline Ax::String GetDir()
	{
		char szPath[ PATH_MAX + 1 ];
		if( !GetDir( szPath, sizeof( szPath ) ) ) {
			return Ax::String();
		}

		return Ax::String( szPath );
	}

	bool PushDir( const char *path );
	void PopDir();

	uint64 GetUniqueFileId( const char *filename );
	bool ReadFile( String &dst, const char *filename, EEncoding InEncoding = EEncoding::Unknown );
	bool WriteFile( const char *filename, const char *text );

}}

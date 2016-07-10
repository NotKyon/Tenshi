#include "_PCH.hpp"
#include "Directory.hpp"
#include "File.hpp"

TE_FUNC bool TE_CALL FS_Init( TenshiRuntimeGlob_t *pGlob )
{
	Tenshi::Init( pGlob );

	if( !CDirectory::Init() ) {
		return false;
	}
	if( !CFile::Init() ) {
		return false;
	}

	return true;
}
TE_FUNC void TE_CALL FS_Fini()
{
	CFile::Fini();
	CDirectory::Fini();
}

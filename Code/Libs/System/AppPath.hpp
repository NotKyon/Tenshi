#pragma once

#include <stddef.h>
#include "../Core/String.hpp"

namespace Ax { namespace System {

	char *GetAppPath( char *buff, size_t n );
	char *GetAppDir( char *buff, size_t n );
	
	template< size_t _Size_ >
	inline char *GetAppPath( char( &buff )[ _Size_ ] )
	{
		return GetAppPath( buff, _Size_ );
	}
	template< size_t _Size_ >
	inline char *GetAppDir( char( &buff )[ _Size_ ] )
	{
		return GetAppDir( buff, _Size_ );
	}

	inline String GetAppPath()
	{
		char szBuf[ 512 ];
		if( !GetAppPath( szBuf, sizeof( szBuf ) ) ) {
			return String();
		}

		return String( szBuf );
	}
	inline String GetAppDir()
	{
		char szBuf[ 512 ];
		if( !GetAppDir( szBuf, sizeof( szBuf ) ) ) {
			return String();
		}

		return String( szBuf );
	}

}}

#pragma once

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# include <winerror.h>
# undef min
# undef max
#endif

#include <Core/Assert.hpp>
#include <Core/Manager.hpp>
#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Core/Types.hpp>
#include <errno.h>

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C					extern "C"
# else
#  define EXTERN_C
# endif
#endif

#ifndef TE_FUNC
# ifdef _WIN32
#  define TE_FUNC					EXTERN_C __declspec(dllexport)
# else
#  define TE_FUNC					EXTERN_C
# endif
#endif

#ifndef TE_CALL
# ifdef _WIN32
#  define TE_CALL					__cdecl
# else
#  define TE_CALL
# endif
#endif

#include "../Runtime/TenshiRuntime.h"

struct ErrorCode
{
	Ax::int32						Value;

	inline ErrorCode &SetErrno()
	{
		Value = errno;
		return *this;
	}

#ifdef _WIN32
	inline ErrorCode &SetHResult( HRESULT hr )
	{
		if( SUCCEEDED( hr ) ) {
			Value = 0;
			return *this;
		}
	}
	inline ErrorCode &SetLastError_MSWin()
	{
		return SetSystemError_MSWin( GetLastError() );
	}
	inline ErrorCode &SetSystemError_MSWin( DWORD dwCode )
	{
		switch( dwCode )
		{
# define S_(X_,Y_) case X_: Value = Y_; return *this
		S_(0,0);
		S_(ERROR_INVALID_FUNCTION,			EINVAL);
		S_(ERROR_INVALID_PARAMETER,			EINVAL);
		S_(ERROR_FILE_NOT_FOUND,			EEXIST);
		S_(ERROR_PATH_NOT_FOUND,			EEXIST);
		S_(ERROR_TOO_MANY_OPEN_FILES,		ENOMEM);
		S_(ERROR_ACCESS_DENIED,				EPERM);
		S_(ERROR_INVALID_HANDLE,			EINVAL);
		S_(ERROR_ARENA_TRASHED,				EIO);
		S_(ERROR_NOT_ENOUGH_MEMORY,			ENOMEM);
		S_(ERROR_OUTOFMEMORY,				ENOMEM);
# undef S_

		default:
			break;
		}

		Value = EINVAL;
		return *this;
	}
#endif

	inline operator bool() const
	{
		return Value != 0;
	}
	inline bool operator!() const
	{
		return Value == 0;
	}
};

#define UNIMPLEMENTED()\
	do {\
		static bool didLog = false;\
		if( !didLog ) {\
			didLog = true;\
			TE_TRACE( "Unimplemented" );\
		}\
	} while(false)

using Tenshi::StrRef;

inline Tenshi::StrRef MakeStrRef( const Ax::String &InStr )
{
	return Tenshi::StrRef( InStr.CString(), InStr.Len() );
}

inline void LogError( ErrorCode &EC )
{
	char errbuf[512];
#ifdef _MSC_VER
	strerror_s( errbuf, EC.Value );
	errbuf[ sizeof( errbuf ) - 1 ] = '\0';
#else
	strncpy( errbuf, strerror( EC.Value ), sizeof( errbuf ) - 1 );
	errbuf[ sizeof( errbuf ) - 1 ] = '\0';
#endif

	Tenshi::Logf( TELOG_ERROR | TENSHI_FACILITY | TELOG_C_APP, TENSHI_MODNAME,
		nullptr, 0, nullptr, nullptr, "%s [%i]", errbuf, EC.Value );
}

TENSHI_NORETURN inline void SystemError( ErrorCode &EC )
{
	LogError( EC );
	Tenshi::RuntimeError( TE_ERR_SYSTEM );
}

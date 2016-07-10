#pragma once

#if AX_THREAD_MODEL != AX_THREAD_MODEL_WINDOWS || !defined( AX_THREAD_MODEL )
# error AX_THREAD_MODEL must be set to AX_THREAD_MODEL_WINDOWS to include this header!
#endif

#ifndef _WIN32
# error AX_THREAD_MODEL_WINDOWS is only for the Windows platform!
#endif

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#undef min
#undef max
#undef Yield
#undef AddJob

#define AX_THREAD_CALL __stdcall
namespace Ax
{

	typedef DWORD threadResult_t;

}

#include <intrin.h>
#ifdef _MSC_VER
# pragma intrinsic( _ReadWriteBarrier )
#endif

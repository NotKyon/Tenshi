#pragma once

#if AX_THREAD_MODEL != AX_THREAD_MODEL_PTHREAD || !defined( AX_THREAD_MODEL )
# error AX_THREAD_MODEL must be set to AX_THREAD_MODEL_PTHREAD to include this header!
#endif

#include <pthread.h>

#define AX_THREAD_CALL /*intentionally blank*/

namespace Ax
{

	typedef void *threadResult_t;

}

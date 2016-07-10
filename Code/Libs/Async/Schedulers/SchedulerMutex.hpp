#pragma once

#include "../Mutex.hpp"
#include "../QuickMutex.hpp"

#define AX_ASYNC_MUTEXTYPE_CRITICALSECTION				1
#define AX_ASYNC_MUTEXTYPE_QUICKMUTEX					2

/// @def AX_ASYNC_SCHEDULER_MUTEXTYPE
/// Specifies the specific type of mutex to use internally
#ifndef AX_ASYNC_SCHEDULER_MUTEXTYPE
# define AX_ASYNC_SCHEDULER_MUTEXTYPE					AX_ASYNC_MUTEXTYPE_QUICKMUTEX
#endif

namespace Ax { namespace Async {

	// Configuration for the scheduler mutex
#if AX_ASYNC_SCHEDULER_MUTEXTYPE == AX_ASYNC_MUTEXTYPE_CRITICALSECTION
	typedef CMutex SchedulerMutex;
#elif AX_ASYNC_SCHEDULER_MUTEXTYPE == AX_ASYNC_MUTEXTYPE_QUICKMUTEX
	typedef CQuickMutex SchedulerMutex;
#else
# error AX_ASYNC_SCHEDULER_MUTEXTYPE: Unknown mutex type
#endif
	typedef LockGuard< SchedulerMutex > SchedulerMutexLockGuard;

}}

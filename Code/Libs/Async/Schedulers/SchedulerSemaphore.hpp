#pragma once

#include "../Semaphore.hpp"
#include "../QuickSemaphore.hpp"

#define AX_ASYNC_SEMAPHORETYPE_SYSTEM					1
#define AX_ASYNC_SEMAPHORETYPE_QUICK					2

/// @def AX_ASYNC_SCHEDULER_SEMAPHORETYPE
/// Specifies the specific type of semaphore to use internally
///
/// AX_ASYNC_SEMAPHORETYPE_SYSTEM
/// Uses the default system semaphore (Ax::Async::CSemaphore), which will enable
/// threads to sleep while waiting for data
///
/// AX_ASYNC_SEMAPHORETYPE_QUICK
/// Uses the quick semaphore (Ax::Async::CQuickSemaphore), which avoids putting
/// threads to sleep
#ifndef AX_ASYNC_SCHEDULER_SEMAPHORETYPE
# define AX_ASYNC_SCHEDULER_SEMAPHORETYPE				AX_ASYNC_SEMAPHORETYPE_QUICK
#endif

namespace Ax { namespace Async {

	// Configuration for the scheduler mutex
#if AX_ASYNC_SCHEDULER_SEMAPHORETYPE == AX_ASYNC_SEMAPHORETYPE_SYSTEM
	typedef CSemaphore SchedulerSemaphore;
#elif AX_ASYNC_SCHEDULER_SEMAPHORETYPE == AX_ASYNC_SEMAPHORETYPE_QUICK
	typedef CQuickSemaphore SchedulerSemaphore;
#else
# error AX_ASYNC_SCHEDULER_SEMAPHORETYPE: Unknown semaphore type
#endif

}}

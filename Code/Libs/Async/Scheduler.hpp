#pragma once

#ifndef AX_ASYNC_SCHEDULER_VERSION
# define AX_ASYNC_SCHEDULER_VERSION 4
#endif

#if AX_ASYNC_SCHEDULER_VERSION < 1 || AX_ASYNC_SCHEDULER_VERSION > 4
# error Unknown scheduler version
#endif

#if AX_ASYNC_SCHEDULER_VERSION == 1
# include "Schedulers/Scheduler01.hpp"
#elif AX_ASYNC_SCHEDULER_VERSION == 2
# include "Schedulers/Scheduler02.hpp"
#elif AX_ASYNC_SCHEDULER_VERSION == 3
# include "Schedulers/Scheduler03.hpp"
#elif AX_ASYNC_SCHEDULER_VERSION == 4
# include "Schedulers/Scheduler04.hpp"
#endif

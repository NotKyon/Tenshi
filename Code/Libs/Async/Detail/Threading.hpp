#pragma once

#define AX_THREAD_MODEL_WINDOWS 1
#define AX_THREAD_MODEL_PTHREAD 2

#ifndef AX_THREAD_MODEL
# ifdef _WIN32
#  define AX_THREAD_MODEL AX_THREAD_MODEL_WINDOWS
# else
#  define AX_THREAD_MODEL AX_THREAD_MODEL_PTHREAD
# endif
#endif

#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
# include "Threading_Windows.hpp"
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
# include "Threading_PThread.hpp"
#else
# error AX_THREAD_MODEL: Unhandled threading model
#endif

#pragma once

#include "Detail/Threading.hpp"
#undef Yield

#include "../Platform/Platform.hpp"
#include "../Core/Types.hpp"
#if AX_DEBUG_ENABLED
# include "../Core/String.hpp"
#endif

#ifndef AX_ASYNC_MSWIN_SWITCHTOTHREAD_ENABLED
# if defined( _WIN32 ) && !( AX_PLATFORM & AX_PLATFORM_STORE )
#  define AX_ASYNC_MSWIN_SWITCHTOTHREAD_ENABLED 1
# else
#  define AX_ASYNC_MSWIN_SWITCHTOTHREAD_ENABLED 0
# endif
#endif

#if AX_ASYNC_MSWIN_SWITCHTOTHREAD_ENABLED && defined( _WIN32 )
// SwitchToThread() might not be declared in Windows.h depending on the user's
// environment, so declare here (if it's needed)
extern "C" AX_DLLIMPORT BOOL WINAPI SwitchToThread();
#endif

namespace Ax { namespace Async {

	struct SThreadTimes
	{
		// Time (since app start) the thread was started [microseconds]
		uint64						EnterTime;
		// Time (since app start) the thread was terminated (0 if still active) [microseconds]
		uint64						LeaveTime;
		// Time the thread has spent in kernel mode [microseconds]
		uint64						KernelTime;
		// Time the thread has spent in user mode [microseconds]
		uint64						UserTime;
	};

	class CThread
	{
	public:
		enum class EPriority
		{
			Idle,
			VeryLow,
			Low,
			Normal,
			High,
			VeryHigh,
			Critical
		};

									// Yield processing time to other threads
		inline static void			Yield				()
									{
#ifdef _WIN32
# if AX_ASYNC_MSWIN_SWITCHTOTHREAD_ENABLED
										SwitchToThread();
# else
										Sleep( 0 );
# endif
#else
										sched_yield();
#endif
									}

									// Constructor
									CThread				();
									// Destructor
		virtual						~CThread				();

									// Set the priority of the thread (returns previous priority)
		EPriority					SetPriority			( EPriority prio );
									// Retrieve the current priority of the thread
		EPriority					GetPriority			() const;

									// Set the name of the thread (call before starting the thread) [DEBUG ONLY]
		void						SetName				( const char *name );
									// Retrieve the name of this thread
		const char *				GetName				() const;

									// Start a thread
		bool						Start				();
									// Hint that the thread should quit
		void						SignalStop			();
									// Set the thread's state to "quit" and exit the thread
		void						Stop				();
									// Retrieve whether the thread's state is set to "quit"
		bool						IsQuitting			() const;
									// Determine whether the thread is currently running
		bool						IsRunning			() const;
									// Retrieve the exit-value from the thread (only valid if !IsRunning())
		int							GetExitCode			() const;
									// Retrieve thread times
		SThreadTimes &				GetTimes			( SThreadTimes &OutTimes ) const;
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
									// Retrieve the native handle to the thread [Windows]
		inline HANDLE				GetNative_MSWin		() const
									{
										return m_hThread;
									}
#endif

	protected:
									// User-provided thread running callback
		virtual int					OnRun				() = 0;

	private:
#if AX_THREAD_MODEL == AX_THREAD_MODEL_WINDOWS
		HANDLE						m_hThread;
		DWORD						m_uThreadId;
#elif AX_THREAD_MODEL == AX_THREAD_MODEL_PTHREAD
		pthread_t					m_Thread;
		int                         m_ThreadId;
#else
# error AX_THREAD_MODEL( CThread ): Unhandled thread model
#endif
		volatile bool				m_bTerminate;
		int							m_iExitValue;
#if AX_DEBUG_ENABLED
		String						m_Name;
#endif

		static threadResult_t AX_THREAD_CALL Main_f( void *parm );
	};

}}

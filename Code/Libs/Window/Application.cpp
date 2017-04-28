#include "Application.hpp"
#include "../Core/Types.hpp"
#include "../Core/Assert.hpp"
#include "../Platform/Platform.hpp"

#if AX_OS(Windows)
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# undef min
# undef max
#endif

namespace Ax { namespace Window {

	static bool						g_bReceivedQuit = false;

#if AX_OS(Windows)
	namespace Windows
	{

		void ProcessMessage( MSG &Message )
		{
			if( Message.message == WM_QUIT ) {
				g_bReceivedQuit = true;
			}

			TranslateMessage( &Message );
			DispatchMessageW( &Message );
		}

	}
#endif

	void SubmitQuitEvent()
	{
#if AX_OS(Windows)
		PostQuitMessage( EXIT_SUCCESS );
#endif
	}
	bool ReceivedQuitEvent()
	{
		return g_bReceivedQuit;
	}

	bool WaitForAndProcessEvent()
	{
#if AX_OS(Windows)
		MSG Msg;

		BOOL Result = GetMessageW( &Msg, NULL, 0, 0 );
		if( Result <= FALSE ) {
			g_bReceivedQuit = Result == FALSE;
			return false;
		}

		Windows::ProcessMessage( Msg );
		return true;
#else
		AX_ASSERT_MSG( false, "Unimplemented" );
		return false;
#endif
	}
	bool ProcessAllQueuedEvents()
	{
#if AX_OS(Windows)
		MSG Msg;
		bool bDidProcess = false;

		while( PeekMessageW( &Msg, NULL, 0, 0, PM_REMOVE ) ) {
			bDidProcess = true;
			Windows::ProcessMessage( Msg );
		}

		return bDidProcess;
#else
		AX_ASSERT_MSG( false, "Unimplemented" );
		return false;
#endif
	}

}}

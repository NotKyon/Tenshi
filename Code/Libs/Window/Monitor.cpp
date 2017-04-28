#include "Monitor.hpp"
#include "../Core/Assert.hpp"
#include "../Platform/Platform.hpp"

namespace Ax { namespace Window {

#if AX_OS(Windows)
	namespace Windows
	{
		static BOOL CALLBACK EnumMonitor_f( HMONITOR hMonitor, HDC hMonitorDC, LPRECT pMonitorRect, LPARAM dwData )
		{
			SDesktopInfo *const pDesktopInfo = reinterpret_cast< SDesktopInfo * >( dwData );
			AX_ASSERT_NOT_NULL( pDesktopInfo );
			if( !pDesktopInfo ) { return FALSE; }

			MONITORINFOEXW Info;
			Info.cbSize = sizeof( Info );

			if( !AX_VERIFY( GetMonitorInfoW( hMonitor, &Info ) != 0 ) ) {
				return FALSE;
			}

			const uintptr uIndex = pDesktopInfo->Monitors.Num();
			if( !AX_VERIFY( pDesktopInfo->Monitors.Resize( uIndex + 1 ) ) ) {
				return FALSE;
			}

			SMonitorInfo &Monitor = *pDesktopInfo->Monitors.Pointer( uIndex );
			Monitor.DeviceName.AssignUTF16( Info.szDevice );
			Monitor.Resolution.x = Info.rcMonitor.right - Info.rcMonitor.left;
			Monitor.Resolution.y = Info.rcMonitor.bottom - Info.rcMonitor.top;
			Monitor.WorkArea.x1 = Info.rcWork.left;
			Monitor.WorkArea.y1 = Info.rcWork.top;
			Monitor.WorkArea.x2 = Info.rcWork.right;
			Monitor.WorkArea.y2 = Info.rcWork.bottom;
			Monitor.DotsPerInch.x = GetDeviceCaps( hMonitorDC, LOGPIXELSX );
			Monitor.DotsPerInch.y = GetDeviceCaps( hMonitorDC, LOGPIXELSY );
			Monitor.bIsPrimary = ( Info.dwFlags & MONITORINFOF_PRIMARY ) != 0;
			Monitor.pNativeHandle = ( void * )hMonitor;

			if( Monitor.bIsPrimary ) {
				pDesktopInfo->PrimaryMonitor = uIndex;
			}

			if( pDesktopInfo->VirtualWorkArea.x1 > Monitor.WorkArea.x1 ) {
				pDesktopInfo->VirtualWorkArea.x1 = Monitor.WorkArea.x1;
			}
			if( pDesktopInfo->VirtualWorkArea.y1 > Monitor.WorkArea.y1 ) {
				pDesktopInfo->VirtualWorkArea.y1 = Monitor.WorkArea.y1;
			}
			if( pDesktopInfo->VirtualWorkArea.x2 < Monitor.WorkArea.x2 ) {
				pDesktopInfo->VirtualWorkArea.x2 = Monitor.WorkArea.x2;
			}
			if( pDesktopInfo->VirtualWorkArea.y2 < Monitor.WorkArea.y2 ) {
				pDesktopInfo->VirtualWorkArea.y2 = Monitor.WorkArea.y2;
			}

			return TRUE;
		}
		bool QueryDesktopInfo( SDesktopInfo &OutInfo )
		{
			// Get information about all monitors
			HDC hdcTest = CreateDCW( L"DISPLAY", nullptr, nullptr, nullptr );
			if( !AX_VERIFY( EnumDisplayMonitors( hdcTest, nullptr, &EnumMonitor_f, ( LPARAM )&OutInfo ) ) ) {
				DeleteDC( hdcTest );
				return false;
			}
			DeleteDC( hdcTest );

			// Find the launch monitor
			STARTUPINFOW StartupInfo;
			GetStartupInfoW( &StartupInfo );

			OutInfo.LaunchMonitor = OutInfo.PrimaryMonitor;
			if( ( ~StartupInfo.dwFlags & STARTF_USESTDHANDLES ) && StartupInfo.hStdOutput != NULL ) {
				for( uintptr i = 0; i < OutInfo.Monitors.Num(); ++i ) {
					if( OutInfo.Monitors[ i ].pNativeHandle != ( void * )StartupInfo.hStdOutput ) {
						continue;
					}

					OutInfo.LaunchMonitor = i;
					break;
				}
			}

			// Done
			return true;
		}
	}
#endif

	SDesktopInfo GetDesktopInfo()
	{
#if AX_OS(Windows)
		SDesktopInfo Info;
		if( !Windows::QueryDesktopInfo( Info ) ) {
			return SDesktopInfo();
		}
		return Info;
#else
		AX_ASSERT_MSG( false, "Unimplemented" );
		return SDesktopInfo();
#endif
	}

}}

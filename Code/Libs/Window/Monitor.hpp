#pragma once

#include "../Core/Types.hpp"
#include "../Core/String.hpp"
#include "../Collections/Array.hpp"
#include "../Math/Types/IntVector2.hpp"
#include "../Math/Types/Rect.hpp"

namespace Ax { namespace Window {

	/*!
	 *	Monitor Information
	 *
	 *	Information about a specific monitor
	 */
	struct SMonitorInfo
	{
		String						DeviceName;
		String						Identifier;
		Math::SIntVector2			Resolution;
		Math::SRect					WorkArea;
		Math::SIntVector2			DotsPerInch;
		bool						bIsPrimary;
		void *						pNativeHandle;
	};

	/*!
	 *	Desktop Information
	 *
	 *	Contains information about the desktop
	 */
	struct SDesktopInfo
	{
		TArray< SMonitorInfo >		Monitors;
		uintptr						PrimaryMonitor;
		uintptr						LaunchMonitor;
		Math::SRect					VirtualWorkArea;

		inline const SMonitorInfo &GetPrimaryMonitor() const
		{
			return Monitors[ PrimaryMonitor ];
		}
		inline const SMonitorInfo &GetLaunchMonitor() const
		{
			return Monitors[ LaunchMonitor ];
		}
	};

	// Retrieve information about the desktop (including all monitors)
	SDesktopInfo GetDesktopInfo();
	// Retrieve information about the primary monitor
	inline SMonitorInfo GetPrimaryMonitorInfo()
	{
		return GetDesktopInfo().GetPrimaryMonitor();
	}
	// Retrieve the resolution of the primary monitor
	inline Math::SIntVector2 GetDesktopSize()
	{
		return GetDesktopInfo().GetPrimaryMonitor().Resolution;
	}

}}

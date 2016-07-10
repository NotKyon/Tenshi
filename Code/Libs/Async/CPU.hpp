#pragma once

#include "../Core/Types.hpp"

namespace Ax { namespace Async {

	bool IsHyperthreadingAvailable();

	const char *GetCPUVendor();
	const char *GetCPUBrand();

	uint32 GetCPUCoreCount();
	uint32 GetCPUThreadCount();

	uint64 GetCPUCycles();
	void CPUPause( uint32 uDelay );

#ifndef AX_ASYNC_SPIN_CPUPAUSE_ENABLED
# define AX_ASYNC_SPIN_CPUPAUSE_ENABLED 1
#endif

	/// Default spin time to use in LocalSpin()
#if AX_ASYNC_SPIN_CPUPAUSE_ENABLED
	static const uint32 kSpinCount = 256;
#else
	static const uint32 kSpinCount = 1536;
#endif
	static const uint32 kBackoffMaxSpinCount = 16384;

	/// Spins for the given number of iterations
	///
	/// @todo Count clock cycles instead
	inline void LocalSpin( uint32 spinCount = kSpinCount )
	{
#if AX_ASYNC_SPIN_CPUPAUSE_ENABLED
		CPUPause( spinCount );
#else
		for( volatile uint32 i = 0; i < spinCount; ++i ) {
		}
#endif
	}

	/// Spins for the given number of iterations, gradually waiting longer each
	/// time (up to some maximum)
	///
	/// The number of iterations (cSpins) is incremented each time this is
	/// called
	void Backoff( uint32 &cSpins, uint32 cMaxSpins = kBackoffMaxSpinCount );

}}

#pragma once

#include "../Core/Types.hpp"

namespace Ax { namespace System {

	double Seconds();
	uint64 Microseconds();
	void Nanoseconds( int &seconds, int &nanoseconds );

	uint32 Milliseconds_LowLatency();

}}

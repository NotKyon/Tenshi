#pragma once

namespace Ax { namespace Window {

	void SubmitQuitEvent();
	bool ReceivedQuitEvent();

	bool WaitForAndProcessEvent();
	bool ProcessAllQueuedEvents();

}}

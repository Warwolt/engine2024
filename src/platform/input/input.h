#pragma once

#include <platform/input/keyboard.h>

#include <stdint.h>

namespace platform {

	struct Input {
		uint64_t delta_ms;
		bool quit_signal_received;
		Keyboard keyboard;
	};

	void read_input(Input* input);

} // namespace platform

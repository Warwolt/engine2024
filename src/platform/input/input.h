#pragma once

#include <platform/input/keyboard.h>

#include <glm/glm.hpp>

#include <stdint.h>

namespace platform {

	struct Input {
		uint64_t delta_ms = 0;
		bool quit_signal_received = false;
		Keyboard keyboard;
	};

	void read_input(Input* input);

} // namespace platform

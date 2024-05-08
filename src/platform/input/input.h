#pragma once

#include <platform/input/keyboard.h>
#include <platform/input/timing.h>

#include <glm/glm.hpp>

#include <stdint.h>

namespace platform {

	struct Mouse {
		int window_x;
		int window_y;
		int canvas_x;
		int canvas_y;
	};

	struct Input {
		uint64_t delta_ms = 0;
		bool quit_signal_received = false;
		Keyboard keyboard;
		Mouse mouse = { 0 };
	};

	void read_input(Input* input, Timer* frame_timer, glm::ivec2 window_size, glm::ivec2 canvas_size);

} // namespace platform

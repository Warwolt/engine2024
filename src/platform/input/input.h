#pragma once

#include <platform/input/keyboard.h>
#include <platform/input/timing.h>

#include <glm/glm.hpp>

#include <stdint.h>

namespace platform {

	struct Mouse {
		int x;
		int y;
		Button left_button;
		Button middle_button;
		Button right_button;
	};

	struct Input {
		uint64_t delta_ms = 0;
		bool quit_signal_received = false;
		Keyboard keyboard;
		Mouse mouse = { 0 };
	};

	void read_input(Input* input, Timer* frame_timer, glm::ivec2 window_size, glm::ivec2 canvas_size);

} // namespace platform

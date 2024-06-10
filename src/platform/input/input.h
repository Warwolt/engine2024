#pragma once

#include <platform/input/keyboard.h>
#include <platform/input/timing.h>
#include <platform/platform_api.h>
#include <platform/signal.h>
#include <platform/win32.h>

#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>

#include <stdint.h>
#include <vector>

namespace platform {

	struct Mouse {
		glm::vec2 pos;
		glm::vec2 pos_delta;
		int scroll_delta = 0;
		Button left_button;
		Button middle_button;
		Button right_button;
		Button x1_button;
		Button x2_button;
	};

	struct Input {
		bool quit_signal_received = false;

		uint64_t delta_ms = 0;
		uint64_t global_time_ms = 0;

		glm::vec2 window_resolution;
		Signal<bool> engine_is_rebuilding = false;
		ExitCode engine_rebuild_exit_code = 0;
		Signal<RunMode> mode;

		Keyboard keyboard;
		Mouse mouse;
	};

} // namespace platform

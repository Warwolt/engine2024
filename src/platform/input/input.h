#pragma once

#include <core/signal.h>
#include <platform/debug/logging.h>
#include <platform/graphics/renderer_debug.h>
#include <platform/input/keyboard.h>
#include <platform/input/timing.h>
#include <platform/os/win32.h>
#include <platform/platform_api.h>

#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>

#include <stdint.h>
#include <vector>

namespace platform {

	class Window;

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
		bool is_editor_mode = false;

		uint64_t delta_ms = 0;
		uint64_t global_time_ms = 0;

		const platform::Window* window = nullptr;
		glm::vec2 monitor_size;
		glm::vec2 window_resolution;
		core::Signal<bool> engine_is_rebuilding = false;
		ExitCode engine_rebuild_exit_code = 0;
		core::Signal<RunMode> mode;

		Keyboard keyboard;
		Mouse mouse;

		// debug
		RenderDebugData renderer_debug_data;
		const std::vector<LogEntry>* log; // may be null
	};

} // namespace platform

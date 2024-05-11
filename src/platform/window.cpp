#include <platform/window.h>

#include "window.h"
#include <platform/assert.h>

namespace platform {

	std::optional<WindowInfo> create_window(int width, int height) {
		/* Create window */
		SDL_Window* window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			SDL_WINDOW_OPENGL
		);

		if (!window) {
			LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
			return {};
		}

		int x, y;
		SDL_GetWindowPosition(window, &x, &y);

		return WindowInfo {
			.window = window,
			.size = { width, height },
			.resolution = { width, height },
			.windowed_pos = { x, y },
			.is_fullscreen = false,
		};
	}

} // namespace platform

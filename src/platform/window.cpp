#include <platform/window.h>

#include <platform/assert.h>
#include <platform/renderer.h>

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

	void destroy_window(WindowInfo window_info) {
		SDL_DestroyWindow(window_info.window);
	}

	void toggle_fullscreen(WindowInfo* window_info) {
		int display_index = SDL_GetWindowDisplayIndex(window_info->window);

		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(display_index, &display_mode);

		SDL_Rect display_bound;
		SDL_GetDisplayBounds(display_index, &display_bound);

		if (window_info->is_fullscreen) {
			window_info->is_fullscreen = false;

			/* Toggle windowed */
			SDL_SetWindowBordered(window_info->window, SDL_TRUE);
			SDL_SetWindowPosition(window_info->window, window_info->windowed_pos.x, window_info->windowed_pos.y);
			SDL_SetWindowSize(window_info->window, window_info->resolution.x, window_info->resolution.y);

			/* Update window size */
			window_info->size = window_info->resolution;
		}
		else {
			window_info->is_fullscreen = true;

			/* Save current windowed position */
			SDL_GetWindowPosition(window_info->window, &window_info->windowed_pos.x, &window_info->windowed_pos.y);

			/* Toggle fullscreen */
			SDL_SetWindowBordered(window_info->window, SDL_FALSE);
			SDL_SetWindowPosition(window_info->window, display_bound.x, display_bound.y);
			SDL_SetWindowSize(window_info->window, display_mode.w, display_mode.h);

			/* Update windowed size */
			window_info->size = glm::ivec2 { display_mode.w, display_mode.h };
		}
	}

	void change_resolution(WindowInfo* window_info, int width, int height) {
		// update resolution
		window_info->resolution.x = width;
		window_info->resolution.y = height;

		if (!window_info->is_fullscreen) {
			window_info->size.x = width;
			window_info->size.y = height;
			SDL_SetWindowSize(window_info->window, window_info->size.x, window_info->size.y);
		}
	}

} // namespace platform

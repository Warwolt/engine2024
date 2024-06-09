#include <platform/window.h>

#include <platform/assert.h>
#include <platform/renderer.h>

namespace platform {

	std::optional<Window> create_window(int width, int height, int window_flags) {
		SDL_Window* window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			window_flags | SDL_WINDOW_OPENGL
		);

		if (!window) {
			LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
			return {};
		}

		int x, y;
		SDL_GetWindowPosition(window, &x, &y);

		return Window {
			.sdl_window = window,
			.size = { width, height },
			.resolution = { width, height },
			.windowed_pos = { x, y },
			.is_fullscreen = false,
		};
	}

	void destroy_window(Window window) {
		SDL_DestroyWindow(window.sdl_window);
	}

	void toggle_fullscreen(Window* window) {
		int display_index = SDL_GetWindowDisplayIndex(window->sdl_window);

		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(display_index, &display_mode);

		SDL_Rect display_bound;
		SDL_GetDisplayBounds(display_index, &display_bound);

		if (window->is_fullscreen) {
			window->is_fullscreen = false;

			/* Toggle windowed */
			SDL_SetWindowBordered(window->sdl_window, SDL_TRUE);
			SDL_SetWindowPosition(window->sdl_window, window->windowed_pos.x, window->windowed_pos.y);
			SDL_SetWindowSize(window->sdl_window, window->resolution.x, window->resolution.y);

			/* Update window size */
			window->size = window->resolution;
		}
		else {
			window->is_fullscreen = true;

			/* Save current windowed position */
			SDL_GetWindowPosition(window->sdl_window, &window->windowed_pos.x, &window->windowed_pos.y);

			/* Toggle fullscreen */
			SDL_SetWindowBordered(window->sdl_window, SDL_FALSE);
			SDL_SetWindowPosition(window->sdl_window, display_bound.x, display_bound.y);
			SDL_SetWindowSize(window->sdl_window, display_mode.w, display_mode.h);

			/* Update windowed size */
			window->size = glm::ivec2 { display_mode.w, display_mode.h };
		}
	}

	void change_resolution(Window* window, int width, int height) {
		// update resolution
		window->resolution.x = width;
		window->resolution.y = height;

		if (!window->is_fullscreen) {
			window->size.x = width;
			window->size.y = height;
			SDL_SetWindowSize(window->sdl_window, window->size.x, window->size.y);
		}
	}

} // namespace platform

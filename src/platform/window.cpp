#include <platform/window.h>

#include <platform/assert.h>
#include <platform/renderer.h>

namespace platform {

	std::optional<Window> Window::create(int width, int height, int window_flags) {
		SDL_Window* sdl_window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			window_flags | SDL_WINDOW_OPENGL
		);

		if (!sdl_window) {
			LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
			return {};
		}

		int x, y;
		SDL_GetWindowPosition(sdl_window, &x, &y);

		Window window;
		window.m_sdl_window = sdl_window;
		window.m_size = { width, height };
		window.m_resolution = { width, height };
		window.m_windowed_pos = { x, y };
		window.m_is_fullscreen = false;
		return window;
	}

	void Window::destroy() {
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = nullptr;
	}

	SDL_Window* Window::sdl_window() const {
		return m_sdl_window;
	}

	bool Window::is_maximized() const {
		return m_is_maximized;
	}

	bool Window::is_fullscreen() const {
		return m_is_fullscreen;
	}

	glm::ivec2 Window::size() const {
		return m_size;
	}

	glm::ivec2 Window::resolution() const {
		return m_resolution;
	}

	void Window::on_resize(int width, int height) {
		m_is_maximized = SDL_GetWindowFlags(m_sdl_window) & SDL_WINDOW_MAXIMIZED;
		m_size = glm::ivec2 { width, height };
	}

	void Window::toggle_fullscreen() {
		int display_index = SDL_GetWindowDisplayIndex(m_sdl_window);

		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(display_index, &display_mode);

		SDL_Rect display_bound;
		SDL_GetDisplayBounds(display_index, &display_bound);

		if (m_is_fullscreen) {
			m_is_fullscreen = false;

			/* Toggle windowed */
			SDL_SetWindowBordered(m_sdl_window, SDL_TRUE);
			SDL_SetWindowPosition(m_sdl_window, m_windowed_pos.x, m_windowed_pos.y);
			SDL_SetWindowSize(m_sdl_window, m_windowed_size.x, m_windowed_size.y);

			/* Update window size */
			m_size = m_windowed_size;
		}
		else {
			m_is_fullscreen = true;

			/* Save current windowed position and size */
			SDL_GetWindowPosition(m_sdl_window, &m_windowed_pos.x, &m_windowed_pos.y);
			SDL_GetWindowSize(m_sdl_window, &m_windowed_size.x, &m_windowed_size.y);

			/* Toggle fullscreen */
			SDL_SetWindowBordered(m_sdl_window, SDL_FALSE);
			SDL_SetWindowPosition(m_sdl_window, display_bound.x, display_bound.y);
			SDL_SetWindowSize(m_sdl_window, display_mode.w, display_mode.h);

			/* Update window size */
			m_size = glm::ivec2 { display_mode.w, display_mode.h };
		}
	}

	void Window::change_resolution(int width, int height) {
		// update resolution
		m_resolution.x = width;
		m_resolution.y = height;

		if (!m_is_fullscreen) {
			m_size.x = width;
			m_size.y = height;
			SDL_SetWindowSize(m_sdl_window, m_size.x, m_size.y);
		}
	}

} // namespace platform

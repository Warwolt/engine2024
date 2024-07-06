#include <platform/window.h>

#include <platform/assert.h>
#include <platform/logging.h>
#include <platform/renderer.h>

namespace platform {

	std::optional<Window> Window::create(int width, int height, int window_flags, const std::string& window_title) {
		SDL_Window* sdl_window = SDL_CreateWindow(
			window_title.c_str(),
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

	glm::ivec2 Window::position() const {
		glm::ivec2 position;
		SDL_GetWindowPosition(m_sdl_window, &position.x, &position.y);
		return position;
	}

	void Window::set_size(glm::ivec2 size) {
		if (!m_is_fullscreen) {
			SDL_SetWindowSize(m_sdl_window, size.x, size.y);
			on_resize(size.x, size.y);
		}
		else {
			LOG_WARNING("Trying to set size on a full screen window");
		}
	}

	void Window::set_position(glm::ivec2 position) {
		if (!m_is_fullscreen) {
			SDL_SetWindowPosition(m_sdl_window, position.x, position.y);
		}
		else {
			LOG_WARNING("Trying to set position of a full screen window");
		}
	}

	void Window::on_resize(int width, int height) {
		m_is_maximized = SDL_GetWindowFlags(m_sdl_window) & SDL_WINDOW_MAXIMIZED;
		m_size = glm::ivec2 { width, height };
	}

	void Window::set_window_mode(WindowMode mode) {
		if (!m_is_fullscreen && mode == WindowMode::FullScreen) {
			_set_fullscreen_mode();
		}
		if (m_is_fullscreen && mode == WindowMode::Windowed) {
			_set_windowed_mode();
		}
	}

	void Window::toggle_fullscreen() {
		if (m_is_fullscreen) {
			_set_windowed_mode();
		}
		else {
			_set_fullscreen_mode();
		}
	}

	void Window::_set_fullscreen_mode() {
		int display_index = SDL_GetWindowDisplayIndex(m_sdl_window);

		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(display_index, &display_mode);

		SDL_Rect display_bound;
		SDL_GetDisplayBounds(display_index, &display_bound);

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

	void Window::_set_windowed_mode() {
		m_is_fullscreen = false;

		/* Toggle windowed */
		SDL_SetWindowBordered(m_sdl_window, SDL_TRUE);
		SDL_SetWindowPosition(m_sdl_window, m_windowed_pos.x, m_windowed_pos.y);
		SDL_SetWindowSize(m_sdl_window, m_windowed_size.x, m_windowed_size.y);

		/* Update window size */
		m_size = m_windowed_size;
	}

} // namespace platform

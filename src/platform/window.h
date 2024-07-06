#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>
#include <string>

namespace platform {

	enum class WindowMode {
		Windowed,
		FullScreen,
	};

	class Window {
	public:
		static std::optional<Window> create(int widht, int height, int window_flags, const std::string& window_title);
		void destroy();

		glm::ivec2 size() const;
		glm::ivec2 position() const;
		SDL_Window* sdl_window() const;
		bool is_maximized() const;
		bool is_fullscreen() const;

		void set_size(glm::ivec2 size);
		void set_position(glm::ivec2 position);
		void on_resize(int width, int height);
		void set_window_mode(WindowMode mode);
		void toggle_fullscreen();

	private:
		void _set_fullscreen_mode();
		void _set_windowed_mode();

		mutable SDL_Window* m_sdl_window;
		glm::ivec2 m_size;
		glm::ivec2 m_windowed_pos;
		glm::ivec2 m_windowed_size;
		bool m_is_maximized = false;
		bool m_is_fullscreen = false;
	};

} // namespace platform

#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>

namespace platform {

	class Window {
	public:
		static std::optional<Window> create(int widht, int height, int window_flags);
		void destroy();

		glm::ivec2 size() const;
		SDL_Window* sdl_window() const;
		bool is_maximized() const;
		bool is_fullscreen() const;

		void on_resize(int width, int height);
		void toggle_fullscreen();

	private:
		SDL_Window* m_sdl_window;
		glm::ivec2 m_size;
		glm::ivec2 m_windowed_pos;
		glm::ivec2 m_windowed_size;
		bool m_is_maximized = false;
		bool m_is_fullscreen = false;
	};

} // namespace platform

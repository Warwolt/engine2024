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
		glm::ivec2 resolution() const;
		SDL_Window* sdl_window() const;

		void on_resize(int width, int height);
		void toggle_fullscreen();
		void change_resolution(int width, int height);

	private:
		SDL_Window* m_sdl_window;
		glm::ivec2 m_size;
		glm::ivec2 m_resolution;
		glm::ivec2 m_windowed_pos;
		glm::ivec2 m_windowed_size;
		bool m_is_fullscreen = false;
	};

} // namespace platform

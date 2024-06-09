#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>

namespace platform {

	class Window {
	public:
		static std::optional<Window> create(int widht, int height, int window_flags);
		void destroy();

		void toggle_fullscreen();
		void change_resolution(int width, int height);

		glm::ivec2 size() const;
		glm::ivec2 resolution() const;

		SDL_Window* sdl_window() const;

	private:
		SDL_Window* m_sdl_window;
		glm::ivec2 m_size;
		glm::ivec2 m_resolution;
		glm::ivec2 m_windowed_pos;
		bool m_is_fullscreen = false;
	};

} // namespace platform

#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>

namespace platform {

	struct Window {
		SDL_Window* sdl_window;
		glm::ivec2 size;
		glm::ivec2 resolution;
		glm::ivec2 windowed_pos;
		bool is_fullscreen = false;
	};

	std::optional<Window> create_window(int widht, int height, int window_flags);
	void destroy_window(Window window);

	void toggle_fullscreen(Window* window);
	void change_resolution(Window* window, int width, int height);

} // namespace platform

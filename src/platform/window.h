#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>

namespace platform {

	struct WindowInfo {
		SDL_Window* window;
		glm::ivec2 size;
		glm::ivec2 resolution;
		glm::ivec2 windowed_pos;
		bool is_fullscreen = false;
	};

	std::optional<WindowInfo> create_window(int widht, int height);
	void destroy_window(WindowInfo window_info);

	void toggle_fullscreen(WindowInfo* window_info);

} // namespace platform

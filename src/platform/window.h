#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include <optional>

namespace platform {

	// TODO return this from platform::create_window
	// Add SDL_Window* to WindowInfo as well
	// Move to "platform/window.h"?
	// Could do the toggle_fullscreen and change_resolution there too
	struct WindowInfo {
		SDL_Window* window;
		glm::ivec2 size;
		glm::ivec2 resolution;
		glm::ivec2 windowed_pos;
		bool is_fullscreen = false;
	};

	std::optional<WindowInfo> create_window(int widht, int height);
	void destroy_window(WindowInfo window_info);

} // namespace platform

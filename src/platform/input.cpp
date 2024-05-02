#include <platform/input.h>

#include <SDL2/SDL.h>

namespace platform {

	Input read_input() {
		Input input = { 0 };
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					input.quit_signal_received = true;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						input.escape_key_pressed = true;
					}
					break;
			}
		}
		return input;
	}

} // namespace platform

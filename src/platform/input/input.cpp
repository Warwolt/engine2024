#include <platform/input/input.h>
#include <platform/logging.h>

#include <SDL2/SDL.h>

namespace platform {

	void read_input(Input* input) {
		input->window_resized = {};

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					input->quit_signal_received = true;
					break;
				case SDL_KEYDOWN:
					input->keyboard.register_event(event.key.keysym.sym, ButtonEvent::Down);
					break;
				case SDL_KEYUP:
					input->keyboard.register_event(event.key.keysym.sym, ButtonEvent::Up);
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						input->window_resized = glm::vec2 { (float)event.window.data1, (float)event.window.data2 };
					}
					break;
			}
		}
		input->keyboard.update();
	}

} // namespace platform

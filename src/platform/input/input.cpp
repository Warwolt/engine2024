#include <platform/input/input.h>
#include <platform/logging.h>

#include <SDL2/SDL.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include <array>

namespace platform {

	SDL_Rect stretched_and_centered_canvas(glm::ivec2 window_size, glm::ivec2 canvas_size) {
		int scale = (int)std::max(std::round(window_size.x / canvas_size.x), std::round(window_size.y / canvas_size.y));
		glm::ivec2 scaled_canvas_size = { scale * canvas_size.x, scale * canvas_size.y };
		glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;

		return SDL_Rect { .x = top_left.x, .y = top_left.y, .w = scaled_canvas_size.x, .h = scaled_canvas_size.y };
	}

	void read_input(
		Input* input,
		Timer* frame_timer,
		glm::ivec2 window_size,
		glm::ivec2 canvas_size
	) {
		std::array<ButtonEvent, 5> mouse_button_events = { ButtonEvent::None };
		input->mouse.scroll_delta = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
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
				case SDL_MOUSEMOTION: {
					SDL_Rect canvas = stretched_and_centered_canvas(window_size, canvas_size);
					int scale = canvas.w / canvas_size.x;
					input->mouse.pos = glm::ivec2 {
						(event.motion.x - canvas.x) / scale,
						(event.motion.y - canvas.y) / scale
					};
					break;
				}
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button - 1 < 5) {
						mouse_button_events[event.button.button - 1] = ButtonEvent::Down;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button - 1 < 5) {
						mouse_button_events[event.button.button - 1] = ButtonEvent::Up;
					}
					break;
				case SDL_MOUSEWHEEL:
					input->mouse.scroll_delta += event.wheel.y;
					break;
			}
		}

		input->window_resolution = canvas_size;

		input->mouse.left_button = update_button(input->mouse.left_button, mouse_button_events[SDL_BUTTON_LEFT - 1]);
		input->mouse.middle_button = update_button(input->mouse.middle_button, mouse_button_events[SDL_BUTTON_MIDDLE - 1]);
		input->mouse.right_button = update_button(input->mouse.right_button, mouse_button_events[SDL_BUTTON_RIGHT - 1]);
		input->mouse.x1_button = update_button(input->mouse.x1_button, mouse_button_events[SDL_BUTTON_X1 - 1]);
		input->mouse.x2_button = update_button(input->mouse.x2_button, mouse_button_events[SDL_BUTTON_X2 - 1]);

		input->keyboard.update();

		input->delta_ms = frame_timer->elapsed_ms();
		frame_timer->reset();
	}

} // namespace platform

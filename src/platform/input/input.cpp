#include <platform/input/input.h>
#include <platform/logging.h>

#include <SDL2/SDL.h>
#include <imgui/backends/imgui_impl_sdl2.h>

#include "input.h"
#include <array>

namespace platform {

	SDL_Rect stretched_and_centered_canvas(glm::ivec2 window_size, glm::ivec2 window_resolution) {
		int scale = (int)std::max(std::round(window_size.x / window_resolution.x), std::round(window_size.y / window_resolution.y));
		glm::ivec2 scaled_canvas_size = { scale * window_resolution.x, scale * window_resolution.y };
		glm::ivec2 top_left = (window_size - scaled_canvas_size) / 2;

		return SDL_Rect { .x = top_left.x, .y = top_left.y, .w = scaled_canvas_size.x, .h = scaled_canvas_size.y };
	}

	std::vector<SDL_Event> read_events() {
		std::vector<SDL_Event> events;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			events.push_back(event);
		}
		return events;
	}

	void process_events(
		const std::vector<SDL_Event>* events,
		Input* input,
		Timer* frame_timer,
		glm::ivec2 window_size,
		glm::ivec2 window_resolution
	) {
		constexpr size_t NUM_MOUSE_BUTTONS = 5;
		std::array<ButtonEvent, NUM_MOUSE_BUTTONS> mouse_button_events = { ButtonEvent::None };
		input->mouse.scroll_delta = 0;
		input->mouse.pos_delta = glm::vec2 { 0, 0 };

		ImGuiIO& imgui_io = ImGui::GetIO();
		for (SDL_Event event : *events) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type) {
				case SDL_QUIT:
					input->quit_signal_received = true;
					break;

				case SDL_KEYDOWN:
					if (!imgui_io.WantCaptureKeyboard) {
						input->keyboard.register_event(event.key.keysym.sym, ButtonEvent::Down);
					}
					break;

				case SDL_KEYUP:
					if (!imgui_io.WantCaptureKeyboard) {
						input->keyboard.register_event(event.key.keysym.sym, ButtonEvent::Up);
					}
					break;

				case SDL_MOUSEMOTION: {
					if (!imgui_io.WantCaptureMouse) {
						SDL_Rect canvas = stretched_and_centered_canvas(window_size, window_resolution);
						int scale = canvas.w / window_resolution.x;
						glm::vec2 new_mouse_pos = glm::vec2 {
							(event.motion.x - canvas.x) / scale,
							(event.motion.y - canvas.y) / scale
						};
						input->mouse.pos_delta = new_mouse_pos - input->mouse.pos;
						input->mouse.pos = new_mouse_pos;
					}
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
					if (!imgui_io.WantCaptureMouse) {
						if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
							mouse_button_events[event.button.button - 1] = ButtonEvent::Down;
						}
						break;
					}

				case SDL_MOUSEBUTTONUP:
					if (!imgui_io.WantCaptureMouse) {
						if (event.button.button - 1 < NUM_MOUSE_BUTTONS) {
							mouse_button_events[event.button.button - 1] = ButtonEvent::Up;
						}
					}
					break;

				case SDL_MOUSEWHEEL:
					if (!imgui_io.WantCaptureMouse) {
						input->mouse.scroll_delta += event.wheel.y;
					}
					break;
			}
		}

		input->window_resolution = window_resolution;

		input->mouse.left_button.update(mouse_button_events[SDL_BUTTON_LEFT - 1]);
		input->mouse.middle_button.update(mouse_button_events[SDL_BUTTON_MIDDLE - 1]);
		input->mouse.right_button.update(mouse_button_events[SDL_BUTTON_RIGHT - 1]);
		input->mouse.x1_button.update(mouse_button_events[SDL_BUTTON_X1 - 1]);
		input->mouse.x2_button.update(mouse_button_events[SDL_BUTTON_X2 - 1]);

		input->keyboard.update();

		input->delta_ms = frame_timer->elapsed_ms();
		input->global_time_ms += input->delta_ms;
		frame_timer->reset();
	}

} // namespace platform

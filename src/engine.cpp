#include <engine.h>

#include <platform/logging.h>
#include <plog/Init.h>

namespace engine {

	void on_load(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	platform::Commands update(State* state, const platform::Input* input) {
		platform::Commands commands = { 0 };

		state->millis += input->delta_ms;
		if (state->millis >= 1000) {
			state->millis -= 1000;
			state->tick += 1;
			LOG_INFO("%zu", state->tick);
		}

		if (input->quit_signal_received || input->keyboard.key_pressed_now(SDL_SCANCODE_ESCAPE)) {
			commands.quit();
		}

		return commands;
	}

	void render(platform::Renderer* renderer, const State* /* state */) {
		glm::vec4 color = { 70.f / 255, 55.f / 255, 56.f / 255, 1.0f };
		renderer->draw_rect_fill({ -0.5f, 0.5f }, { 0.5f, -0.5f }, color);
	}

} // namespace engine

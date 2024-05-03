#include <engine.h>

#include <platform/logging.h>

#include <nlohmann/json.hpp>
#include <plog/Init.h>

namespace engine {

	struct State {
		uint64_t tick = 0;
		uint64_t millis = 0;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State, tick, millis)

	static State g_state;

	void save_state(nlohmann::json* target) {
		to_json(*target, g_state);
	}

	void load_state(nlohmann::json* source) {
		from_json(*source, g_state);
	}

	void init_logging(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	platform::Commands update(const platform::Input* input) {
		platform::Commands commands = { 0 };

		g_state.millis += input->delta_ms;
		if (g_state.millis >= 1000) {
			g_state.millis -= 1000;
			g_state.tick += 1;
			LOG_INFO("%zu", g_state.tick);
		}

		if (input->quit_signal_received || input->keyboard.key_pressed_now(SDL_SCANCODE_ESCAPE)) {
			commands.quit();
		}

		return commands;
	}

	void render(platform::Renderer* renderer) {
		renderer->draw_rect_fill({ -0.5f, 0.5f }, { 0.5f, -0.5f }, { 1.0f, 0.5f, 0.0f });
	}

} // namespace engine

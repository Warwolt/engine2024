#include <engine.h>

#include <platform/logging.h>
#include <plog/Init.h>

namespace engine {

	void on_load(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	platform::Commands update(EngineState* engine, const platform::Input* input) {
		platform::Commands commands = { 0 };

		engine->millis += input->delta_ms;
		if (engine->millis >= 1000) {
			engine->millis -= 1000;
			engine->tick += 1;
			LOG_INFO("%zu", engine->tick);
		}

		if (input->quit_signal_received || input->escape_key_pressed) {
			commands.quit();
		}

		return commands;
	}

	void render(platform::Renderer* renderer, const EngineState* /* engine */) {
		renderer->draw_rect_fill({ -0.5f, 0.5f }, { 0.5f, -0.5f }, { 1.0f, 0.5f, 0.0f });
	}

} // namespace engine

#include <engine.h>

#include <platform/logging.h>
#include <plog/Init.h>

namespace engine {

	void on_load(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	void update(EngineState* engine, uint64_t delta_ms) {
		engine->millis += delta_ms;
		if (engine->millis >= 1000) {
			engine->millis -= 1000;
			engine->tick += 1;
			LOG_INFO("%zu", engine->tick);
		}
	}

	void render(platform::Renderer* renderer, const EngineState* /* engine */) {
		renderer->draw_rect_fill({ -0.5f, 0.5f }, { 0.5f, -0.5f }, { 1.0f, 0.5f, 0.0f });
	}

} // namespace engine

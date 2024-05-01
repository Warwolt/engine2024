#include <engine.h>

#include <platform/logging.h>
#include <plog/Init.h>

namespace engine {

	void engine_on_load(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	void engine_update(EngineState* engine, uint64_t delta_ms) {
		engine->millis += delta_ms;
		if (engine->millis >= 1000) {
			engine->millis -= 1000;
			engine->tick += 1;
			LOG_INFO("%zu", engine->tick);
		}
	}

} // namespace engine

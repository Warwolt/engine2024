#include <engine.h>

#include <stdio.h>

namespace engine {

	void engine_update(EngineState* engine, uint64_t delta_ms) {
		engine->millis += delta_ms;
		if (engine->millis >= 1000) {
			engine->millis -= 1000;
			engine->tick += 1;
			printf("%zu\n", engine->tick);
		}
	}

} // namespace engine

#include <engine.h>

#include <stdio.h>

void engine_update(EngineState* engine) {
	uint64_t delta_ms = engine->timer.elapsed_ms();
	engine->timer.reset();

	engine->millis += delta_ms;
	if (engine->millis >= 1000) {
		engine->millis -= 1000;
		engine->tick += 1;
		printf("%zu\n", engine->tick);
	}
}

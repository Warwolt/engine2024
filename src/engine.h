#pragma once

#include <stdint.h>

namespace engine {

	struct EngineState {
		uint64_t tick = 0;
		uint64_t millis = 0;
	};

	extern "C" __declspec(dllexport) void engine_update(EngineState* engine, uint64_t delta_ms);

} // namespace engine

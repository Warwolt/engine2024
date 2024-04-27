#pragma once

#include <timing.h>

#include <stdint.h>

namespace engine {

	struct EngineState {
		uint64_t tick = 0;
		uint64_t millis = 0;
		timing::Timer timer;
	};

	extern "C" __declspec(dllexport) void engine_update(EngineState* engine);

} // namespace engine

#pragma once

#include <platform/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <stdint.h>

namespace engine {

	struct EngineState {
		uint64_t tick = 0;
		uint64_t millis = 0;
	};

	extern "C" __declspec(dllexport) void on_load(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void update(EngineState* engine, uint64_t delta_ms);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const EngineState* engine);

} // namespace engine

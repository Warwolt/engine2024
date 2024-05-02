#pragma once

#include <platform/commands.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <stdint.h>

namespace engine {

	struct State {
		uint64_t tick = 0;
		uint64_t millis = 0;
	};

	extern "C" __declspec(dllexport) void on_load(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) platform::Commands update(State* state, const platform::Input* input);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

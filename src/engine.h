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
		platform::Texture texture;
	};

	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void initialize(State* state);
	extern "C" __declspec(dllexport) void deinitialize(State* state);
	extern "C" __declspec(dllexport) void update(State* state, const platform::Input* input, platform::CommandAPI* commands);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

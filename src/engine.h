#pragma once

#include <platform/commands.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <nlohmann/json.hpp>

#include <stdint.h>

namespace engine {

	struct State {
		uint64_t tick = 0;
		uint64_t millis = 0;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(State, tick, millis)

	extern "C" __declspec(dllexport) void init_logging(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) platform::Commands update(State* state, const platform::Input* input);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

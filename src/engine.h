#pragma once

#include <platform/commands.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <nlohmann/json_fwd.hpp>

#include <stdint.h>

namespace engine {

	extern "C" __declspec(dllexport) void save_state(nlohmann::json* target);
	extern "C" __declspec(dllexport) void load_state(nlohmann::json* source);
	extern "C" __declspec(dllexport) void init_logging(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) platform::Commands update(const platform::Input* input);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer);

} // namespace engine

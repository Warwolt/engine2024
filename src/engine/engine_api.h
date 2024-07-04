#pragma once

#include <engine/state/engine_state.h>
#include <platform/logging.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>

struct ImGuiContext;
typedef struct FT_LibraryRec_* FT_Library;

namespace engine {

	// global state wiring
	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void set_imgui_context(ImGuiContext* imgui_context);
	extern "C" __declspec(dllexport) void set_freetype_library(FT_Library ft);

	// engine interface
	extern "C" __declspec(dllexport) void initialize(State* state);
	extern "C" __declspec(dllexport) void shutdown(State* state);
	extern "C" __declspec(dllexport) void update(State* state, const platform::Input& input, platform::PlatformAPI* platform);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

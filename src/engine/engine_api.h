#pragma once

#include <engine/engine.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/graphics/renderer.h>
#include <platform/platform_api.h>

struct ImGuiContext;
namespace ImWin32 {
	struct ImWin32Context;
} // namespace ImWin32
typedef struct FT_LibraryRec_* FT_Library;

namespace engine {

	// global state wiring
	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void set_imgui_context(ImGuiContext* imgui_context);
	extern "C" __declspec(dllexport) void set_imwin32_context(ImWin32::ImWin32Context* imwin32_context);
	extern "C" __declspec(dllexport) void set_freetype_library(FT_Library ft);

	// engine interface
	extern "C" __declspec(dllexport) Engine* initialize(const platform::Configuration* config);
	extern "C" __declspec(dllexport) void shutdown(Engine* state);
	extern "C" __declspec(dllexport) void load_project(Engine* state, const char* path);
	extern "C" __declspec(dllexport) void update(Engine* state, const platform::Input& input, platform::PlatformAPI* platform);
	extern "C" __declspec(dllexport) void render(const Engine& engine, platform::Renderer* renderer);

} // namespace engine

#pragma once

#include <editor/editor.h>
#include <engine/engine.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/graphics/graphics_context.h>
#include <platform/graphics/renderer.h>
#include <platform/platform_api.h>

struct ImGuiContext;
namespace ImWin32 {
	struct ImWin32Context;
} // namespace ImWin32
typedef struct FT_LibraryRec_* FT_Library;

namespace library {

	// global state wiring
	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void set_imgui_context(ImGuiContext* imgui_context);
	extern "C" __declspec(dllexport) void set_imwin32_context(ImWin32::ImWin32Context* imwin32_context);
	extern "C" __declspec(dllexport) void set_freetype_library(FT_Library ft);

	// engine interface
	extern "C" __declspec(dllexport) engine::Engine* initialize_engine(platform::GraphicsContext* graphics);
	extern "C" __declspec(dllexport) void shutdown_engine(engine::Engine* engine);
	extern "C" __declspec(dllexport) void update_engine(engine::Engine* engine, const platform::Input& input, platform::PlatformAPI* platform, platform::GraphicsContext* graphics);
	extern "C" __declspec(dllexport) void render_engine(const engine::Engine& engine, platform::Renderer* renderer);
	extern "C" __declspec(dllexport) void load_engine_data(engine::Engine* engine, const char* path);

	// editor interface
	extern "C" __declspec(dllexport) editor::Editor* initialize_editor(engine::Engine* engine, platform::GraphicsContext* graphics, const platform::Configuration& config);
	extern "C" __declspec(dllexport) void shutdown_editor(editor::Editor* editor);
	extern "C" __declspec(dllexport) void update_editor(editor::Editor* editor, const platform::Configuration& config, const platform::Input& input, engine::Engine* engine, platform::PlatformAPI* platform, platform::GraphicsContext* graphics);
	extern "C" __declspec(dllexport) void render_editor(const editor::Editor& editor, const engine::Engine& engine, platform::Renderer* renderer);

} // namespace library

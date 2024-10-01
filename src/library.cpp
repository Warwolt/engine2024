#include <library.h>

#include <core/container.h>
#include <engine/system/hot_reloading.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/file/file.h>
#include <platform/os/imwin32.h>

#include <imgui/imgui.h>
#include <plog/Init.h>

#include <numeric>

namespace library {

	void set_logger(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	void set_imgui_context(ImGuiContext* imgui_context) {
		ImGui::SetCurrentContext(imgui_context);
	}

	void set_imwin32_context(ImWin32::ImWin32Context* imwin32_context) {
		ImWin32::SetCurrentContext(imwin32_context);
	}

	void set_freetype_library(FT_Library ft) {
		platform::set_ft(ft);
	}

	engine::Engine* initialize_engine(platform::GraphicsContext* graphics) {
		return new engine::Engine(graphics);
	}

	void shutdown_engine(engine::Engine* engine) {
		delete engine;
	}

	void update_engine(engine::Engine* engine, const platform::Input& input, platform::PlatformAPI* platform, platform::GraphicsContext* graphics) {
		engine->update(input, platform, graphics);
	}

	void render_engine(const engine::Engine& engine, platform::Renderer* renderer) {
		engine.render(renderer);
	}

	void load_engine_data(engine::Engine* engine, const char* path) {
		engine->load_data(path);
	}

	editor::Editor* initialize_editor(engine::Engine* engine, platform::GraphicsContext* graphics, const platform::Configuration& config) {
		return new editor::Editor(engine, graphics, config);
	}

	void shutdown_editor(editor::Editor* editor) {
		delete editor;
	}

	void update_editor(editor::Editor* editor, const platform::Configuration& config, const platform::Input& input, engine::Engine* engine, platform::PlatformAPI* platform, platform::GraphicsContext* graphics) {
		editor->update(input, config, engine, platform, graphics);
	}

	void render_editor(const editor::Editor& editor, const engine::Engine& engine, platform::Renderer* renderer) {
		editor.render(engine, renderer);
	}

} // namespace library

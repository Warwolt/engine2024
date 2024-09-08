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

	engine::Engine* initialize_engine(const platform::Configuration& config) {
		return new engine::Engine(config);
	}

	void shutdown_engine(engine::Engine* engine) {
		delete engine;
	}

	void update_engine(engine::Engine* engine, const platform::Input& input, platform::PlatformAPI* platform) {
		engine->update(input, platform);
	}

	void render_engine(const engine::Engine& engine, platform::Renderer* renderer) {
		engine.render(renderer);
	}

	void load_engine_data(engine::Engine* engine, const char* path) {
		engine->load_data(path);
	}

	editor::Editor* initialize_editor(engine::Engine* engine, const platform::Configuration& config) {
		editor::Editor* editor = new editor::Editor();
		editor::init_editor(editor, engine, config);
		return editor;
	}

	void shutdown_editor(editor::Editor* editor) {
		delete editor;
	}

	void update_editor(editor::Editor* editor, const platform::Configuration& config, const platform::Input& input, engine::Engine* engine, platform::PlatformAPI* platform) {
		editor::update_editor(editor, input, config, engine, platform);
	}

	void render_editor(const editor::Editor& editor, const engine::Engine& engine, platform::Renderer* renderer) {
		editor::render_editor(editor, engine, renderer);
	}

} // namespace library

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

	engine::Engine* initialize_engine(const platform::Configuration* config) {
		return new engine::Engine(config);
	}

	void shutdown_engine(engine::Engine* engine) {
		delete engine;
	}

	void load_project(engine::Engine* engine, const char* path) {
		engine->load_project(path);
	}

	void update_engine(engine::Engine* engine, const platform::Input& input, platform::PlatformAPI* platform) {
		engine->update(input, platform);
	}

	void render_engine(const engine::Engine& engine, platform::Renderer* renderer) {
		engine.render(renderer);
	}

} // namespace library

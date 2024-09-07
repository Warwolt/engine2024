#include <engine/engine_api.h>

#include <core/container.h>
#include <engine/system/hot_reloading.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/file/file.h>
#include <platform/os/imwin32.h>

#include <imgui/imgui.h>
#include <plog/Init.h>

#include <numeric>

namespace engine {

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

	Engine* initialize(const platform::Configuration* config) {
		Engine* state = new Engine();

		/* Initialize */
		const bool reset_docking = !config->window.docking_initialized;
		init_editor(&state->m_editor, &state->m_systems, state->m_project, reset_docking);

		// add fake elements
		const char* arial_font_path = "C:/windows/Fonts/Arial.ttf";
		FontID arial_font_16 = core::container::unwrap(state->m_systems.text.add_ttf_font(arial_font_path, 16), [&] {
			ABORT("Failed to load font \"%s\"", arial_font_path);
		});
		TextID hello = state->m_systems.text.add_text_node(arial_font_16, "Hello", { 0.0f, 0.0f });
		TextID world = state->m_systems.text.add_text_node(arial_font_16, "World", { 0.0f, 18.0f });
		state->m_scene_graph.add_text_node(state->m_scene_graph.root(), hello);
		state->m_scene_graph.add_text_node(state->m_scene_graph.root(), world);

		return state;
	}

	void shutdown(Engine* state) {
		delete state;
	}

	void load_project(Engine* engine, const char* path) {
		engine->load_project(path);
	}

	void update(Engine* engine, const platform::Input& input, platform::PlatformAPI* platform) {
		engine->update(input, platform);
	}

	void render(const Engine& engine, platform::Renderer* renderer) {
		engine.render(renderer);
	}

} // namespace engine

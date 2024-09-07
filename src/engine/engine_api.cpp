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
	static void draw_imgui(DebugUiState* debug_ui, platform::PlatformAPI* platform, const platform::Input& input) {
		struct Resolution {
			glm::ivec2 value;
			const char* str;
		};
		const static Resolution resolutions[] = {
			{ { 800, 600 }, "800x600" },
			{ { 640, 480 }, "640x480" },
			{ { 960, 600 }, "960x600" },
			{ { 480, 300 }, "480x300" },

		};

		const char* combo_preview_value = resolutions[debug_ui->resolution_index].str;
		if (ImGui::BeginCombo("Resolution", combo_preview_value, 0)) {
			for (int n = 0; n < IM_ARRAYSIZE(resolutions); n++) {
				const bool current_is_selected = (debug_ui->resolution_index == n);
				if (ImGui::Selectable(resolutions[n].str, current_is_selected)) {
					debug_ui->resolution_index = n;
				}

				if (current_is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Change resolution")) {
			glm::ivec2 resolution = resolutions[debug_ui->resolution_index].value;
			platform->change_resolution(resolution.x, resolution.y);
		}

		ImGui::SeparatorText("Render Debug");
		{
			debug_ui->frame_render_deltas.push_back((float)input.renderer_debug_data.render_ns / 1000000.0f);
			debug_ui->second_counter_ms += input.delta_ms;
			if (debug_ui->second_counter_ms >= 1000) {
				debug_ui->second_counter_ms = 0;
				debug_ui->render_delta_avg_ms = std::accumulate(debug_ui->frame_render_deltas.begin(), debug_ui->frame_render_deltas.end(), 0.0f) / debug_ui->frame_render_deltas.size();
			}

			ImGui::Text("Draw calls: %zu", input.renderer_debug_data.num_draw_calls);
			ImGui::Text("Num vertices: %zu", input.renderer_debug_data.num_vertices);
			ImGui::Text("Render ms: %2.2f", debug_ui->render_delta_avg_ms);
		}
	}

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

	void update(Engine* state, const platform::Input& input, platform::PlatformAPI* platform) {
		state->m_window_resolution = input.window_resolution;
		state->m_editor_is_running = input.mode == platform::RunMode::Editor;
		const bool game_is_running = input.mode == platform::RunMode::Game;

		/* Quit */
		{
			// editor handles quit in editor mode
			if (game_is_running) {
				if (input.quit_signal_received) {
					platform->quit();
				}

				if (input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
					if (input.is_editor_mode) {
						platform->set_run_mode(platform::RunMode::Editor);
						platform->set_window_mode(platform::WindowMode::Windowed);
					}
					else {
						platform->quit();
					}
				}
			}
		}

		/* Update game */
		if (game_is_running) {
			state->m_game.time_ms += input.delta_ms;
			if (state->m_game.time_ms >= 1000) {
				state->m_game.time_ms -= 1000;
				state->m_game.counter += 1;
			}
		}

		/* Window */
		{
			if (input.keyboard.key_pressed_now(SDLK_F11)) {
				platform->toggle_fullscreen();
			}
		}

		/* Debug UI */
		{
			if (input.keyboard.key_pressed_now(SDLK_F3)) {
				state->m_debug_ui.show_debug_ui = !state->m_debug_ui.show_debug_ui;
			}

			if (state->m_debug_ui.show_debug_ui) {
				draw_imgui(&state->m_debug_ui, platform, input);
			}
		}

		/* Modules */
		{
			std::string window_title = state->m_project.name;
			if (input.is_editor_mode) {
				window_title += std::string(state->m_editor.project_has_unsaved_changes ? "*" : "") + " - Engine2024";
			}
			update_hot_reloading(&state->m_hot_reloading, &state->m_systems.animation, input, platform, &window_title);
			platform->set_window_title(window_title.c_str());
			if (input.mode == platform::RunMode::Editor) {
				editor::update_editor(
					&state->m_editor,
					&state->m_game,
					&state->m_project,
					&state->m_systems,
					&state->m_scene_graph,
					input,
					platform
				);
			}
		}
	}

	static void render_game(const Engine& state, platform::Renderer* renderer) {
		// clear
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, state.m_window_resolution }, platform::Color::black);

		// render text
		glm::vec2 window_center = state.m_window_resolution / 2.0f;
		for (const auto& [node_id, text_node] : state.m_systems.text.text_nodes()) {
			const platform::Font& font = state.m_systems.text.fonts().at(text_node.font_id);
			renderer->draw_text(font, text_node.text, window_center + text_node.position, platform::Color::white);
		}
	}

	void render(platform::Renderer* renderer, const Engine* state) {
		if (state->m_editor_is_running) {
			editor::render_editor(state->m_editor, state->m_systems, renderer);
		}
		else {
			render_game(*state, renderer);
		}
	}

} // namespace engine

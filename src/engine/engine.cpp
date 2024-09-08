#include <engine/engine.h>

#include <core/container.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/file/file.h>

#include <imgui/imgui.h>

#include <filesystem>
#include <vector>

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

	Engine::Engine(const platform::Configuration& config) {
		/* Initialize */
		//const bool reset_docking = !config->window.docking_initialized;
		//init_editor(&m_editor, &m_systems, m_project, reset_docking);

		// add fake elements
		const char* arial_font_path = "C:/windows/Fonts/Arial.ttf";
		FontID arial_font_16 = core::container::unwrap(m_systems.text.add_ttf_font(arial_font_path, 16), [&] {
			ABORT("Failed to load font \"%s\"", arial_font_path);
		});
		TextID hello = m_systems.text.add_text_node(arial_font_16, "Hello", { 0.0f, 0.0f });
		TextID world = m_systems.text.add_text_node(arial_font_16, "World", { 0.0f, 18.0f });
		m_scene_graph.add_text_node(m_scene_graph.root(), hello);
		m_scene_graph.add_text_node(m_scene_graph.root(), world);
	}

	void Engine::load_data(const char* path_str) {
		std::filesystem::path path = std::filesystem::path(path_str);
		if (std::filesystem::is_regular_file(path)) {
			std::vector<uint8_t> data = platform::read_file_bytes(path).value();
			m_project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
				ABORT("Could not parse json file \"%s\": %s", path_str, error.c_str());
			});
			LOG_INFO("Game data loaded from \"%s\"", path_str);
		}
		else {
			LOG_ERROR("Could not load game data from path \"%s\"", path_str);
		}
	}

	void Engine::update(const platform::Input& input, platform::PlatformAPI* platform) {
		m_window_resolution = input.window_resolution;
		m_game_is_running = input.mode == platform::RunMode::Game;

		/* Quit */
		{
			// editor handles quit in editor mode
			if (m_game_is_running) {
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

		/* Window */
		{
			if (input.keyboard.key_pressed_now(SDLK_F11)) {
				platform->toggle_fullscreen();
			}
		}

		/* Debug UI */
		{
			if (input.keyboard.key_pressed_now(SDLK_F3)) {
				m_debug_ui.show_debug_ui = !m_debug_ui.show_debug_ui;
			}

			if (m_debug_ui.show_debug_ui) {
				draw_imgui(&m_debug_ui, platform, input);
			}
		}

		/* Modules */
		{
			std::string window_title = m_project.name;
			update_hot_reloading(&m_hot_reloading, &m_systems.animation, input, platform, &window_title);
			platform->set_window_title(window_title.c_str());
			//if (input.is_editor_mode) {
			//	window_title += std::string(m_editor.project_has_unsaved_changes ? "*" : "") + " - Engine2024";
			//}
			//if (input.mode == platform::RunMode::Editor) {
			//	editor::update_editor(
			//		&m_editor,
			//		&m_game,
			//		&m_project,
			//		&m_systems,
			//		&m_scene_graph,
			//		input,
			//		platform
			//	);
			//}
		}
	}

	void Engine::render(platform::Renderer* renderer) const {
		if (m_game_is_running) {
			_render_game(renderer);
		}
	}

	void Engine::_render_game(platform::Renderer* renderer) const {
		// clear
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, m_window_resolution }, platform::Color::black);

		// render text
		glm::vec2 window_center = m_window_resolution / 2.0f;
		for (const auto& [node_id, text_node] : m_systems.text.text_nodes()) {
			const platform::Font& font = m_systems.text.fonts().at(text_node.font_id);
			renderer->draw_text(font, text_node.text, window_center + text_node.position, platform::Color::white);
		}
	}

} // namespace engine

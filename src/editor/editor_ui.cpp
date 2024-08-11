#include <editor/editor_ui.h>

#include <editor/ui/main_menu_bar.h>
#include <engine/state/engine_state.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/debug/logging.h>
#include <platform/graphics/font.h>
#include <platform/input/input.h>
#include <platform/os/imwin32.h>

#include <algorithm>
#include <format>

namespace editor {

	constexpr char LOG_WINDOW[] = "Log";
	constexpr char SCENE_WINDOW[] = "Scene";
	constexpr char PROJECT_WINDOW[] = "Project";
	constexpr char GAME_WINDOW[] = "Game";

	static void setup_docking_space(ImGuiID dockspace) {
		/* Create docks */
		ImGui::DockBuilderAddNode(dockspace); // Create a new dock node to use
		ImGui::DockBuilderSetNodeSize(dockspace, ImVec2 { 1, 1 });

		// +---------------+
		// |               |
		// |               |
		// |       1       |
		// |               |
		// |               |
		// +---------------+
		ImGuiID dock1 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.25f, nullptr, &dockspace);

		// // +---------------+
		// // |               |
		// // |       2       |   ^
		// // |               |   |
		// // +---------------+ split
		// // |       1       |
		// // +---------------+
		ImGuiID dock2 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.75f, nullptr, &dockspace);

		// //    <- split
		// // +-----+---------+
		// // |     |         |
		// // |  3  |    2    |
		// // |     |         |
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock3 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.5f, nullptr, &dockspace);

		// // +-----+---------+
		// // |  3  |         |  split
		// // +-----+    2    |    |
		// // |  4  |         |    V
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock4 = ImGui::DockBuilderSplitNode(dock3, ImGuiDir_Down, 0.5f, nullptr, &dock3);

		/* Add windows to docks */
		ImGui::DockBuilderDockWindow(LOG_WINDOW, dock1);
		ImGui::DockBuilderDockWindow(SCENE_WINDOW, dock2);
		ImGui::DockBuilderDockWindow(PROJECT_WINDOW, dock3);
		ImGui::DockBuilderDockWindow(GAME_WINDOW, dock4);
		ImGui::DockBuilderFinish(dockspace);
	}

	void init_editor_ui(
		EditorUiState* ui,
		const engine::ProjectState& project,
		bool reset_docking
	) {
		init_editor_scene_view(&ui->scene_view);

		ui->window_canvas = platform::add_canvas(1, 1);
		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<engine::ProjectState>()(project);
		ui->scene_view_position_initialized = false;

		/* Setup docking */
		if (reset_docking) {
			ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			setup_docking_space(dockspace);
		}
	}

	void shutdown_editor_ui(const EditorUiState& ui) {
		platform::free_canvas(ui.window_canvas);
		shutdown_editor_scene_view(ui.scene_view);
	}

	static ImU32 log_severity_to_color(plog::Severity severity) {
		ImU32 color = 0;
		switch (severity) {
			case plog::Severity::verbose:
			case plog::Severity::debug:
				color = IM_COL32(59, 215, 226, 255);
				break;

			case plog::Severity::info:
				color = IM_COL32(255, 255, 255, 255);
				break;

			case plog::Severity::warning:
				color = IM_COL32(255, 216, 96, 255);
				break;

			case plog::Severity::error:
			case plog::Severity::fatal:
				color = IM_COL32(255, 0, 0, 255);
				break;
		}
		return color;
	}

	std::vector<editor::EditorCommand> update_editor_ui(
		EditorUiState* ui,
		engine::GameState* game,
		engine::ProjectState* project,
		const platform::Input& input,
		const engine::Resources& /* resources */,
		bool unsaved_changes,
		bool game_is_running
	) {
		std::vector<editor::EditorCommand> commands;

		/* Quit */
		if (input.quit_signal_received || input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(editor::EditorCommand::Quit);
		}

		/* Window Canvas */
		if (ui->window_canvas.texture.size != input.monitor_size) {
			platform::free_canvas(ui->window_canvas);
			ui->window_canvas = platform::add_canvas((int)input.monitor_size.x, (int)input.monitor_size.y);
			LOG_INFO("Resized scene window canvas");
		}

		/* Dockspace */
		ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		/* Editor Menu Bar*/
		bool reset_window_layout = false;
		update_main_menu_bar(unsaved_changes, game_is_running, &commands, &reset_window_layout, &ui->show_imgui_demo);
		if (reset_window_layout) {
			setup_docking_space(dockspace);
		}

		/* ImGui Demo */
		if (ui->show_imgui_demo) {
			ImGui::ShowDemoWindow(&ui->show_imgui_demo);
		}

		/* Log Window */
		if (ImGui::Begin(LOG_WINDOW)) {
			if (input.log) {
				// Print log messages
				for (const platform::LogEntry& entry : *input.log) {
					ImGui::PushStyleColor(ImGuiCol_Text, log_severity_to_color(entry.severity));
					ImGui::Text("%s", entry.message.c_str());
					ImGui::PopStyleColor();
				}

				// Auto-scroll on new messages unless scroll position is set by user
				ui->last_num_seen_log_entries = input.log->size();
				const float scroll_y = ImGui::GetScrollY();
				const float scroll_max = ImGui::GetScrollMaxY();
				const float text_height = ImGui::GetTextLineHeightWithSpacing();
				const int lines_to_count = 5;
				const bool scrolled_up = scroll_y <= scroll_max - lines_to_count * text_height;
				if (ui->last_num_seen_log_entries.just_changed() && !scrolled_up) {
					ImGui::SetScrollHereY();
				}
			}
		}
		ImGui::End();

		/* Project Window */
		if (ImGui::Begin(PROJECT_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			const int step = 1;
			ImGui::InputScalar("Project Counter", ImGuiDataType_S16, &project->counter, &step, NULL, "%d");

			ImGui::Text("Unsaved changes: %s", unsaved_changes ? "yes" : "no");

			if (ImGui::InputText("Project name", &ui->project_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
				project->name = ui->project_name_buf;
			}

			ImGui::Text("Project path: %s", project->path.string().c_str());

			ImGui::Text("Window is maximized: %s", input.window->is_maximized() ? "true" : "false");
		}
		ImGui::End();

		/* Game Edit Window */
		if (ImGui::Begin(GAME_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			if (game_is_running) {
				const int step = 1;
				ImGui::InputScalar("Game Counter", ImGuiDataType_S16, &game->counter, &step, NULL, "%d");

				if (ImGui::Button("Resume game")) {
					commands.push_back(editor::EditorCommand::RunGame);
				}
				if (ImGui::Button("Stop game")) {
					commands.push_back(editor::EditorCommand::ResetGameState);
				}
				if (ImGui::Button("Restart game")) {
					commands.push_back(editor::EditorCommand::ResetGameState);
					commands.push_back(editor::EditorCommand::RunGame);
				}
			}
			else {
				if (ImGui::Button("Run game")) {
					commands.push_back(editor::EditorCommand::ResetGameState);
					commands.push_back(editor::EditorCommand::RunGame);
				}
			}

			ImGui::Checkbox("Windowed mode", &ui->run_game_windowed);
		}
		ImGui::End();

		/* Scene Window */
		ui->scene_window_visible = ImGui::Begin(SCENE_WINDOW);
		if (ui->scene_window_visible) {
			const glm::vec2 scene_window_pos = ImGui::GetCursorScreenPos();
			const glm::vec2 window_relative_mouse_pos = input.mouse.pos - scene_window_pos;
			glm::vec2 scene_window_size = ImGui::GetContentRegionAvail();

			// Initialize scene view
			static int counter = 0; // imgui needs 1 frame before window sizes are correct
			if (!ui->scene_view_position_initialized && counter++ > 0) {
				ui->scene_view_position_initialized = true;
				// Place scene view in center of window
				ui->scene_view.scaled_canvas_rect.set_position((scene_window_size - ui->scene_view.scaled_canvas_rect.size()) / 2.0f);
			}

			// Render scene texture
			{
				const platform::Texture& scene_texture = ui->window_canvas.texture;
				glm::vec2 top_left = { 0.0f, 1.0f };
				glm::vec2 bottom_right = {
					std::clamp(scene_window_size.x / scene_texture.size.x, 0.0f, 1.0f),
					std::clamp(1.0f - scene_window_size.y / scene_texture.size.y, 0.0f, 1.0f),
				};
				ImGui::Image(scene_texture.id, scene_window_size, top_left, bottom_right);
			}

			// Update scene view
			{
				const core::Rect scene_window_rect = core::Rect::with_pos_and_size(ImGui::GetWindowPos(), ImGui::GetWindowSize());
				std::vector<editor::EditorCommand> scene_view_commands = update_editor_scene_view(
					&ui->scene_view,
					input,
					window_relative_mouse_pos,
					scene_window_rect
				);
				commands.append_range(scene_view_commands);
			}
		}
		ImGui::End();

		return commands;
	}

	void render_editor_ui(
		const EditorUiState& ui,
		platform::Renderer* renderer
	) {
		// Only render scene if ImGui scene window open
		if (ui.scene_window_visible) {
			/* Render scene to canvas */
			renderer->set_draw_canvas(ui.scene_view.canvas);
			render_editor_scene_view(ui.scene_view, renderer);
			renderer->reset_draw_canvas();

			/* Render scene canvas to imgui canvas */
			renderer->set_draw_canvas(ui.window_canvas);
			renderer->draw_rect_fill(core::Rect { glm::vec2 { 0.0f, 0.0f }, ui.window_canvas.texture.size }, glm::vec4 { 0.0f, 0.5f, 0.5f, 1.0f });
			renderer->draw_texture(ui.scene_view.canvas.texture, ui.scene_view.scaled_canvas_rect);
			renderer->reset_draw_canvas();
		}
	}

} // namespace editor

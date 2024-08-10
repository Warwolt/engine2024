#include <engine/editor/editor_ui.h>

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

namespace engine {

	constexpr char PROJECT_WINDOW[] = "Project";
	constexpr char GAME_WINDOW[] = "Game";
	constexpr char SCENE_WINDOW[] = "Scene";

	static void setup_docking_space(ImGuiID dockspace) {
		/* Create docks */
		ImGui::DockBuilderAddNode(dockspace); // Create a new dock node to use
		ImGui::DockBuilderSetNodeSize(dockspace, ImVec2 { 1, 1 });

		ImGuiID dock1 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.75f, nullptr, &dockspace);
		// +-----------+
		// |           |
		// |     1     |
		// |           |
		// +-----------+

		ImGuiID dock2 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.5f, nullptr, &dockspace);
		// +-----+-----+
		// |   |       |
		// | 2 |   1   |
		// |   |       |
		// +-----+-----+
		//    <- split

		ImGuiID dock3 = ImGui::DockBuilderSplitNode(dock2, ImGuiDir_Down, 0.5f, nullptr, &dock2);
		// +-----+-----+
		// | 2 |       |  split
		// +---+   1   |    |
		// | 3 |       |    V
		// +-----+-----+

		/* Add windows to docks */
		ImGui::DockBuilderDockWindow(SCENE_WINDOW, dock1);
		ImGui::DockBuilderDockWindow(PROJECT_WINDOW, dock2);
		ImGui::DockBuilderDockWindow(GAME_WINDOW, dock3);
		ImGui::DockBuilderFinish(dockspace);
	}

	void init_editor_ui(
		EditorUiState* ui,
		const ProjectState& project,
		bool reset_docking
	) {
		ui->window_canvas = platform::add_canvas(1, 1);
		constexpr int scene_canvas_width = 320;
		constexpr int scene_canvas_height = 320;
		ui->scene_canvas_size = { scene_canvas_width, scene_canvas_height };
		ui->scaled_scene_canvas_rect = platform::Rect { { 0, 0 }, { scene_canvas_width, scene_canvas_height } };
		ui->scene_canvas = platform::add_canvas(scene_canvas_width, scene_canvas_height);

		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<ProjectState>()(project);

		/* Setup docking */
		if (reset_docking) {
			ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			setup_docking_space(dockspace);
		}
	}

	void shutdown_editor_ui(const EditorUiState& ui) {
		platform::free_canvas(ui.window_canvas);
		platform::free_canvas(ui.scene_canvas);
	}

	std::vector<EditorCommand> update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		const engine::Resources& /* resources */,
		bool unsaved_changes,
		bool game_is_running
	) {
		std::vector<EditorCommand> commands;

		/* Quit */
		if (input.quit_signal_received || input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(EditorCommand::Quit);
		}

		/* Window Canvas */
		if (ui->window_canvas.texture.size != input.monitor_size) {
			platform::free_canvas(ui->window_canvas);
			ui->window_canvas = platform::add_canvas((int)input.monitor_size.x, (int)input.monitor_size.y);
			LOG_INFO("Resized window canvas");
		}

		/* Dockspace */
		ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		/* Editor Menu Bar*/
		if (ImWin32::BeginMainMenuBar()) {
			if (ImWin32::BeginMenu(L"&File")) {
				if (ImWin32::MenuItem(L"&New Project\tCtrl+N")) {
					commands.push_back(EditorCommand::NewProject);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Open Project\tCtrl+O")) {
					commands.push_back(EditorCommand::OpenProject);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Save Project\tCtrl+S", unsaved_changes)) {
					commands.push_back(EditorCommand::SaveProject);
				}

				if (ImWin32::MenuItem(L"Save Project &As\tCtrl+Shift+S")) {
					commands.push_back(EditorCommand::SaveProjectAs);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Quit\tCtrl+Q")) {
					commands.push_back(EditorCommand::Quit);
				}

				ImWin32::EndMenu();
			}

			if (ImWin32::BeginMenu(L"&Run")) {
				if (game_is_running) {
					if (ImWin32::MenuItem(L"Resume game")) {
						commands.push_back(EditorCommand::RunGame);
					}
					if (ImWin32::MenuItem(L"Stop game")) {
						commands.push_back(EditorCommand::ResetGameState);
					}
					if (ImWin32::MenuItem(L"Restart game")) {
						commands.push_back(EditorCommand::ResetGameState);
						commands.push_back(EditorCommand::RunGame);
					}
				}
				else {
					if (ImWin32::MenuItem(L"Run game")) {
						commands.push_back(EditorCommand::ResetGameState);
						commands.push_back(EditorCommand::RunGame);
					}
				}

				if (ImWin32::MenuItem(L"Show ImGui Demo")) {
					ui->show_imgui_demo = true;
				}

				ImWin32::EndMenu();
			}

			if (ImWin32::BeginMenu(L"&Window")) {
				if (ImWin32::MenuItem(L"Reset Window Layout")) {
					setup_docking_space(dockspace);
				}
			}

			ImWin32::EndMainMenuBar();
		}

		/* ImGui Demo */
		if (ui->show_imgui_demo) {
			ImGui::ShowDemoWindow(&ui->show_imgui_demo);
		}

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
					commands.push_back(EditorCommand::RunGame);
				}
				if (ImGui::Button("Stop game")) {
					commands.push_back(EditorCommand::ResetGameState);
				}
				if (ImGui::Button("Restart game")) {
					commands.push_back(EditorCommand::ResetGameState);
					commands.push_back(EditorCommand::RunGame);
				}
			}
			else {
				if (ImGui::Button("Run game")) {
					commands.push_back(EditorCommand::ResetGameState);
					commands.push_back(EditorCommand::RunGame);
				}
			}

			ImGui::Checkbox("Windowed mode", &ui->run_game_windowed);
		}
		ImGui::End();

		/* Scene Window */
		if (ImGui::Begin(SCENE_WINDOW)) {
			const glm::vec2 scene_window_pos = ImGui::GetCursorScreenPos();
			ui->scene_window_size = ImGui::GetContentRegionAvail();

			// Render scene texture
			{
				const platform::Texture& scene_texture = ui->window_canvas.texture;
				glm::vec2 top_left = { 0.0f, 1.0f };
				glm::vec2 bottom_right = {
					std::clamp(ui->scene_window_size.x / scene_texture.size.x, 0.0f, 1.0f),
					std::clamp(1.0f - ui->scene_window_size.y / scene_texture.size.y, 0.0f, 1.0f),
				};
				ImGui::Image(scene_texture.id, ui->scene_window_size, top_left, bottom_right);
			}

			const glm::vec2 window_relative_mouse_pos = input.mouse.pos - scene_window_pos;

			if (ImGui::Begin("Debug")) {
				ImGui::Text("window_relative_mouse_pos = %f %f", window_relative_mouse_pos.x, window_relative_mouse_pos.y);
				ImGui::Text("ui->scaled_scene_canvas_rect");
				ImGui::Text("\ttop_left: %f %f", ui->scaled_scene_canvas_rect.top_left.x, ui->scaled_scene_canvas_rect.top_left.y);
				ImGui::Text("\tbottom_right: %f %f", ui->scaled_scene_canvas_rect.bottom_right.x, ui->scaled_scene_canvas_rect.bottom_right.y);
			}
			ImGui::End();

			// Scene view hover
			{
				ImVec2 window_pos = ImGui::GetWindowPos();
				ImVec2 window_size = ImGui::GetWindowSize();
				bool mouse_overlaps_horizontally = window_pos.x <= input.mouse.pos.x && input.mouse.pos.x <= window_pos.x + window_size.x;
				bool mouse_overlaps_vertically = window_pos.y <= input.mouse.pos.y && input.mouse.pos.y <= window_pos.y + window_size.y;
				ui->scene_window_hovered = mouse_overlaps_horizontally && mouse_overlaps_vertically;
			}

			// Zoom
			if (ui->scene_window_hovered) {
				constexpr int min_zoom = -12;
				constexpr int max_zoom = 12;
				const int new_zoom_index = std::clamp(ui->scene_zoom_index + input.mouse.scroll_delta, min_zoom, max_zoom);
				const bool zoom_has_changed = ui->scene_zoom_index != new_zoom_index;
				ui->scene_zoom_index = new_zoom_index;
				const float scale_up_factors[max_zoom + 1] = {
					1.0f,
					2.0f,
					3.0f,
					4.0f,
					5.0f,
					6.0f,
					8.0f,
					12.0f,
					16.0f,
					24.0f,
					32.0f,
					48.0f,
					64.0f,
				};
				const float scale_down_factors[-min_zoom] = {
					0.5f,
					0.33f,
					0.25f,
					0.20f,
					0.167f,
					0.125f,
					0.083f,
					0.062f,
					0.042f,
					0.031f,
					0.021f,
					0.016f,
				};
				const float scale = ui->scene_zoom_index < 0 ? scale_down_factors[-ui->scene_zoom_index - 1] : scale_up_factors[ui->scene_zoom_index];

				/* Move canvas when zooming */
				if (zoom_has_changed) {
					// 	// Position the canvas so that the zoom happens around the cursor,
					// 	// by keeping the relative distance from cursor to corner the same.
					// 	//
					// 	// +--------------+
					// 	// |      |       |      +-------+
					// 	// |      |       |      |   |   |      +---+
					// 	// |------o       |  =>  |---o   |  =>  | o |
					// 	// |              |      |       |      +---+
					// 	// |              |      +-------+
					// 	// |              |
					// 	// +--------------+
					// 	//
					const glm::vec2 new_scaled_size = ui->scene_canvas_size * scale;
					const glm::vec2 distance_to_top_left = window_relative_mouse_pos - ui->scaled_scene_canvas_rect.top_left;
					const float relative_scale = new_scaled_size.x / ui->scaled_scene_canvas_rect.size().x;
					ui->scaled_scene_canvas_rect.set_size(new_scaled_size);
					ui->scaled_scene_canvas_rect.set_position(window_relative_mouse_pos - relative_scale * distance_to_top_left);
				}
			}
		}
		else {
			ui->scene_window_size = { 0.0f, 0.0f }; // inactive
		}
		ImGui::End();

		if (ImGui::Begin("Debug")) {
			ImGui::Spacing();
			if (ImGui::Button("Reset")) {
				ui->scaled_scene_canvas_rect.set_position(glm::vec2 { 250, 150 });
				ui->scene_zoom_index = 0;
				ui->scaled_scene_canvas_rect.set_size(ui->scene_canvas_size);
			}
		}
		ImGui::End();

		return commands;
	}

	static void render_scene(
		const EditorUiState& ui,
		const engine::Resources& resources,
		platform::Renderer* renderer
	) {
		const glm::vec2 scene_canvas_size = ui.scene_canvas.texture.size;

		// Clear scene
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, scene_canvas_size }, { 0.75f, 0.75f, 0.75f, 1.0f });

		// Coordinate Axes
		renderer->draw_line({ 0.0f, scene_canvas_size.y / 2.0f }, { scene_canvas_size.x, scene_canvas_size.y / 2.0f }, glm::vec4 { 1.0f, 0.0f, 0.0f, 1.0f });
		renderer->draw_line({ scene_canvas_size.x / 2.0f, 0.0f }, { scene_canvas_size.x / 2.0f, scene_canvas_size.y }, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f });

		// Hover text
		if (ui.scene_window_hovered) {
			renderer->draw_text_centered(resources.fonts.at("arial-16"), "Hovered", scene_canvas_size / 2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
		}

		// Zoom text
		std::string text = std::format("Zoom: {}", ui.scene_zoom_index);
		renderer->draw_text(resources.fonts.at("arial-16"), text.c_str(), { 64.0f, 64.0f }, { 0.0f, 0.0f, 0.0f, 1.0f });
	}

	void render_editor_ui(
		const EditorUiState& ui,
		const engine::Resources& resources,
		platform::Renderer* renderer
	) {
		// Only render scene if ImGui scene window open
		if (ui.scene_window_size != glm::vec2 { 0.0f, 0.0f }) {
			/* Render scene to canvas */
			renderer->set_draw_canvas(ui.scene_canvas);
			render_scene(ui, resources, renderer);
			renderer->reset_draw_canvas();

			/* Render scene canvas to imgui canvas */
			renderer->set_draw_canvas(ui.window_canvas);
			renderer->draw_rect_fill(platform::Rect { glm::vec2 { 0.0f, 0.0f }, ui.window_canvas.texture.size }, glm::vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
			renderer->draw_texture(ui.scene_canvas.texture, ui.scaled_scene_canvas_rect);
			renderer->reset_draw_canvas();
		}
	}

} // namespace engine

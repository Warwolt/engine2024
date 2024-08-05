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
		ui->scene_canvas = platform::add_canvas(1, 1);
		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<ProjectState>()(project);

		/* Setup docking */
		if (reset_docking) {
			ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			setup_docking_space(dockspace);
		}
	}

	void shutdown_editor_ui(const EditorUiState& ui) {
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

		/* Scene Canvas */
		if (ui->scene_canvas.texture.size != input.monitor_size) {
			platform::free_canvas(ui->scene_canvas);
			ui->scene_canvas = platform::add_canvas((int)input.monitor_size.x, (int)input.monitor_size.y);
			LOG_INFO("Resized scene canvas");
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
			// FIXME: we should calculate UV coordinates so that we clip the
			// canvas to fit the scene window (just render top left corner)
			ImGui::Image(ui->scene_canvas.texture.id, ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 });
		}
		ImGui::End();

		return commands;
	}

	void render_editor_ui(
		const EditorUiState& ui,
		const engine::Resources& resources,
		platform::Renderer* renderer
	) {
		renderer->set_draw_canvas(ui.scene_canvas);

		renderer->draw_rect_fill({ { 0, 0 }, ui.scene_canvas.texture.size }, { 0.75, 0.75, 0.75, 1.0 });

		renderer->draw_text(resources.fonts.at("arial-16"), "Editor", { 100.0, 100.0 }, { 0.0, 0.0, 0.0, 1.0 });

		renderer->reset_draw_canvas();
	}

} // namespace engine

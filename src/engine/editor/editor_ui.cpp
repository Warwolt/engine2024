#include <engine/editor/editor_ui.h>

#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/input/input.h>
#include <platform/os/imwin32.h>

namespace engine {

	constexpr char PROJECT_WINDOW[] = "Project Window";
	constexpr char GAME_WINDOW[] = "Game Window";
	constexpr char SCENE_WINDOW[] = "Scene Window";

	void init_editor_ui(
		EditorUiState* ui,
		const ProjectState& project,
		bool reset_docking
	) {
		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<ProjectState>()(project);

		/* Setup docking */
		if (reset_docking) {
			/* Create docks */
			ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			ImGui::DockBuilderAddNode(id); // Create a new dock node to use
			ImGui::DockBuilderSetNodeSize(id, ImVec2 { 1, 1 });

			ImGuiID dock1 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.75f, nullptr, &id);
			// +-----------+
			// |           |
			// |     1     |
			// |           |
			// +-----------+

			ImGuiID dock2 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.5f, nullptr, &id);
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
			ImGui::DockBuilderFinish(id);
		}
	}

	std::vector<EditorCommand> update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		bool unsaved_changes,
		bool game_is_running
	) {
		std::vector<EditorCommand> commands;

		/* Quit */
		if (input.quit_signal_received || input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(EditorCommand::Quit);
		}

		/* Dockspace */
		{
			ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		}

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

			ImGui::End();
		}

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

			ImGui::End();
		}

		/* Scene Window */
		if (ImGui::Begin(SCENE_WINDOW)) {
			ImGui::End();
		}

		return commands;
	}

} // namespace engine

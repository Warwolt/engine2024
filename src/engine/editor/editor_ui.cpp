#include <engine/editor/editor_ui.h>

#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/input/input.h>
#include <platform/os/imwin32.h>

namespace engine {

	void init_editor_ui(EditorUiState* ui, const ProjectState& project) {
		ui->project_name_buf = project.name;
		ui->cached_project_hash = std::hash<ProjectState>()(project);

		/* Setup docking */
		{
			ImVec2 work_center = ImGui::GetMainViewport()->GetWorkCenter();

			// 3. Now we'll need to create our dock node:
			ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			ImGui::DockBuilderRemoveNode(id); // Clear any preexisting layouts associated with the ID we just chose
			ImGui::DockBuilderAddNode(id); // Create a new dock node to use

			// 4. Set the dock node's size and position:
			ImVec2 size { 600, 300 }; // A decently large dock node size (600x300px) so everything displays clearly

			// Calculate the position of the dock node
			//
			// `DockBuilderSetNodePos()` will position the top-left corner of the node to the coordinate given.
			// This means that the node itself won't actually be in the center of the window; its top-left corner will.
			//
			// To fix this, we'll need to subtract half the node size from both the X and Y dimensions to move it left and up.
			// This new coordinate will be the position of the node's top-left corner that will center the node in the window.
			ImVec2 node_pos { work_center.x - size.x * 0.5f, work_center.y - size.y * 0.5f };

			// Set the size and position:
			ImGui::DockBuilderSetNodeSize(id, size);
			ImGui::DockBuilderSetNodePos(id, node_pos);

			// 5. Split the dock node to create spaces to put our windows in:

			// Split the dock node in the left direction to create our first docking space. This will be on the left side of the node.
			// (The 0.5f means that the new space will take up 50% of its parent - the dock node.)
			ImGuiID dock1 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.5f, nullptr, &id);
			// +-----------+
			// |           |
			// |     1     |
			// |           |
			// +-----------+

			// Split the same dock node in the right direction to create our second docking space.
			// At this point, the dock node has two spaces, one on the left and one on the right.
			ImGuiID dock2 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);
			// +-----+-----+
			// |     |     |
			// |  1  |  2  |
			// |     |     |
			// +-----+-----+
			//    split ->

			// For our last docking space, we want it to be under the second one but not under the first.
			// Split the second space in the down direction so that we now have an additional space under it.
			//
			// Notice how "dock2" is now passed rather than "id".
			// The new space takes up 50% of the second space rather than 50% of the original dock node.
			ImGuiID dock3 = ImGui::DockBuilderSplitNode(dock2, ImGuiDir_Down, 0.5f, nullptr, &dock2);
			// +-----+-----+
			// |     |  2  |  split
			// |  1  +-----+    |
			// |     |  3  |    V
			// +-----+-----+

			// 6. Add windows to each docking space:
			ImGui::DockBuilderDockWindow("One", dock1);
			ImGui::DockBuilderDockWindow("Two", dock2);
			ImGui::DockBuilderDockWindow("Three", dock3);

			// 7. We're done setting up our docking configuration:
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

			ImGui::Begin("One");
			ImGui::End();

			ImGui::Begin("Two");
			ImGui::End();

			ImGui::Begin("Three");
			ImGui::End();
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

				ImWin32::EndMenu();
			}
			ImWin32::EndMainMenuBar();
		}

		/* Project Window */
		if (0 && ImGui::Begin("Project Window", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			const int step = 1;
			ImGui::InputScalar("Project Counter", ImGuiDataType_S16, &project->counter, &step, NULL, "%d");

			ImGui::Text("Unsaved changes: %s", unsaved_changes ? "yes" : "no");

			if (ImGui::InputText("Project name", &ui->project_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
				project->name = ui->project_name_buf;
			}

			ImGui::Text("Project path: %s", project->path.string().c_str());

			ImGui::End();
		}

		/* Game Edit Window */
		if (0 && ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
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

		return commands;
	}

} // namespace engine

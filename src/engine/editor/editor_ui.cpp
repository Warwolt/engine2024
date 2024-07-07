#include <engine/editor/editor_ui.h>

#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/input/input.h>

namespace engine {

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

		/* Editor Menu Bar*/
		// if (ImGui::BeginMainMenuBar()) {
		// 	if (ImGui::BeginMenu("File")) {
		// 		if (ImGui::MenuItem(" New Project")) {
		// 			commands.push_back(EditorCommand::NewProject);
		// 		}

		// 		ImGui::Separator();

		// 		if (ImGui::MenuItem(" Open Project")) {
		// 			commands.push_back(EditorCommand::OpenProject);
		// 		}

		// 		ImGui::Separator();

		// 		if (ImGui::MenuItem(" Save Project", NULL, false, unsaved_changes)) {
		// 			commands.push_back(EditorCommand::SaveProject);
		// 		}

		// 		if (ImGui::MenuItem(" Save Project As")) {
		// 			commands.push_back(EditorCommand::SaveProjectAs);
		// 		}

		// 		ImGui::EndMenu();
		// 	}
		// 	ImGui::EndMainMenuBar();
		// }

		/* Project Window */
		if (ImGui::Begin("Project Window", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
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
		if (ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
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

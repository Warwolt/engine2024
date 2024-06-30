#include <engine/editor.h>

#include <core/container.h>
#include <engine/game_state.h>
#include <engine/project.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/platform_api.h>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>

namespace engine {

	enum class EditorCommand {
		NewProject,
		LoadProject,
		SaveProject,
		ResetGameState,
		RunGame,
	};

	static std::vector<EditorCommand> update_editor_ui(EditorUiState* ui, GameState* game, ProjectState* project) {
		std::vector<EditorCommand> commands;

		/* Editor Menu Bar*/
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					commands.push_back(EditorCommand::NewProject);
				}

				if (ImGui::MenuItem("Load Project")) {
					commands.push_back(EditorCommand::LoadProject);
				}

				if (ImGui::MenuItem("Save Project")) {
					commands.push_back(EditorCommand::SaveProject);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		size_t current_project_hash = std::hash<ProjectState>()(*project);
		const bool unsaved_changes = ui->loaded_project_hash != current_project_hash;

		/* Editor Window */
		if (ImGui::Begin("Editor Window")) {
			const int step = 1;
			ImGui::InputScalar("Counter", ImGuiDataType_S16, &game->counter, &step, NULL, "%d");

			ImGui::Text("Unsaved changes: %s", unsaved_changes ? "yes" : "no");

			if (ImGui::InputText("Project name", &ui->project_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
				project->name = ui->project_name_buf;
			}

			if (ImGui::Button("Run game")) {
				commands.push_back(EditorCommand::ResetGameState);
				commands.push_back(EditorCommand::RunGame);
			}

			if (ImGui::Button("Resume game")) {
				commands.push_back(EditorCommand::RunGame);
			}

			ImGui::End();
		}

		return commands;
	}

	static std::optional<nlohmann::json> get_loaded_project_data(std::future<std::vector<uint8_t>>* project_data) {
		if (core::container::future_has_value(*project_data)) {
			std::vector<uint8_t> buffer = project_data->get();
			if (!buffer.empty()) {
				return nlohmann::json::parse(buffer);
			}
		}
		return {};
	}

	void init_editor(EditorState* editor, const ProjectState* project) {
		editor->ui.project_name_buf = project->name;
		editor->ui.loaded_project_hash = std::hash<ProjectState>()(*project);
	}

	void update_editor(
		EditorState* editor,
		GameState* game,
		ProjectState* project,
		const platform::Input* input,
		platform::PlatformAPI* platform
	) {
		/* Input */
		const bool editor_is_running = input->mode == platform::RunMode::Editor;
		const std::optional<nlohmann::json> loaded_project_data = get_loaded_project_data(&editor->input.project_data);

		/* Process input */
		if (loaded_project_data.has_value()) {
			nlohmann::json json_object = loaded_project_data.value();
			project->name = json_object["project_name"];
			editor->ui.project_name_buf = project->name;
			editor->ui.loaded_project_hash = std::hash<ProjectState>()(*project);
		}

		/* Run UI */
		std::vector<EditorCommand> commands;
		if (editor_is_running) {
			commands = update_editor_ui(&editor->ui, game, project);
		}

		/* Process commands */
		for (const EditorCommand& cmd : commands) {
			switch (cmd) {
				case EditorCommand::NewProject:
					*game = {};
					break;

				case EditorCommand::LoadProject: {
					platform::FileExplorerDialog dialog = {
						.title = "Load project",
						.description = "(PAK *.pak)",
						.extension = "pak",
					};
					editor->input.project_data = platform->load_file_with_dialog(dialog);
				} break;

				case EditorCommand::SaveProject: {
					nlohmann::json json_object = {
						{ "project_name", project->name }
					};
					std::string data = json_object.dump();
					platform::FileExplorerDialog dialog = {
						.title = "Save project",
						.description = "PAK (*.pak)",
						.extension = "pak",
					};
					platform->save_file_with_dialog(std::vector<uint8_t>(data.begin(), data.end()), dialog);
				} break;

				case EditorCommand::ResetGameState:
					game->counter = 0;
					game->time_ms = 0;
					break;

				case EditorCommand::RunGame:
					platform->set_run_mode(platform::RunMode::Game);
					break;
			}
		}
	}

} // namespace engine

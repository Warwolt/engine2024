#include <engine/editor.h>

#include <core/container.h>
#include <core/future.h>
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
		Quit,
	};

	static std::vector<EditorCommand> update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		const platform::Input* input,
		bool unsaved_changes
	) {
		std::vector<EditorCommand> commands;

		/* Quit */
		if (input->quit_signal_received || input->keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(EditorCommand::Quit);
		}

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

		/* Editor Window */
		if (ImGui::Begin("Editor Window")) {
			const int step = 1;
			ImGui::InputScalar("Counter", ImGuiDataType_S16, &game->counter, &step, NULL, "%d");

			ImGui::Text("Unsaved changes: %s", unsaved_changes ? "yes" : "no");

			if (ImGui::InputText("Project name", &ui->project_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
				project->name = ui->project_name_buf;
			}

			ImGui::Text("Project path: %s", project->path.string().c_str());

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

	static std::string serialize_project_to_json_string(const ProjectState* project) {
		nlohmann::json json_object = {
			{ "project_name", project->name }
		};
		return json_object.dump();
	}

	void init_editor(EditorState* editor, const ProjectState* project) {
		editor->ui.project_name_buf = project->name;
		editor->ui.saved_project_hash = std::hash<ProjectState>()(*project);
	}

	void update_editor(
		EditorState* editor,
		GameState* game,
		ProjectState* project,
		const platform::Input* input,
		platform::PlatformAPI* platform
	) {
		/* Run UI */
		const size_t current_project_hash = std::hash<ProjectState>()(*project);
		const bool project_has_unsaved_changes = editor->ui.saved_project_hash != current_project_hash;
		std::vector<EditorCommand> commands = update_editor_ui(&editor->ui, game, project, input, project_has_unsaved_changes);

		// TODO:
		// - warn when closing editor with unsaved changes
		// - warn when loading project if unsaved changes
		// - add save-as menu option

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
					platform->load_file_with_dialog(dialog, [project, editor](std::vector<uint8_t> data, std::filesystem::path path) {
						nlohmann::json json_object = nlohmann::json::parse(data);
						project->name = json_object["project_name"];
						project->path = path;
						editor->ui.project_name_buf = project->name;
						editor->ui.saved_project_hash = std::hash<ProjectState>()(*project);
					});

				} break;

				case EditorCommand::SaveProject: {
					const std::string json = serialize_project_to_json_string(project);
					const std::vector<uint8_t> bytes = std::vector<uint8_t>(json.begin(), json.end());
					const bool project_file_exists = !project->path.empty() && std::filesystem::is_regular_file(project->path);
					/* Save existing file */
					if (project_file_exists) {
						platform->save_file(bytes, project->path, [editor, current_project_hash]() {
							editor->ui.saved_project_hash = current_project_hash;
						});
					}
					/* Save new file */
					else {
						platform::FileExplorerDialog dialog = {
							.title = "Save project",
							.description = "PAK (*.pak)",
							.extension = "pak",
						};
						platform->save_file_with_dialog(bytes, dialog, [project, editor, current_project_hash](std::filesystem::path path) {
							project->path = path;
							editor->ui.saved_project_hash = current_project_hash;
						});
					}
				} break;

				case EditorCommand::ResetGameState:
					game->counter = 0;
					game->time_ms = 0;
					break;

				case EditorCommand::RunGame:
					platform->set_run_mode(platform::RunMode::Game);
					break;

				case EditorCommand::Quit: {
					if (project_has_unsaved_changes) {
						platform->show_unsaved_changes_dialog(project->name, [](platform::UnsavedChangesDialogChoice choice) {
							switch (choice) {
								case platform::UnsavedChangesDialogChoice::Save:
									LOG_DEBUG("Choice: Save");
									break;

								case platform::UnsavedChangesDialogChoice::DontSave:
									LOG_DEBUG("Choice: Don't Save");
									break;

								case platform::UnsavedChangesDialogChoice::Cancel:
									LOG_DEBUG("Choice: Cancel");
									break;
							}
						});
					}
					else {
						platform->quit();
					}
				} break;
			}
		}
	}

} // namespace engine

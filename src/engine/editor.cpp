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

	static void update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		bool unsaved_changes,
		std::vector<EditorCommand>* editor_cmds
	) {
		/* Editor Menu Bar*/
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					editor_cmds->push_back(EditorCommand::NewProject);
				}

				if (ImGui::MenuItem("Load Project")) {
					editor_cmds->push_back(EditorCommand::LoadProject);
				}

				if (ImGui::MenuItem("Save Project")) {
					editor_cmds->push_back(EditorCommand::SaveProject);
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
				editor_cmds->push_back(EditorCommand::ResetGameState);
				editor_cmds->push_back(EditorCommand::RunGame);
			}

			if (ImGui::Button("Resume game")) {
				editor_cmds->push_back(EditorCommand::RunGame);
			}

			ImGui::End();
		}
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
		size_t current_project_hash = std::hash<ProjectState>()(*project);
		std::vector<EditorCommand> editor_cmds;

		/* Process events */
		{
			/* On project saved */
			if (core::future::has_value(editor->input.save_project_result)) {
				const platform::SaveFileResult<std::filesystem::path> save_result = editor->input.save_project_result.get();
				if (save_result.has_value()) {
					const std::filesystem::path& path = save_result.value();
					if (!path.empty()) {
						project->path = path;
						editor->ui.saved_project_hash = current_project_hash;
					}
				}
			}

			/* On project loaded */
			if (core::future::has_value(editor->input.load_project_result)) {
				const platform::LoadFileResult<platform::LoadFileData> load_result = editor->input.load_project_result.get();
				if (load_result.has_value()) {
					const platform::LoadFileData& loaded_project = load_result.value();
					if (!loaded_project.data.empty() && !loaded_project.path.empty()) {
						nlohmann::json json_object = nlohmann::json::parse(loaded_project.data);
						project->path = loaded_project.path;
						project->name = json_object["project_name"];
						editor->ui.project_name_buf = project->name;
						current_project_hash = std::hash<ProjectState>()(*project);
						editor->ui.saved_project_hash = current_project_hash;
					}
				}
			}

			/* Quit */
			if (input->quit_signal_received || input->keyboard.key_pressed_now(SDLK_ESCAPE)) {
				const bool project_has_unsaved_changes = editor->ui.saved_project_hash != current_project_hash;
				if (project_has_unsaved_changes) {
					editor->input.save_before_quit_choice = platform->show_unsaved_changes_dialog(project->name);
				}
				else {
					platform->quit();
				}
			}

			// TODO
			// This is complex enough to warrant a big fat flowchart!
			//
			// There's enough different cases here that we need to know how to
			// unify things and make things neat and tiny now that we're in this
			// always-async development mode.
			//
			// Somehow, we need to NEATLY keep track of all these this-then-that
			// continuations without wanting to blow our brains out.

			/* Unsaved changes on quit dialog */
			if (core::future::has_value(editor->input.save_before_quit_choice)) {
				const platform::UnsavedChangesDialogChoice choice = editor->input.save_before_quit_choice.get();
				switch (choice) {
					case platform::UnsavedChangesDialogChoice::Save:
						editor_cmds.push_back(EditorCommand::SaveProject);
						// FIXME: we should not quit if we cancel the "save as" dialog
						editor_cmds.push_back(EditorCommand::Quit);
						break;

					case platform::UnsavedChangesDialogChoice::DontSave:
						editor_cmds.push_back(EditorCommand::Quit);
						break;

					case platform::UnsavedChangesDialogChoice::Cancel:
						break;
				}
			}
		}

		/* Run UI */
		const bool project_has_unsaved_changes = editor->ui.saved_project_hash != current_project_hash;
		update_editor_ui(&editor->ui, game, project, project_has_unsaved_changes, &editor_cmds);

		// TODO:
		// - warn when closing editor with unsaved changes
		// - warn when loading project if unsaved changes
		// - add save-as menu option

		/* Process commands */
		for (const EditorCommand& cmd : editor_cmds) {
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
					editor->input.load_project_result = platform->load_file_with_dialog(dialog);
				} break;

				case EditorCommand::SaveProject: {
					const std::string json_data = serialize_project_to_json_string(project);
					const bool project_file_exists = !project->path.empty() && std::filesystem::is_regular_file(project->path);
					if (project_file_exists && project_has_unsaved_changes) {
						editor->input.save_project_result = platform->save_file(std::vector<uint8_t>(json_data.begin(), json_data.end()), project->path);
					}
					if (!project_file_exists) {
						platform::FileExplorerDialog dialog = {
							.title = "Save project",
							.description = "PAK (*.pak)",
							.extension = "pak",
						};
						editor->input.save_project_result = platform->save_file_with_dialog(std::vector<uint8_t>(json_data.begin(), json_data.end()), dialog);
					}
				} break;

				case EditorCommand::ResetGameState:
					game->counter = 0;
					game->time_ms = 0;
					break;

				case EditorCommand::RunGame:
					platform->set_run_mode(platform::RunMode::Game);
					break;

				case EditorCommand::Quit:
					platform->quit();
			}
		}
	}

} // namespace engine

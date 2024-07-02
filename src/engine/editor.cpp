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
		OpenProject,
		SaveProject,
		SaveProjectAs,
		ResetGameState,
		RunGame,
		Quit,
	};

	static const platform::FileExplorerDialog g_load_project_dialog = {
		.title = "Load project",
		.description = "(PAK *.pak)",
		.extension = "pak",
	};

	static const platform::FileExplorerDialog g_save_project_dialog = {
		.title = "Save project",
		.description = "PAK (*.pak)",
		.extension = "pak",
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
				//
				//
				// FIXME: use WindowPadding here instead of spaces in the strings
				//
				//
				if (ImGui::MenuItem(" New Project")) {
					commands.push_back(EditorCommand::NewProject);
				}

				ImGui::Separator();

				if (ImGui::MenuItem(" Open Project")) {
					commands.push_back(EditorCommand::OpenProject);
				}

				ImGui::Separator();

				if (ImGui::MenuItem(" Save Project", NULL, false, unsaved_changes)) {
					commands.push_back(EditorCommand::SaveProject);
				}

				if (ImGui::MenuItem(" Save Project As")) {
					commands.push_back(EditorCommand::SaveProjectAs);
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

	static void new_project(
		EditorState* editor,
		GameState* game,
		ProjectState* project
	) {
		LOG_INFO("Opened new project");
		*project = {};
		*game = {};
		init_editor(editor, project);
	}

	static void open_project(
		EditorState* editor,
		ProjectState* project,
		platform::PlatformAPI* platform
	) {
		platform->load_file_with_dialog(g_load_project_dialog, [=](std::vector<uint8_t> data, std::filesystem::path path) {
			nlohmann::json json_object = nlohmann::json::parse(data);
			project->name = json_object["project_name"];
			project->path = path;
			editor->ui.project_name_buf = project->name;
			editor->ui.cached_project_hash = std::hash<ProjectState>()(*project);
			LOG_INFO("Opened project \"%s\"", project->name.c_str());
		});
	}

	static void save_project_as(
		EditorState* editor,
		ProjectState* project,
		platform::PlatformAPI* platform,
		size_t current_project_hash,
		std::function<void()> on_file_saved = []() {}
	) {
		const std::string json = serialize_project_to_json_string(project);
		const std::vector<uint8_t> bytes = std::vector<uint8_t>(json.begin(), json.end());
		platform->save_file_with_dialog(bytes, g_save_project_dialog, [=](std::filesystem::path path) {
			LOG_INFO("Saved project \"%s\"", project->name.c_str());
			project->path = path;
			editor->ui.cached_project_hash = current_project_hash;
			on_file_saved();
		});
	}

	static void save_project(
		EditorState* editor,
		ProjectState* project,
		platform::PlatformAPI* platform,
		size_t current_project_hash,
		std::function<void()> on_file_saved = []() {}
	) {
		const bool project_file_exists = !project->path.empty() && std::filesystem::is_regular_file(project->path);
		/* Save existing file */
		if (project_file_exists) {
			const std::string json = serialize_project_to_json_string(project);
			const std::vector<uint8_t> bytes = std::vector<uint8_t>(json.begin(), json.end());
			platform->save_file(bytes, project->path, [=]() {
				LOG_INFO("Saved project \"%s\"", project->name.c_str());
				editor->ui.cached_project_hash = current_project_hash;
				on_file_saved();
			});
		}
		/* Save new file */
		else {
			save_project_as(editor, project, platform, current_project_hash, on_file_saved);
		}
	}

	static void show_unsaved_project_changes_dialog(
		EditorState* editor,
		ProjectState* project,
		platform::PlatformAPI* platform,
		size_t current_project_hash,
		std::function<void()> on_dialog_not_cancelled = []() {}
	) {
		platform->show_unsaved_changes_dialog(project->name, [=](platform::UnsavedChangesDialogChoice choice) {
			switch (choice) {
				case platform::UnsavedChangesDialogChoice::Save:
					save_project(editor, project, platform, current_project_hash, on_dialog_not_cancelled);
					break;

				case platform::UnsavedChangesDialogChoice::DontSave:
					on_dialog_not_cancelled();
					break;

				case platform::UnsavedChangesDialogChoice::Cancel:
					break;
			}
		});
	}

	static void quit_editor(platform::PlatformAPI* platform) {
		platform->quit();
		LOG_INFO("Editor quit");
	}

	void init_editor(EditorState* editor, const ProjectState* project) {
		editor->ui.project_name_buf = project->name;
		editor->ui.cached_project_hash = std::hash<ProjectState>()(*project);
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
		const bool project_has_unsaved_changes = editor->ui.cached_project_hash != current_project_hash;
		std::vector<EditorCommand> commands = update_editor_ui(&editor->ui, game, project, input, project_has_unsaved_changes);

		/* Process commands */
		for (const EditorCommand& cmd : commands) {
			switch (cmd) {
				case EditorCommand::NewProject:
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, current_project_hash, [=]() {
							new_project(editor, game, project);
						});
					}
					else {
						new_project(editor, game, project);
					}
					break;

				case EditorCommand::OpenProject:
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, current_project_hash, [=]() {
							open_project(editor, project, platform);
						});
					}
					else {
						open_project(editor, project, platform);
					}
					break;

				case EditorCommand::SaveProject:
					save_project(editor, project, platform, current_project_hash);
					break;

				case EditorCommand::SaveProjectAs:
					save_project_as(editor, project, platform, current_project_hash);
					break;

				case EditorCommand::ResetGameState:
					game->counter = 0;
					game->time_ms = 0;
					break;

				case EditorCommand::RunGame:
					platform->set_run_mode(platform::RunMode::Game);
					break;

				case EditorCommand::Quit:
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, current_project_hash, [=]() {
							quit_editor(platform);
						});
					}
					else {
						quit_editor(platform);
					}
					break;
			}
		}
	}

} // namespace engine

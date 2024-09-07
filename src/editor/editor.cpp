#include <editor/editor.h>

#include <core/container.h>
#include <core/future.h>
#include <editor/editor_command.h>
#include <engine/engine.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <platform/debug/logging.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>

namespace editor {

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

	static void new_project(
		Editor* editor,
		engine::Systems* systems,
		engine::GameState* game,
		engine::ProjectState* project
	) {
		LOG_INFO("Opened new project");
		*project = {};
		*game = {};
		init_editor(editor, systems, *project, false);
	}

	static void open_project(
		Editor* editor,
		engine::GameState* game,
		engine::ProjectState* project,
		platform::PlatformAPI* platform
	) {
		platform->load_file_with_dialog(g_load_project_dialog, [=](std::vector<uint8_t> data, std::filesystem::path path) {
			*project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
				ABORT("Could not parse json file \"%s\": %s", path.string().c_str(), error.c_str());
			});
			editor->ui.project_name_buf = project->name;
			editor->ui.cached_project_hash = std::hash<engine::ProjectState>()(*project);
			editor->game_is_running = false;
			init_game_state(game, *project);
			LOG_INFO("Opened project \"%s\"", project->name.c_str());
		});
	}

	static void save_project_as(
		Editor* editor,
		engine::ProjectState* project,
		platform::PlatformAPI* platform,
		std::function<void()> on_file_saved = []() {}
	) {
		const std::string json = engine::ProjectState::to_json_string(*project);
		const std::vector<uint8_t> bytes = std::vector<uint8_t>(json.begin(), json.end());
		platform->save_file_with_dialog(bytes, g_save_project_dialog, [=](std::filesystem::path path) {
			LOG_INFO("Saved project \"%s\"", project->name.c_str());
			project->path = path;
			editor->ui.cached_project_hash = std::hash<engine::ProjectState>()(*project);
			on_file_saved();
		});
	}

	static void save_project(
		Editor* editor,
		engine::ProjectState* project,
		platform::PlatformAPI* platform,
		std::function<void()> on_file_saved = []() {}
	) {
		const bool project_file_exists = !project->path.empty() && std::filesystem::is_regular_file(project->path);
		if (project_file_exists) {
			/* Save existing file */
			const std::string json = engine::ProjectState::to_json_string(*project);
			const std::vector<uint8_t> bytes = std::vector<uint8_t>(json.begin(), json.end());
			platform->save_file(bytes, project->path, [=]() {
				LOG_INFO("Saved project \"%s\"", project->name.c_str());
				editor->ui.cached_project_hash = std::hash<engine::ProjectState>()(*project);
				on_file_saved();
			});
		}
		else {
			/* Save new file */
			save_project_as(editor, project, platform, on_file_saved);
		}
	}

	static void show_unsaved_project_changes_dialog(
		Editor* editor,
		engine::ProjectState* project,
		platform::PlatformAPI* platform,
		std::function<void()> on_dialog_not_cancelled = []() {}
	) {
		platform->show_unsaved_changes_dialog(project->name, [=](platform::UnsavedChangesDialogChoice choice) {
			switch (choice) {
				case platform::UnsavedChangesDialogChoice::Save:
					save_project(editor, project, platform, on_dialog_not_cancelled);
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

	void init_editor(Editor* editor, engine::Systems* systems, const engine::ProjectState& project, bool reset_docking) {
		init_editor_ui(&editor->ui, &systems->text, project, reset_docking);
	}

	void update_editor(
		Editor* editor,
		engine::GameState* game,
		engine::ProjectState* project,
		engine::Systems* systems,
		engine::SceneGraph* scene_graph,
		const platform::Input& input,
		platform::PlatformAPI* platform
	) {
		const size_t current_project_hash = std::hash<engine::ProjectState>()(*project);
		const bool is_new_file = project->path.empty();
		editor->project_has_unsaved_changes = editor->ui.cached_project_hash != current_project_hash;

		/* Run UI */
		std::vector<EditorCommand> commands = update_editor_ui(
			&editor->ui,
			game,
			project,
			systems,
			scene_graph,
			input,
			editor->project_has_unsaved_changes,
			editor->game_is_running
		);

		/* Project keyboard shortcuts */
		{
			if (input.keyboard.key_pressed_now_with_modifier(SDLK_n, platform::KEY_MOD_CTRL)) {
				commands.push_back(EditorCommand::NewProject);
			}

			if (input.keyboard.key_pressed_now_with_modifier(SDLK_o, platform::KEY_MOD_CTRL)) {
				commands.push_back(EditorCommand::OpenProject);
			}

			if (input.keyboard.key_pressed_now_with_modifier(SDLK_s, platform::KEY_MOD_CTRL)) {
				commands.push_back(EditorCommand::SaveProject);
			}

			if (input.keyboard.key_pressed_now_with_modifier(SDLK_s, platform::KEY_MOD_CTRL | platform::KEY_MOD_SHIFT)) {
				commands.push_back(EditorCommand::SaveProjectAs);
			}
		}

		/* Run game keyboard shortcuts*/
		{
			/* Run */
			if (input.keyboard.key_pressed_now(SDLK_F5)) {
				if (editor->game_is_running) {
					commands.push_back(EditorCommand::RunGame);
				}
				else {
					commands.push_back(EditorCommand::ResetGameState);
					commands.push_back(EditorCommand::RunGame);
				}
			}

			/* Restart */
			if (input.keyboard.key_pressed_now_with_modifier(SDLK_F5, platform::KEY_MOD_CTRL | platform::KEY_MOD_SHIFT)) {
				commands.push_back(EditorCommand::ResetGameState);
				commands.push_back(EditorCommand::RunGame);
			}
		}

		/* Process commands */
		for (const EditorCommand& cmd : commands) {
			switch (cmd) {
				case EditorCommand::NewProject:
					if (editor->project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, [=]() {
							new_project(editor, systems, game, project);
						});
					}
					else {
						new_project(editor, systems, game, project);
					}
					break;

				case EditorCommand::OpenProject:
					if (editor->project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, [=]() {
							open_project(editor, game, project, platform);
						});
					}
					else {
						open_project(editor, game, project, platform);
					}
					break;

				case EditorCommand::SaveProject:
					if (editor->project_has_unsaved_changes || is_new_file) {
						save_project(editor, project, platform);
					}
					break;

				case EditorCommand::SaveProjectAs:
					save_project_as(editor, project, platform);
					break;

				case EditorCommand::SetCursorToSizeAll:
					platform->set_cursor(platform::Cursor::SizeAll);
					break;

				case EditorCommand::SetCursorToArrow:
					platform->set_cursor(platform::Cursor::Arrow);
					break;

				case EditorCommand::ResetGameState:
					editor->game_is_running = false;
					init_game_state(game, *project);
					break;

				case EditorCommand::RunGame:
					editor->game_is_running = true;
					platform->set_run_mode(platform::RunMode::Game);
					platform->set_window_mode(editor->ui.run_game_windowed ? platform::WindowMode::Windowed : platform::WindowMode::FullScreen);
					break;

				case EditorCommand::ClearLog:
					platform->clear_log();
					break;

				case EditorCommand::Quit:
					if (editor->project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(editor, project, platform, [=]() {
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

	void render_editor(
		const Editor& editor,
		const engine::Systems& systems,
		platform::Renderer* renderer
	) {
		render_editor_ui(editor.ui, systems, renderer);
	}

} // namespace editor

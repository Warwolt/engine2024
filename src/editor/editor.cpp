#include <editor/editor.h>

#include <core/container.h>
#include <core/future.h>
#include <editor/editor_command.h>
#include <engine/engine.h>
#include <engine/state/project_state.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>

namespace editor {

	static const platform::FileExplorerDialog LOAD_PROJECT_DIALOG = {
		.title = "Load project",
		.description = "(PAK *.pak)",
		.extension = "pak",
	};

	static const platform::FileExplorerDialog SAVE_PROJECT_DIALOG = {
		.title = "Save project",
		.description = "PAK (*.pak)",
		.extension = "pak",
	};

	static void new_project(
		Editor* editor,
		engine::Engine* engine,
		const platform::Configuration& config
	) {
		LOG_INFO("Opened new project");
		engine->systems().reset();
		*editor = Editor(engine, config);
	}

	static void open_project(
		Editor* /*editor*/,
		engine::Engine* /*engine*/,
		platform::PlatformAPI* platform
	) {
		// TODO: re-implement load project so that it works with SceneGraph stuff
		platform->load_file_with_dialog(LOAD_PROJECT_DIALOG, [=](std::vector<uint8_t> /*data*/, std::filesystem::path /*path*/) {
			LOG_INFO("Opened project is not implemented!");
		});
	}

	static void save_project_as(
		Editor* /*editor*/,
		engine::ProjectState* /*project*/,
		platform::PlatformAPI* platform,
		std::function<void()> on_file_saved = []() {}
	) {
		const std::vector<uint8_t> bytes;
		platform->save_file_with_dialog(bytes, SAVE_PROJECT_DIALOG, [=](std::filesystem::path path) {
			LOG_ERROR("save_project_as is unimplemented!");
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
			const std::vector<uint8_t> bytes;
			platform->save_file(bytes, project->path, [=]() {
				LOG_ERROR("save_project is unimplemented!");
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

	Editor::Editor(
		engine::Engine* engine,
		const platform::Configuration& config
	) {
		const bool reset_docking = !config.window.docking_initialized;
		init_editor_ui(&m_ui, &engine->systems().text, engine->project(), reset_docking);
	}

	void Editor::update(
		const platform::Input& input,
		const platform::Configuration& config,
		engine::Engine* engine,
		platform::PlatformAPI* platform
	) {
		const size_t current_project_hash = std::hash<engine::ProjectState>()(engine->project());
		const bool is_new_file = engine->project().path.empty();
		const bool game_is_running = input.mode == platform::RunMode::Game;
		const bool project_has_unsaved_changes = m_ui.cached_project_hash != current_project_hash;

		/* Run UI */
		std::vector<EditorCommand> commands;
		if (!game_is_running) {
			commands = update_editor_ui(
				&m_ui,
				engine,
				input,
				project_has_unsaved_changes
			);
		}

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
				if (game_is_running) {
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
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(this, &engine->project(), platform, [=]() {
							new_project(this, engine, config);
						});
					}
					else {
						new_project(this, engine, config);
					}
					break;

				case EditorCommand::OpenProject:
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(this, &engine->project(), platform, [=]() {
							open_project(this, engine, platform);
						});
					}
					else {
						open_project(this, engine, platform);
					}
					break;

				case EditorCommand::SaveProject:
					if (project_has_unsaved_changes || is_new_file) {
						save_project(this, &engine->project(), platform);
					}
					break;

				case EditorCommand::SaveProjectAs:
					save_project_as(this, &engine->project(), platform);
					break;

				case EditorCommand::SetCursorToSizeAll:
					platform->set_cursor(platform::Cursor::SizeAll);
					break;

				case EditorCommand::SetCursorToArrow:
					platform->set_cursor(platform::Cursor::Arrow);
					break;

				case EditorCommand::ResetGameState:
					engine->systems().reset();
					break;

				case EditorCommand::RunGame:
					platform->set_run_mode(platform::RunMode::Game);
					platform->set_window_mode(m_ui.run_game_windowed ? platform::WindowMode::Windowed : platform::WindowMode::FullScreen);
					break;

				case EditorCommand::ClearLog:
					platform->clear_log();
					break;

				case EditorCommand::Quit:
					if (project_has_unsaved_changes) {
						show_unsaved_project_changes_dialog(this, &engine->project(), platform, [=]() {
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

	void Editor::render(const engine::Engine& engine, platform::Renderer* renderer) const {
		render_editor_ui(m_ui, engine, renderer);
	}

} // namespace editor

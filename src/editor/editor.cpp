#include <editor/editor.h>

#include <core/future.h>
#include <core/unwrap.h>
#include <editor/editor_command.h>
#include <editor/ui/main_menu_bar.h>
#include <engine/engine.h>
#include <engine/state/project_state.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <platform/debug/logging.h>
#include <platform/file/config.h>
#include <platform/input/input.h>
#include <platform/os/imwin32.h>
#include <platform/platform_api.h>

#include <imgui/imgui.h>

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

	constexpr char LOG_WINDOW[] = "Log";
	constexpr char SCENE_WINDOW[] = "Scene";
	constexpr char SCENE_GRAPH_WINDOW[] = "Scene Graph";
	constexpr char GAME_WINDOW[] = "Game";

	static void setup_docking_space(ImGuiID dockspace) {
		/* Create docks */
		ImGui::DockBuilderAddNode(dockspace); // Create a new dock node to use
		ImGui::DockBuilderSetNodeSize(dockspace, ImVec2 { 1, 1 });

		// +---------------+
		// |               |
		// |               |
		// |       1       |
		// |               |
		// |               |
		// +---------------+
		ImGuiID dock1 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.25f, nullptr, &dockspace);

		// // +---------------+
		// // |               |
		// // |       2       |   ^
		// // |               |   |
		// // +---------------+ split
		// // |       1       |
		// // +---------------+
		ImGuiID dock2 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.75f, nullptr, &dockspace);

		// //    <- split
		// // +-----+---------+
		// // |     |         |
		// // |  3  |    2    |
		// // |     |         |
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock3 = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.5f, nullptr, &dockspace);

		// // +-----+---------+
		// // |  3  |         |  split
		// // +-----+    2    |    |
		// // |  4  |         |    V
		// // +---------------+
		// // |       1       |
		// // +---------------+
		ImGuiID dock4 = ImGui::DockBuilderSplitNode(dock3, ImGuiDir_Down, 0.5f, nullptr, &dock3);

		/* Add windows to docks */
		ImGui::DockBuilderDockWindow(LOG_WINDOW, dock1);
		ImGui::DockBuilderDockWindow(SCENE_WINDOW, dock2);
		ImGui::DockBuilderDockWindow(SCENE_GRAPH_WINDOW, dock3);
		ImGui::DockBuilderDockWindow(GAME_WINDOW, dock4);
		ImGui::DockBuilderFinish(dockspace);
	}

	static void new_project(
		Editor* editor,
		engine::Engine* engine,
		platform::OpenGLContext* gl_context,
		const platform::Configuration& config
	) {
		LOG_INFO("Opened new project");
		engine->systems().reset();
		*editor = Editor(engine, gl_context, config);
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

	static void update_edit_window(
		bool game_is_running,
		std::vector<EditorCommand>* commands
	) {
		if (game_is_running) {
			if (ImGui::Button("Resume game")) {
				commands->push_back(editor::EditorCommand::RunGame);
			}
			if (ImGui::Button("Stop game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
			}
			if (ImGui::Button("Restart game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
				commands->push_back(editor::EditorCommand::RunGame);
			}
		}
		else {
			if (ImGui::Button("Run game")) {
				commands->push_back(editor::EditorCommand::ResetGameState);
				commands->push_back(editor::EditorCommand::RunGame);
			}
		}
	}

	static engine::FontID add_font(platform::OpenGLContext* gl_context, engine::TextSystem* text_system, const char* path, uint8_t font_size) {
		return core::unwrap(text_system->add_font(gl_context, path, font_size), [&](std::string error) {
			ABORT("Failed to load font \"%s\": %s", path, error.c_str());
		});
	}

	Editor::Editor(
		engine::Engine* engine,
		platform::OpenGLContext* gl_context,
		const platform::Configuration& config
	)
		: m_scene_window(gl_context) {
		m_project_hash = std::hash<engine::ProjectState>()(engine->project());
		m_system_font_id = add_font(gl_context, &engine->systems().text, "C:/windows/Fonts/tahoma.ttf", 13);

		/* Setup docking */
		if (!config.window.docking_initialized) {
			ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
			setup_docking_space(dockspace);
		}
	}

	void Editor::update(
		const platform::Input& input,
		const platform::Configuration& config,
		engine::Engine* engine,
		platform::PlatformAPI* platform,
		platform::OpenGLContext* gl_context
	) {
		const size_t current_project_hash = std::hash<engine::ProjectState>()(engine->project());
		const bool is_new_file = engine->project().path.empty();
		const bool game_is_running = input.mode == platform::RunMode::Game;
		const bool unsaved_changes = m_project_hash != current_project_hash;

		/* Run UI */
		std::vector<EditorCommand> commands;
		if (!game_is_running) {
			commands = _update_ui(
				input,
				gl_context,
				unsaved_changes,
				engine
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
					if (unsaved_changes) {
						show_unsaved_project_changes_dialog(this, &engine->project(), platform, [=]() {
							new_project(this, engine, gl_context, config);
						});
					}
					else {
						new_project(this, engine, gl_context, config);
					}
					break;

				case EditorCommand::OpenProject:
					if (unsaved_changes) {
						show_unsaved_project_changes_dialog(this, &engine->project(), platform, [=]() {
							open_project(this, engine, platform);
						});
					}
					else {
						open_project(this, engine, platform);
					}
					break;

				case EditorCommand::SaveProject:
					if (unsaved_changes || is_new_file) {
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
					platform->set_window_mode(platform::WindowMode::FullScreen);
					break;

				case EditorCommand::ClearLog:
					platform->clear_log();
					break;

				case EditorCommand::Quit:
					if (unsaved_changes) {
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

	std::vector<editor::EditorCommand> Editor::_update_ui(
		const platform::Input& input,
		platform::OpenGLContext* gl_context,
		bool unsaved_changes,
		engine::Engine* engine
	) {
		std::vector<editor::EditorCommand> commands;
		const bool game_is_running = input.mode == platform::RunMode::Game;

		/* Quit */
		if (input.quit_signal_received || input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands.push_back(editor::EditorCommand::Quit);
		}

		/* Dockspace */
		ImGuiID dockspace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton);

		/* Editor Menu Bar*/
		bool reset_window_layout = false;
		if (ImWin32::BeginMainMenuBar()) {
			update_main_menu_bar(unsaved_changes, game_is_running, &commands, &reset_window_layout, &m_show_imgui_demo);
			if (reset_window_layout) {
				setup_docking_space(dockspace);
			}
		}
		ImWin32::EndMainMenuBar();

		/* ImGui Demo */
		if (m_show_imgui_demo) {
			ImGui::ShowDemoWindow(&m_show_imgui_demo);
		}

		/* Log Window */
		if (ImGui::Begin(LOG_WINDOW)) {
			if (input.log) {
				m_log_window.update(*input.log, &commands);
			}
			else {
				ImGui::Text("Log not available!");
			}
		}
		ImGui::End();

		/* Project Window */
		if (ImGui::Begin(SCENE_GRAPH_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			m_scene_graph_window.update(&engine->systems(), &engine->scene_graph(), m_system_font_id);
		}
		ImGui::End();

		/* Game Edit Window */
		if (ImGui::Begin(GAME_WINDOW, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
			update_edit_window(game_is_running, &commands);
		}
		ImGui::End();

		/* Scene Window */
		if (ImGui::Begin(SCENE_WINDOW)) {
			m_scene_window.update(&engine->scene_graph(), gl_context, input, &commands);
		}
		ImGui::End();

		return commands;
	}

	void Editor::render(const engine::Engine& engine, platform::OpenGLContext* gl_context, platform::Renderer* renderer) const {
		m_scene_window.render(gl_context, engine.systems().text, m_system_font_id, renderer);
	}

} // namespace editor

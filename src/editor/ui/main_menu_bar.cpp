#include <editor/ui/main_menu_bar.h>

#include <platform/os/imwin32.h>

namespace editor {

	void update_main_menu_bar(
		bool unsaved_changes,
		bool game_is_running,
		std::vector<EditorCommand>* commands,
		bool* reset_window_layout,
		bool* show_imgui_demo
	) {
		if (ImWin32::BeginMainMenuBar()) {
			if (ImWin32::BeginMenu(L"&File")) {
				if (ImWin32::MenuItem(L"&New Project\tCtrl+N")) {
					commands->push_back(editor::EditorCommand::NewProject);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Open Project\tCtrl+O")) {
					commands->push_back(editor::EditorCommand::OpenProject);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Save Project\tCtrl+S", unsaved_changes)) {
					commands->push_back(editor::EditorCommand::SaveProject);
				}

				if (ImWin32::MenuItem(L"Save Project &As\tCtrl+Shift+S")) {
					commands->push_back(editor::EditorCommand::SaveProjectAs);
				}

				ImWin32::Separator();

				if (ImWin32::MenuItem(L"&Quit\tCtrl+Q")) {
					commands->push_back(editor::EditorCommand::Quit);
				}

				ImWin32::EndMenu();
			}

			if (ImWin32::BeginMenu(L"&Run")) {
				if (game_is_running) {
					if (ImWin32::MenuItem(L"Resume game")) {
						commands->push_back(editor::EditorCommand::RunGame);
					}
					if (ImWin32::MenuItem(L"Stop game")) {
						commands->push_back(editor::EditorCommand::ResetGameState);
					}
					if (ImWin32::MenuItem(L"Restart game")) {
						commands->push_back(editor::EditorCommand::ResetGameState);
						commands->push_back(editor::EditorCommand::RunGame);
					}
				}
				else {
					if (ImWin32::MenuItem(L"Run game")) {
						commands->push_back(editor::EditorCommand::ResetGameState);
						commands->push_back(editor::EditorCommand::RunGame);
					}
				}

				if (ImWin32::MenuItem(L"Show ImGui Demo")) {
					*show_imgui_demo = true;
				}

				ImWin32::EndMenu();
			}

			if (ImWin32::BeginMenu(L"&Window")) {
				if (ImWin32::MenuItem(L"Reset Window Layout")) {
					*reset_window_layout = true;
				}
			}
		}
		ImWin32::EndMainMenuBar();
	}

} // namespace editor

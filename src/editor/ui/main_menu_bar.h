#pragma once

#include <editor/editor_command.h>

#include <vector>

namespace editor {

	void update_main_menu_bar(
		bool unsaved_changes,
		bool game_is_running,
		std::vector<EditorCommand>* commands,
		bool* reset_window_layout,
		bool* show_imgui_demo
	);

} // namespace editor

#pragma once

#include <engine/editor/editor_command.h>

#include <stdint.h>
#include <string>
#include <vector>

namespace platform {

	struct Input;

} // namespace platform

namespace engine {

	struct GameState;
	struct ProjectState;
	struct Resources;

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
		bool run_game_windowed = false;
		bool show_imgui_demo = false;
	};

	void init_editor_ui(
		EditorUiState* ui,
		const ProjectState& project,
		bool reset_docking
	);

	std::vector<EditorCommand> update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		const engine::Resources& resources,
		bool unsaved_changes,
		bool game_is_running
	);

} // namespace engine

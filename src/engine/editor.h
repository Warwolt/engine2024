#pragma once

#include <platform/platform_api.h>

#include <expected>
#include <filesystem>
#include <future>
#include <stdint.h>
#include <vector>

namespace platform {
	struct Input;
}

namespace engine {

	struct GameState;
	struct ProjectState;

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
	};

	struct EditorState {
		EditorUiState ui;
		bool game_is_running = false;
	};

	void init_editor(EditorState* editor, const ProjectState& project);

	void update_editor(
		EditorState* editor,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		platform::PlatformAPI* platform
	);

} // namespace engine

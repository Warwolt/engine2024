#pragma once

#include <future>
#include <stdint.h>
#include <vector>

namespace platform {
	struct Input;
	class PlatformAPI;
}

namespace engine {

	struct GameState;
	struct ProjectState;

	struct EditorInput {
		std::future<std::vector<uint8_t>> project_data;
	};

	struct EditorUiState {
		size_t loaded_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
	};

	struct EditorState {
		EditorInput input;
		EditorUiState ui;
	};

	void init_editor(EditorState* editor, const ProjectState* project);

	void update_editor(
		EditorState* editor,
		GameState* game,
		ProjectState* project,
		const platform::Input* input,
		platform::PlatformAPI* platform
	);

} // namespace engine

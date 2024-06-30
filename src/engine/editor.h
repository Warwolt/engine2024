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

	struct EditorInput {
		struct Futures {
			std::future<std::vector<uint8_t>> project_data;
			std::future<std::expected<void, platform::SaveFileError>> save_project_result;
			std::future<std::filesystem::path> saved_project_path;
		} futures;
	};

	struct EditorUiState {
		size_t saved_project_hash; // for "unsaved changes" prompts
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

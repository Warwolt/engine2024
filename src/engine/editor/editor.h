#pragma once

#include <engine/editor/editor_ui.h>
#include <platform/platform_api.h>

namespace platform {
	struct Input;
}

namespace engine {

	struct GameState;
	struct ProjectState;

	struct EditorState {
		EditorUiState ui;
		bool project_has_unsaved_changes = false;
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

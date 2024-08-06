#pragma once

#include <engine/editor/editor_ui.h>
#include <platform/platform_api.h>

namespace platform {
	struct Input;
	class Renderer;
}

namespace engine {

	struct GameState;
	struct ProjectState;
	struct Resources;

	struct EditorState {
		EditorUiState ui;
		bool project_has_unsaved_changes = false;
		bool game_is_running = false;
	};

	void init_editor(EditorState* editor, const ProjectState& project, bool reset_docking);

	void update_editor(
		EditorState* editor,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		const engine::Resources& resources,
		platform::PlatformAPI* platform
	);

	void render_editor(
		const EditorState& editor,
		const engine::Resources& resources,
		platform::Renderer* renderer
	);

} // namespace engine

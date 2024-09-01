#pragma once

#include <editor/ui/editor_ui.h>
#include <platform/platform_api.h>

namespace platform {
	struct Input;
	class Renderer;
}

namespace engine {

	struct GameState;
	struct ProjectState;
	struct Resources;
	struct Systems;

}

namespace editor {

	struct EditorState {
		EditorUiState ui;
		bool project_has_unsaved_changes = false;
		bool game_is_running = false;
	};

	void init_editor(
		EditorState* editor,
		engine::Systems* systems,
		const engine::ProjectState& project,
		bool reset_docking
	);

	void update_editor(
		EditorState* editor,
		engine::GameState* game,
		engine::ProjectState* project,
		engine::Systems* systems,
		engine::SceneGraph* scene_graph,
		const platform::Input& input,
		const engine::Resources& resources,
		platform::PlatformAPI* platform
	);

	void render_editor(
		const EditorState& editor,
		const engine::Systems& systems,
		platform::Renderer* renderer
	);

} // namespace editor

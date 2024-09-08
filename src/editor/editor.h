#pragma once

#include <editor/ui/editor_ui.h>
#include <platform/platform_api.h>

namespace platform {
	class Renderer;
	struct Configuration;
	struct Input;
}

namespace engine {
	class Engine;
	struct GameState;
	struct ProjectState;
	struct Systems;
}

namespace editor {

	struct Editor {
		EditorUiState ui;
		bool project_has_unsaved_changes = false;
		bool game_is_running = false;
	};

	void init_editor(
		Editor* editor,
		engine::Engine* engine,
		const platform::Configuration& config
	);

	void update_editor(
		Editor* editor,
		const platform::Input& input,
		const platform::Configuration& config,
		engine::Engine* engine,
		platform::PlatformAPI* platform

	);

	void render_editor(
		const Editor& editor,
		const engine::Engine& engine,
		platform::Renderer* renderer
	);

} // namespace editor

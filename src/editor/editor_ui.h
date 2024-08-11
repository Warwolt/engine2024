#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <editor/ui/scene_view.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

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

}

namespace editor {

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
		bool run_game_windowed = false;
		bool show_imgui_demo = false;
		core::Signal<size_t> last_num_seen_log_entries = 0;

		platform::Canvas window_canvas; // used to render ImGui::Image
		bool scene_window_visible = false;

		SceneViewState scene_view;
	};

	void init_editor_ui(
		EditorUiState* ui,
		const engine::ProjectState& project,
		bool reset_docking
	);

	void shutdown_editor_ui(const EditorUiState& ui);

	std::vector<editor::EditorCommand> update_editor_ui(
		EditorUiState* ui,
		engine::GameState* game,
		engine::ProjectState* project,
		const platform::Input& input,
		const engine::Resources& resources,
		bool unsaved_changes,
		bool game_is_running
	);

	void render_editor_ui(
		const EditorUiState& ui,
		platform::Renderer* renderer
	);

} // namespace editor

#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <editor/ui/editor_fonts.h>
#include <editor/ui/scene_window.h>
#include <engine/state/scene_graph.h>
#include <engine/system/text_system.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace platform {
	struct Input;
} // namespace platform

namespace engine {

	struct GameState;
	struct ProjectState;
	struct Resources;
	struct Systems;

}

namespace editor {

	struct UiGraphNode {
		bool is_open = false;
	};

	struct SceneGraphUiState {
		engine::GraphNodeID selected_node = engine::GraphNodeID(0);
		engine::GraphNodeID next_id = engine::GraphNodeID(1);
		std::unordered_map<engine::GraphNodeID, UiGraphNode> nodes = { { engine::GraphNodeID(0), { .is_open = true } } };
	};

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
		bool run_game_windowed = false;
		bool show_imgui_demo = false;
		core::Signal<size_t> last_num_seen_log_entries = 0;

		EditorFonts editor_fonts;
		SceneWindowState scene_window;

		SceneGraphUiState scene_graph_ui;

		// TODO move tese to the engine
		engine::SceneGraph scene_graph;
	};

	void init_editor_ui(
		EditorUiState* ui,
		engine::TextSystem* text_system,
		const engine::ProjectState& project,
		bool reset_docking
	);

	void shutdown_editor_ui(const EditorUiState& ui);

	std::vector<editor::EditorCommand> update_editor_ui(
		EditorUiState* ui,
		engine::GameState* game,
		engine::ProjectState* project,
		engine::Systems* systems,
		const platform::Input& input,
		const engine::Resources& resources,
		bool unsaved_changes,
		bool game_is_running
	);

	void render_editor_ui(
		const EditorUiState& ui,
		const engine::Systems& text_system,
		platform::Renderer* renderer
	);

} // namespace editor

#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <editor/ui/editor_fonts.h>
#include <editor/ui/scene_window.h>
#include <engine/state/scene_graph.h>
#include <engine/system/text_system.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

#include <set>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace platform {
	struct Input;
}

namespace engine {
	struct GameState;
	struct ProjectState;
	struct Systems;
	class Engine;
}

namespace editor {

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
		bool run_game_windowed = false;
		bool show_imgui_demo = false;
		core::Signal<size_t> last_num_seen_log_entries = 0;

		EditorFonts editor_fonts;
		SceneWindowState scene_window;

		std::set<engine::GraphNodeID> selected_nodes = { engine::GraphNodeID(0) };
		std::set<engine::GraphNodeID> open_nodes;
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
		engine::Engine* engine,
		const platform::Input& input,
		bool unsaved_changes
	);

	void render_editor_ui(
		const EditorUiState& ui,
		const engine::Engine& engine,
		platform::Renderer* renderer
	);

} // namespace editor

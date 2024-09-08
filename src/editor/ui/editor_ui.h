#pragma once

#include <core/signal.h>
#include <editor/editor_command.h>
#include <editor/ui/log_window.h>
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

	struct SceneGraphWindow {
		std::unordered_map<engine::GraphNodeID, bool> node_is_selected = { { engine::GraphNodeID(0), true } };
		std::unordered_map<engine::GraphNodeID, bool> node_is_open;
	};

	struct EditorUiState {
		bool show_imgui_demo = false;
		engine::FontID system_font_id;

		// components
		LogWindow log_window;
		SceneGraphWindow scene_graph_window;
		SceneWindowState scene_window;
	};

	void init_editor_ui(
		EditorUiState* ui,
		engine::TextSystem* text_system,
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

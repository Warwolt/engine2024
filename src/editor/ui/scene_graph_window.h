#pragma once

#include <engine/state/scene_graph.h>
#include <platform/graphics/font.h>

#include <unordered_map>

namespace engine {
	class SceneGraph;
	struct Systems;
}

namespace editor {

	struct SceneGraphWindow {
		std::unordered_map<engine::GraphNodeID, bool> node_is_selected = { { engine::GraphNodeID(0), true } };
		std::unordered_map<engine::GraphNodeID, bool> node_is_open;
	};

	void update_scene_graph_window(
		SceneGraphWindow* view,
		engine::Systems* systems,
		engine::SceneGraph* scene_graph,
		engine::FontID system_font_id
	);

} // namespace editor

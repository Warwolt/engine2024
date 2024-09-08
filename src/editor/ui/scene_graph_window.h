#pragma once

#include <engine/state/scene_graph.h>
#include <platform/graphics/font.h>

#include <unordered_map>

namespace engine {
	class SceneGraph;
	struct Systems;
}

namespace editor {

	class SceneGraphWindow {
	public:
		void update(
			engine::Systems* systems,
			engine::SceneGraph* scene_graph,
			engine::FontID system_font_id
		);

	private:
		void _render_scene_graph(const engine::SceneGraph& scene_graph);
		void _render_scene_graph_sub_tree(const kpeeters::tree<engine::GraphNode>::tree_node* node_it);

		std::unordered_map<engine::GraphNodeID, bool> m_node_is_selected = { { engine::GraphNodeID(0), true } };
		std::unordered_map<engine::GraphNodeID, bool> m_node_is_open;
	};

} // namespace editor

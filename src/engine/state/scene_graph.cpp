#include <engine/state/scene_graph.h>

namespace engine {

	const kpeeters::tree<GraphNode>& SceneGraph::tree() const {
		return m_tree;
	}

	kpeeters::tree<GraphNode>::iterator SceneGraph::root() const {
		return m_tree.begin();
	}

	GraphNodeId SceneGraph::add_text_node(kpeeters::tree<GraphNode>::iterator position, TextNode text_node) {
		GraphNodeId node_id = GraphNodeId(m_next_id++);
		m_tree.append_child(position, GraphNode { .id = node_id, .type = GraphNodeType::Text });
		m_text_nodes.push_back(text_node);
		return node_id;
	}

} // namespace engine

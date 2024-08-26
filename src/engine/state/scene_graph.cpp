#include <engine/state/scene_graph.h>

#include <platform/debug/logging.h>

namespace engine {

	const kpeeters::tree<GraphNode>& SceneGraph::tree() const {
		return m_tree;
	}

	kpeeters::tree<GraphNode>::iterator SceneGraph::root() const {
		return m_tree.begin();
	}

	const std::vector<TextNode>& SceneGraph::text_nodes() const {
		return m_text_nodes;
	}

	GraphNodeId SceneGraph::add_text_node(kpeeters::tree<GraphNode>::iterator position, TextNode text_node) {
		GraphNodeId node_id = GraphNodeId(m_next_id++);
		m_tree.append_child(position, GraphNode { .id = node_id, .type = GraphNodeType::Text });

		// NOTE: doing this until we've moved to the associative vector ("vector_map") we need
		text_node.id = node_id;

		m_text_nodes.push_back(text_node);
		return node_id;
	}

	SceneGraph::Tree::iterator SceneGraph::remove_node(Tree::iterator node) {
		if (node == this->root()) {
			LOG_ERROR("Cannot remove the root node of the scene graph!");
			return m_tree.end();
		}

		// Erase sub tree nodes
		for (auto sub_node = Tree::begin(node); sub_node != Tree::end(node); sub_node++) {
			_remove_node_from_vector(sub_node);
		}
		_remove_node_from_vector(node);
		return m_tree.erase(node);
	}

	void SceneGraph::_remove_node_from_vector(Tree::iterator node) {
		switch (node->type) {
			case GraphNodeType::Root:
				// root cannot be erased
				break;

			case GraphNodeType::Text:
				std::erase_if(m_text_nodes, [&](const TextNode& visited_node) { return visited_node.id == node->id; });
				break;
		}
	}

} // namespace engine

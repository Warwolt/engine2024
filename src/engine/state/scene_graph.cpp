#include <engine/state/scene_graph.h>

#include <platform/debug/logging.h>

namespace engine {

	// Tries to pick a node with the same index and depth as the node to be removed.
	// When failing, tries to stay on the same depth, else picks parent.
	static SceneGraph::Tree::iterator get_post_remove_node(SceneGraph::Tree::iterator node) {
		if (node.node->next_sibling) {
			return node.node->next_sibling;
		}
		else if (node.node->prev_sibling) {
			return node.node->prev_sibling;
		}
		else {
			return node.node->parent;
		}
	}

	SceneGraph::SceneGraph() {
		GraphNode root_node = {
			.id = GraphNodeId(0),
			.type = GraphNodeType::Root,
		};
		m_tree = kpeeters::tree<GraphNode>(root_node);
	}

	const kpeeters::tree<GraphNode>&
	SceneGraph::tree() const {
		return m_tree;
	}

	kpeeters::tree<GraphNode>::iterator SceneGraph::root() const {
		return m_tree.begin();
	}

	const std::vector<TextNode>& SceneGraph::text_nodes() const {
		return m_text_nodes.data();
	}

	GraphNodeId SceneGraph::add_text_node(kpeeters::tree<GraphNode>::iterator position, TextNode text_node) {
		GraphNodeId node_id = GraphNodeId(m_next_id++);
		m_tree.append_child(position, GraphNode { .id = node_id, .type = GraphNodeType::Text });
		m_text_nodes.insert({ node_id, text_node });
		return node_id;
	}

	SceneGraph::Tree::iterator SceneGraph::remove_node(Tree::iterator node) {
		if (node == this->root()) {
			LOG_ERROR("Cannot remove the root node of the scene graph!");
			return m_tree.end();
		}

		for (auto sub_node = Tree::begin(node); sub_node != Tree::end(node); sub_node++) {
			_remove_node_from_vector(sub_node);
		}
		_remove_node_from_vector(node);

		Tree::iterator next_node = get_post_remove_node(node);
		m_tree.erase(node);

		return next_node;
	}

	void SceneGraph::_remove_node_from_vector(Tree::iterator node) {
		switch (node->type) {
			case GraphNodeType::Root:
				// root cannot be erased
				break;

			case GraphNodeType::Text:
				m_text_nodes.erase(node->id);
				break;
		}
	}

} // namespace engine

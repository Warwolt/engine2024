#pragma once

#include <core/container/vec_map.h>
#include <core/newtype.h>

#include <glm/vec2.hpp>
#include <kpeeters/tree.hpp>

#include <vector>

namespace engine {
	struct GraphNodeId : public core::NewType<int> {
		GraphNodeId() = default;
		explicit GraphNodeId(const int& value)
			: NewType(value) {
		}
	};
}
DEFINE_NEWTYPE_HASH_IMPL(engine::GraphNodeId, int);

namespace engine {

	enum class GraphNodeType {
		Root,
		Text,
	};

	struct GraphNode {
		GraphNodeId id = GraphNodeId(0);
		GraphNodeType type = GraphNodeType::Root;
	};

	struct TextNode {
		glm::vec2 position = { 0.0f, 0.0f };
		std::string text;
	};

	class SceneGraph {
	public:
		using Tree = kpeeters::tree<GraphNode>;

		// FIX-ME: Doesn't link if this is in the .cpp file for some reason
		SceneGraph() {
			GraphNode root_node = {
				.id = GraphNodeId(0),
				.type = GraphNodeType::Root,
			};
			m_tree = kpeeters::tree<GraphNode>(root_node);
		}

		const Tree& tree() const;
		Tree::iterator root() const;
		const std::vector<TextNode>& text_nodes() const;

		GraphNodeId add_text_node(Tree::iterator position, TextNode text_node);
		Tree::iterator remove_node(Tree::iterator position);

	private:
		void _remove_node_from_vector(Tree::iterator node);

		int m_next_id = 1;
		kpeeters::tree<GraphNode> m_tree;
		core::VecMap<GraphNodeId, TextNode> m_text_nodes;
	};

} // namespace engine

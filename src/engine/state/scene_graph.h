#pragma once

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
		glm::vec2 position = { 0.0f, 0.0f };
	};

	struct TextNode {
		std::string value;
	};

	class SceneGraph {
	public:
		const kpeeters::tree<GraphNode>& tree() const;
		kpeeters::tree<GraphNode>::iterator root() const;
		GraphNodeId add_text_node(kpeeters::tree<GraphNode>::iterator position, TextNode text_node);

	private:
		int m_next_id = 1;
		kpeeters::tree<GraphNode> m_tree = { GraphNode { .id = GraphNodeId(0), .type = GraphNodeType::Root, .position = { 0.0f, 0.0f } } };
		std::vector<TextNode> m_text_nodes;
	};

} // namespace engine

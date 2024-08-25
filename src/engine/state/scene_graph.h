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

	class SceneGraph {
	public:
		const kpeeters::tree<GraphNode>& tree() const;
		kpeeters::tree<GraphNode> m_tree = { GraphNode { .id = GraphNodeId(0), .type = GraphNodeType::Root, .position = { 0.0f, 0.0f } } };
	private:
	};

} // namespace engine

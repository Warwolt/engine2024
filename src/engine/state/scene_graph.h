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

	enum class NodeType {
		Root,
		Text,
	};

	struct GraphNode {
		GraphNodeId id = GraphNodeId(0);
		NodeType type = NodeType::Root;
		glm::vec2 position = { 0.0f, 0.0f };
	};

	struct SceneGraph {
		kpeeters::tree<GraphNode> tree = { GraphNode { .id = GraphNodeId(0), .type = NodeType::Root, .position = { 0.0f, 0.0f } } };
	};

} // namespace engine

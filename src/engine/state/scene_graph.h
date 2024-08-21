#pragma once

#include <core/newtype.h>

#include <vector>

namespace engine {
	DEFINE_NEWTYPE(GraphNodeId, int);
}
DEFINE_NEWTYPE_HASH(engine::GraphNodeId, int);

namespace engine {

	enum class NodeType {
		Root,
		Text,
	};

	struct GraphNode {
		GraphNodeId id = GraphNodeId(0);
		NodeType type = NodeType::Root;
		std::vector<GraphNode> children;
	};

} // namespace engine

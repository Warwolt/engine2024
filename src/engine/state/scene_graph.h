#pragma once

#include <core/newtype.h>

#include <vector>

namespace engine {

	struct GraphNodeId : public core::NewType<size_t> {
		GraphNodeId() = default;
		explicit GraphNodeId(const size_t& value)
			: NewType(value) {
		}
	};

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

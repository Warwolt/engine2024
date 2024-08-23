#pragma once

#include <core/newtype.h>

#include <glm/vec2.hpp>

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

	// FUCK OH NO SHIT:
	// No matter what new member we add, if it's big enough we start causing a
	// bunch of weird crashes.
	//
	// This implies that we're _probably_ doing some kind of heap memory stomping
	// that invalidates other data on the heap. FUCK!
	//
	// We need propper tooling to know exactly what is fucking up the heap
	// memory, otherwise this is practially impossible to figure out. SHIT.
	struct GraphNode {
		GraphNodeId id = GraphNodeId(0);
		NodeType type = NodeType::Root;
		glm::vec2 position = { 0.0f, 0.0f };
		std::vector<GraphNode> children;
	};

} // namespace engine

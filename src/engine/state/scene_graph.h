#pragma once

#include <core/container/vec_map.h>
#include <core/newtype.h>
#include <engine/system/text_system.h>

#include <glm/vec2.hpp>
#include <kpeeters/tree.hpp>

#include <unordered_map>
#include <vector>
#include <optional>

namespace engine {
	DEFINE_NEWTYPE(GraphNodeID, int);
}
DEFINE_NEWTYPE_HASH_IMPL(engine::GraphNodeID, int);

namespace engine {

	enum class GraphNodeType {
		Root,
		Text,
	};

	struct GraphNode {
		static constexpr GraphNodeID INVALID_ID = GraphNodeID(-1);
		GraphNodeID id = INVALID_ID;
		GraphNodeType type = GraphNodeType::Root;
	};

	class SceneGraph {
	public:
		using Tree = kpeeters::tree<GraphNode>;

		SceneGraph();

		const Tree& tree() const;
		Tree::iterator root() const;

		GraphNodeID add_text_node(Tree::iterator position, TextID text_id);
		Tree::iterator remove_node(Tree::iterator position);

		std::optional<TextID> text_id(GraphNodeID node_id);

	private:
		void _remove_node(Tree::iterator node);

		int m_next_id = 1;
		kpeeters::tree<GraphNode> m_tree;
		std::unordered_map<GraphNodeID, TextID> m_text_ids;
	};

} // namespace engine

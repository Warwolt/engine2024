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
		bool is_selected = false;
	};

	class SceneGraph {
	public:
		using Tree = kpeeters::tree<GraphNode>;

		// FIX-ME:
		// Currently, main.cpp needs to be able to construct `engine::State`,
		// which SceneGraph ends up being a member of.
		//
		// Since main.cpp links against the engine as a DLL, we don't have
		// access to `SceneGraph::SceneGraph()` statically.
		//
		// The problem that needs fixing is making sure `main.cpp` can own the
		// engine state (so it persist between hot reloading) but allow
		// `engine::State` to contain members that run constructors.
		//
		// (Solution: Make the engine DLL return a heap allocated instance of
		// the initial engine state)
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

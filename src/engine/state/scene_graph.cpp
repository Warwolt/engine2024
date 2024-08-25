#include <engine/state/scene_graph.h>

namespace engine {

	const kpeeters::tree<GraphNode>& SceneGraph::tree() const {
		return m_tree;
	}

} // namespace engine

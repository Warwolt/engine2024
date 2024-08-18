#pragma once

namespace engine {

	// TODO: figure out if this should be its own struct or if we should just
	// create some kind of core::flat_tree data structure
	//
	// The SceneGraph will be the main data structure to represent the game
	// with, so we want it to be fast to iterate on (hence speculating on
	// flat_tree being a good thing to use).
	struct SceneGraph {
	};

} // namespace engine

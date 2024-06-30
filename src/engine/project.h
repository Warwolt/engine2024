#pragma once

#include <core/signal.h>

#include <string>

namespace engine {

	/**
	 * The meta data for the game project.
	 */
	struct ProjectState {
		core::Signal<std::string> name;
	};

} // namespace engine

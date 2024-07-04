#pragma once

#include <engine/project.h>

#include <stdint.h>

namespace engine {

	/**
	 * The run-time state of the game.
	 */
	struct GameState {
		int counter = 0;
		uint64_t time_ms = 0;
	};

	void init_game_state(GameState* game, const ProjectState& project);

} // namespace engine

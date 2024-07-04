#include <engine/state/game_state.h>

namespace engine {

	void init_game_state(GameState* game, const ProjectState& project) {
		game->counter = project.counter;
		game->time_ms = 0;
	}

} // namespace engine

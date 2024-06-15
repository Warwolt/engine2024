#pragma once

#include <stdint.h>

namespace engine {

	struct GameState {
		int counter = 0;
		uint64_t time_ms = 0;
	};

} // namespace engine

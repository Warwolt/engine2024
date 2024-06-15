#pragma once

#include <future>
#include <stdint.h>
#include <vector>

namespace platform {
	struct Input;
	class PlatformAPI;
}

namespace engine {

	struct GameState;

	struct EditorState {
		std::future<std::vector<uint8_t>> load_project_future;
	};

	void update_editor(EditorState* editor, GameState* game, const platform::Input* input, platform::PlatformAPI* platform);

} // namespace engine

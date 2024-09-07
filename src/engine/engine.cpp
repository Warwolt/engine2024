#include <engine/engine.h>

#include <platform/debug/assert.h>
#include <platform/debug/logging.h>

#include <filesystem>
#include <vector>

namespace engine {

	void Engine::load_project(const char* path) {
		// std::filesystem::path path = std::filesystem::path(path_str);
		// if (std::filesystem::is_regular_file(path)) {
		// 	std::vector<uint8_t> data = platform::read_file_bytes(path).value();
		// 	state->project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
		// 		ABORT("Could not parse json file \"%s\": %s", path_str, error.c_str());
		// 	});
		// 	init_game_state(&state->game, state->project);
		// 	LOG_INFO("Game data loaded from \"%s\"", path_str);
		// }
		// else {
		// 	LOG_ERROR("Could not load game data from path \"%s\"", path_str);
		// }
	}

} // namespace engine

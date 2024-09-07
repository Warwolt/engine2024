#include <engine/engine.h>

#include <core/container.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/file/file.h>

#include <filesystem>
#include <vector>

namespace engine {

	void Engine::load_project(const char* path_str) {
		std::filesystem::path path = std::filesystem::path(path_str);
		if (std::filesystem::is_regular_file(path)) {
			std::vector<uint8_t> data = platform::read_file_bytes(path).value();
			m_project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
				ABORT("Could not parse json file \"%s\": %s", path_str, error.c_str());
			});
			init_game_state(&m_game, m_project);
			LOG_INFO("Game data loaded from \"%s\"", path_str);
		}
		else {
			LOG_ERROR("Could not load game data from path \"%s\"", path_str);
		}
	}

} // namespace engine

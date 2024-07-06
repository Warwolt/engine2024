#pragma once

#include <glm/vec2.hpp>

#include <filesystem>
#include <optional>

namespace platform {

	struct Configuration {
		bool full_screen = false;
		glm::ivec2 window_pos = { 0, 0 };
		glm::ivec2 window_size = { 0, 0 };
	};

	std::optional<Configuration> load_configuration(const std::filesystem::path& path);
	void save_configuration(const Configuration& config, const std::filesystem::path& path);

} // namespace platform

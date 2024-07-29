#pragma once

#include <glm/vec2.hpp>

#include <filesystem>
#include <optional>

namespace platform {

	struct Configuration {
		struct {
			bool docking_initialized = false;
			bool fullscreen = false;
			bool maximized = false;
			glm::ivec2 position = { 0, 0 };
			glm::ivec2 size = { 0, 0 };
		} window;
	};

	std::optional<Configuration> load_configuration(const std::filesystem::path& path);
	void save_configuration(const Configuration& config, const std::filesystem::path& path);

} // namespace platform

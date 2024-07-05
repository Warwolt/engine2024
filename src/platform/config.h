#pragma once

#include <filesystem>
#include <optional>

namespace platform {

	struct Configuration {
		int counter = 0;
	};

	std::optional<Configuration> load_configuration(const std::filesystem::path& path);
	void save_configuration(const Configuration& config, const std::filesystem::path& path);

} // namespace platform

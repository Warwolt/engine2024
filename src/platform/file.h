#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace platform {

	std::optional<std::string> read_file_to_string(const std::filesystem::path& path);

} // namespace platform

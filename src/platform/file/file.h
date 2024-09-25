#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <stdint.h>

namespace platform {

	std::optional<std::string> read_file_to_string(const std::filesystem::path& path);
	std::optional<std::vector<uint8_t>> read_bytes_from_file(const std::filesystem::path& path);

} // namespace platform

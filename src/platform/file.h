#pragma once

#include <optional>
#include <string>

namespace platform {

	std::optional<std::string> read_file(const char* path);

} // namespace platform

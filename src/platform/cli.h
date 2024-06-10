#pragma once

#include <expected>
#include <stdint.h>
#include <string>

namespace platform {

	struct CliCommands {
	};

	std::expected<CliCommands, std::string> parse_arguments(size_t num_args, char** args);

} // namespace platform

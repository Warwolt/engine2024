#pragma once

#include <expected>
#include <string>

namespace platform {

	struct CliCommands {
		bool print_usage = false;
		bool run_game = false;
		std::string game_data_path;
	};

	std::string usage_string();
	std::expected<CliCommands, std::string> parse_arguments(int argc, char** argv);

} // namespace platform

#pragma once

#include <expected>
#include <string>

namespace platform {

	struct CommandLineArgs {
		bool print_usage = false;
		bool start_in_editor_mode = false;
		bool start_game_windowed = false;
	};

	std::string usage_string();
	std::expected<CommandLineArgs, std::string> parse_arguments(int argc, char** argv);

} // namespace platform

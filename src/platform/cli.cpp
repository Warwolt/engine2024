#include <platform/cli.h>

#include <string.h>

namespace platform {

	static bool string_equals(const char* str, const char* str2) {
		return std::string(str) == std::string(str2);
	}

	static bool string_starts_with(const char* str, const char* prefix) {
		return strncmp(str, prefix, strlen(prefix)) == 0;
	}

	std::string usage_string() {
		return "usage: GameEngine2024.exe [-h | --help] [--run=<path>]";
	}

	std::expected<CliCommands, std::string> parse_arguments(int argc, char** argv) {
		CliCommands cmds;

		for (int i = 1; i < argc; i++) {
			if (string_equals(argv[i], "-h") || string_equals(argv[i], "--help")) {
				cmds.print_usage = true;
			}
			else if (string_starts_with(argv[i], "--run=")) {
				cmds.run_game = true;
				cmds.game_data_path = std::string(argv[i] + 6);
			}
			else {
				return std::unexpected(std::string("Unexpected arg: ") + argv[i]);
			}
		}

		return cmds;
	}

} // namespace platform

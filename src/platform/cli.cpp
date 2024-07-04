#include <platform/cli.h>

#include <platform/win32.h>

#include <string.h>

namespace platform {

	// FIXME: move this to "core/string"

	static bool string_equals(const char* str, const char* str2) {
		return std::string(str) == std::string(str2);
	}

	static bool string_starts_with(const char* str, const char* prefix) {
		return strncmp(str, prefix, strlen(prefix)) == 0;
	}

	std::string usage_string() {
		return std::string("usage: ") + application_name() + "[-h | --help] [--editor]";
	}

	std::expected<CommandLineArgs, std::string> parse_arguments(int argc, char** argv) {
		CommandLineArgs cmds;

		for (int i = 1; i < argc; i++) {
			if (string_equals(argv[i], "-h") || string_equals(argv[i], "--help")) {
				cmds.print_usage = true;
			}
			else if (string_equals(argv[i], "--editor")) {
				cmds.start_in_editor_mode = true;
			}
			else {
				return std::unexpected(std::string("Unexpected arg: ") + argv[i]);
			}
		}

		return cmds;
	}

} // namespace platform

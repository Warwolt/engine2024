#include <platform/cli.h>

#include <core/string.h>
#include <platform/win32.h>

#include <string.h>

namespace platform {

	std::string usage_string() {
		return std::string("usage: ") + application_name() + "[-h | --help] [--editor]";
	}

	std::expected<CommandLineArgs, std::string> parse_arguments(int argc, char** argv) {
		CommandLineArgs cmds;

		for (int i = 1; i < argc; i++) {
			if (core::string::equals(argv[i], "-h") || core::string::equals(argv[i], "--help")) {
				cmds.print_usage = true;
			}
			else if (core::string::equals(argv[i], "--editor")) {
				cmds.start_in_editor_mode = true;
			}
			else {
				return std::unexpected(std::string("Unexpected arg: ") + argv[i]);
			}
		}

		return cmds;
	}

} // namespace platform

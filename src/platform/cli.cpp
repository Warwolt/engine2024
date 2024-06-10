#include <platform/cli.h>

namespace platform {

	std::expected<CliCommands, std::string> parse_arguments(size_t /*num_args*/, char** /*args*/) {
		return std::unexpected("Hello world");
	}

} // namespace platform

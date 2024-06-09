#pragma once

#include <expected>
#include <string>

namespace platform {

	using ExitCode = int;

	std::string get_win32_error();
	std::expected<ExitCode, std::string> run_command(const char* cmd_str);

} // namespace platform

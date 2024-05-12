#pragma once

#include <expected>
#include <string>

namespace platform {

	std::string get_win32_error();
	std::expected<void, std::string> run_command(const char* cmd_str);

} // namespace platform

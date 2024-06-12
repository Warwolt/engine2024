#pragma once

#include <lean_mean_windows.h>

#include <expected>
#include <optional>
#include <string>

namespace platform {

	using ExitCode = int;

	std::string get_win32_error();
	std::expected<ExitCode, std::string> run_command(const char* cmd_str);
	std::optional<std::string> show_save_dialog(HWND hwnd, const char* title, const char* file_extension, const char* extension_description);

} // namespace platform

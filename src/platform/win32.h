#pragma once

#include <lean_mean_windows.h>

#include <expected>
#include <optional>
#include <string>

namespace platform {

	using ExitCode = int;

	struct FileExplorerDialog {
		std::string title;
		std::string description;
		std::string extension;
	};

	enum class UnsavedChangesDialogChoice {
		Save,
		DontSave,
		Cancel,
	};

	std::string get_win32_error();
	std::expected<ExitCode, std::string> run_command(const char* cmd_str);
	UnsavedChangesDialogChoice show_unsaved_changes_dialog(const std::string& document_name);
	std::optional<std::string> show_load_dialog(HWND hwnd, const FileExplorerDialog* dialog);
	std::optional<std::string> show_save_dialog(HWND hwnd, const FileExplorerDialog* dialog);

} // namespace platform

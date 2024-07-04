#include <platform/win32.h>

#include <platform/logging.h>

#include <commctrl.h>
#include <commdlg.h>

#include <filesystem>

// Spooky linker magic courtesy of https://stackoverflow.com/a/43215416/3157744
// This makes sure that we load some control related stuff for Comctl32.lib
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace platform {

	std::string get_win32_error() {
		DWORD err_code = GetLastError();
		char* err_msg;
		if (!FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				err_code,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
				(LPTSTR)&err_msg,
				0,
				NULL
			)) {
			return "";
		}

		static char buffer[1024];
		_snprintf_s(buffer, sizeof(buffer), "%s", err_msg);
		LocalFree(err_msg);

		return std::string(buffer);
	}

	std::string application_name() {
		return application_path().string();
	}

	const std::filesystem::path& application_path() {
		static std::filesystem::path path; // memoize, this value is constant for full program lifetime

		if (path.empty()) {
			TCHAR sz_file_name[MAX_PATH];
			GetModuleFileName(NULL, sz_file_name, MAX_PATH);
			path = sz_file_name;
		}

		return path;
	}

	std::expected<ExitCode, std::string> run_command(const char* cmd_str) {
		// this function based on:
		// https://learn.microsoft.com/en-gb/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN

		/* Setup pipes */
		HANDLE stdout_read;
		HANDLE stdout_write;
		HANDLE stderr_read;
		HANDLE stderr_write;
		{
			SECURITY_ATTRIBUTES security_attr;
			security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
			security_attr.bInheritHandle = TRUE;
			security_attr.lpSecurityDescriptor = NULL;

			if (!CreatePipe(&stdout_read, &stdout_write, &security_attr, 0)) {
				return std::unexpected("stdout CreatePipe failed: " + platform::get_win32_error());
			}

			if (!CreatePipe(&stderr_read, &stderr_write, &security_attr, 0)) {
				return std::unexpected("stderr CreatePipe failed: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("stdout SetHandleInformation failed: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("stdout SetHandleInformation failed: " + platform::get_win32_error());
			}
		}

		/* Run command */
		std::string cmd(cmd_str);
		PROCESS_INFORMATION process_info = { 0 };
		{
			STARTUPINFO startup_info = { 0 };
			startup_info.cb = sizeof(STARTUPINFO);
			startup_info.hStdOutput = stdout_write;
			startup_info.hStdError = stderr_write;
			startup_info.dwFlags = STARTF_USESTDHANDLES;

			if (!CreateProcess(
					NULL, // path
					&cmd[0],
					NULL, // process security attributes
					NULL, // primary thread security attributes
					TRUE, // handles are inherited
					0, // creation flags
					NULL, // use parent's environment
					NULL, // use parent's current directory
					&startup_info,
					&process_info
				)) {
				return std::unexpected("CreateProcess(\"" + cmd + "\") failed: " + platform::get_win32_error());
			}

			// close handles
			CloseHandle(stdout_write);
			CloseHandle(stderr_write);
		}
		LOG_INFO("Running \"%s\"", cmd.c_str());

		/* Read from stdout until done */
		while (true) {
			constexpr size_t BUFSIZE = 256;
			char stdout_data[BUFSIZE] = { 0 };
			if (!ReadFile(stdout_read, stdout_data, BUFSIZE, NULL, NULL)) {
				break;
			}
			IF_PLOG(plog::info) {
				printf("%s", stdout_data);
			}
		}

		/* Read exit code */
		DWORD exit_code;
		if (!GetExitCodeProcess(process_info.hProcess, &exit_code)) {
			return std::unexpected("GetExitCodeProcess failed: " + platform::get_win32_error());
		}

		/* Finish */
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
		if (exit_code != 0) {
			LOG_ERROR("\"%s\" failed with exit code: %d", cmd_str, exit_code);
		}
		else {
			LOG_INFO("\"%s\" completed successfully", cmd_str);
		}

		return (int)exit_code;
	}

	UnsavedChangesDialogChoice show_unsaved_changes_dialog(const std::string& document_name) {
		wchar_t main_instruction[256];
		swprintf_s(main_instruction, 256, L"Do you want to save the changes you made to %S?", document_name.c_str());

		TASKDIALOGCONFIG config = { 0 };
		const TASKDIALOG_BUTTON buttons[] = {
			{ IDYES, L"&Save" },
			{ IDNO, L"Do&n't Save" },
		};
		config.cbSize = sizeof(config);
		config.hInstance = GetModuleHandle(NULL);
		config.dwCommonButtons = TDCBF_CANCEL_BUTTON;
		config.pszMainIcon = TD_WARNING_ICON;
		config.pszMainInstruction = main_instruction;
		config.pszContent = L"Your changes will be lost if you don't save them.";
		config.pButtons = buttons;
		config.cButtons = ARRAYSIZE(buttons);

		int pressed_button = 0;
		TaskDialogIndirect(&config, &pressed_button, NULL, NULL);
		switch (pressed_button) {
			case IDYES:
				return UnsavedChangesDialogChoice::Save;

			case IDNO:
				return UnsavedChangesDialogChoice::DontSave;

			case IDCANCEL:
			default:
				return UnsavedChangesDialogChoice::Cancel;
		}
	}

	std::optional<std::filesystem::path> show_load_dialog(HWND hwnd, const FileExplorerDialog* dialog) {
		char path[MAX_PATH] = "";

		char filter[256] = "";
		snprintf(filter, 256, "%s%c*.%s%cAll Files (*.*)%c*.*\0", dialog->description.c_str(), '\0', dialog->extension.c_str(), '\0', '\0');

		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = path;
		ofn.nMaxFile = MAX_PATH - 1;
		ofn.lpstrFilter = filter;
		ofn.lpstrDefExt = dialog->extension.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.lpstrTitle = dialog->title.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn)) {
			return std::filesystem::path(path);
		}

		return {};
	}

	std::optional<std::filesystem::path> show_save_dialog(HWND hwnd, const FileExplorerDialog* dialog) {
		char path[MAX_PATH] = "";

		char filter[256] = "";
		snprintf(filter, 256, "%s%c*.%s%cAll Files (*.*)%c*.*\0", dialog->description.c_str(), '\0', dialog->extension.c_str(), '\0', '\0');

		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = path;
		ofn.nMaxFile = MAX_PATH - 1;
		ofn.lpstrFilter = filter;
		ofn.lpstrDefExt = dialog->extension.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.lpstrTitle = dialog->title.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn)) {
			return std::filesystem::path(path);
		}

		return {};
	}

} // namespace platform

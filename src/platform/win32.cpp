#include <platform/win32.h>

#include <platform/logging.h>

#include <commdlg.h>

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

	std::optional<std::string> show_load_dialog(HWND hwnd, const FileExplorerDialog* dialog) {
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
			return path;
		}

		return {};
	}

	std::optional<std::string> show_save_dialog(HWND hwnd, const FileExplorerDialog* dialog) {
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
			return path;
		}

		return {};
	}

} // namespace platform

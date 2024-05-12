#include <platform/win32.h>

#include <platform/logging.h>

#include <lean_mean_windows.h>

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

	std::expected<void, std::string> run_command(const char* cmd_str) {
		// this function based on:
		// https://learn.microsoft.com/en-gb/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN

		std::string cmd(cmd_str);

		/* Setup pipes */
		HANDLE std_out_read;
		HANDLE std_out_write;
		HANDLE std_err_read;
		HANDLE std_err_write;
		{
			SECURITY_ATTRIBUTES security_attr;
			security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
			security_attr.bInheritHandle = TRUE;
			security_attr.lpSecurityDescriptor = NULL;

			if (!CreatePipe(&std_out_read, &std_out_write, &security_attr, 0)) {
				return std::unexpected("std_out CreatePipe failed: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(std_out_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("std_out SetHandleInformation failed: " + platform::get_win32_error());
			}

			if (!CreatePipe(&std_err_read, &std_err_write, &security_attr, 0)) {
				return std::unexpected("std_err CreatePipe failew: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(std_err_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("std_out SetHandleInformation failed: " + platform::get_win32_error());
			}
		}

		/* Run command */
		// TODO: run this in a background thread not to block main thread?
		PROCESS_INFORMATION process_info = { 0 };
		{
			STARTUPINFO startup_info = { 0 };
			startup_info.cb = sizeof(STARTUPINFO);
			startup_info.hStdOutput = std_out_write;
			startup_info.hStdError = std_err_write;
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
			CloseHandle(std_out_write);
			CloseHandle(std_err_write);
		}
		LOG_INFO("Running \"%s\"", cmd.c_str());

		/* Read from stdout until done */
		{
			while (true) {
				constexpr size_t BUFSIZE = 256;
				DWORD read = 0;
				char buf[BUFSIZE] = { 0 };
				if (!ReadFile(std_out_read, buf + read, BUFSIZE, &read, NULL)) {
					break;
				}
				IF_PLOG(plog::info) {
					printf("  %s", buf);
				}

				DWORD exit_code;
				if (!GetExitCodeProcess(process_info.hProcess, &exit_code)) {
					return std::unexpected("GetExitCodeProcess failed: " + platform::get_win32_error());
				}
				if (exit_code != STILL_ACTIVE) {
					break;
				}
			}
		}

		DWORD exit_code = 0;
		GetExitCodeProcess(process_info.hProcess, &exit_code);

		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);

		if (exit_code != 0) {
			LOG_ERROR("\"%s\" failed with exit code: %d", cmd.c_str(), exit_code);
		}
		else {
			LOG_INFO("\"%s\" completed successfully", cmd.c_str());
		}

		return {};
	}

} // namespace platform

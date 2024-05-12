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
				return std::unexpected("stderr CreatePipe failew: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("stdout SetHandleInformation failed: " + platform::get_win32_error());
			}

			if (!SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0)) {
				return std::unexpected("stdout SetHandleInformation failed: " + platform::get_win32_error());
			}
		}

		/* Run command */
		// TODO: run this in a background thread not to block main thread?
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
			LOG_ERROR("\"%s\" failed with exit code: %d", cmd.c_str(), exit_code);
		}
		else {
			LOG_INFO("\"%s\" completed successfully", cmd.c_str());
		}

		return {};
	}

} // namespace platform

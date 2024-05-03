#include <platform/library_loader.h>

#include <platform/assert.h>
#include <platform/logging.h>

#define LOAD_FUNCTION(hmodule, engine_library, function_name)                                                                          \
	do {                                                                                                                               \
		decltype(engine_library.function_name) fn = (decltype(engine_library.function_name))(GetProcAddress(hmodule, #function_name)); \
		ASSERT(fn != nullptr, "GetProcAddress(\"" #function_name "\") returned null. Is the function name correct?");                  \
		engine_library.function_name = fn;                                                                                             \
	} while (0)

namespace platform {

	std::string get_winapi_error() {
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

	std::optional<std::string> get_dll_full_path(HMODULE dll_module) {
		char dll_full_path[MAX_PATH];
		if (GetModuleFileName(dll_module, dll_full_path, sizeof(dll_full_path)) == 0) {
			std::string error = get_winapi_error();
			LOG_ERROR("GetModuleFileName failed: %s", error.c_str());
			return {};
		}
		return std::string(dll_full_path);
	}

	std::optional<FILETIME> file_last_modified(const char* file_path) {
		/* Open file */
		HANDLE file = CreateFileA(
			file_path,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		/* Read time modified */
		FILETIME create_time, access_time, write_time;
		if (!GetFileTime(file, &create_time, &access_time, &write_time)) {
			std::string error = get_winapi_error();
			LOG_ERROR("GetFileTime failed: %s", error.c_str());
			CloseHandle(file);
			return {};
		}

		FILETIME localized_write_time;
		if (!FileTimeToLocalFileTime(&write_time, &localized_write_time)) {
			std::string error = get_winapi_error();
			LOG_ERROR("FileTimeToLocalFileTime failed: %s", error.c_str());
			CloseHandle(file);
			return {};
		}

		CloseHandle(file);
		return localized_write_time;
	}

	std::string filetime_to_string(const FILETIME* filetime) {
		SYSTEMTIME st;
		char buffer[128];
		FileTimeToSystemTime(filetime, &st);
		sprintf_s(buffer, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		return std::string(buffer);
	}

	EngineLibraryLoader::~EngineLibraryLoader() {
		unload_library();
	}

	std::expected<EngineLibrary, LoadLibraryError> EngineLibraryLoader::load_library(const char* library_name) {
		/* Load library */
		HMODULE library = LoadLibrary(library_name);
		if (!library) {
			std::string error = get_winapi_error();
			LOG_ERROR("LoadLibrary(\"%s\") failed: %s", library_name, error.c_str());
			return std::unexpected(LoadLibraryError::FileDoesNotExist);
		}

		/* Get full path of library */
		m_library_path = get_dll_full_path(library).value_or(std::string());
		if (m_library_path.empty()) {
			return std::unexpected(LoadLibraryError::FailedToGetLibraryFullPath);
		}
		m_copied_library_path = m_library_path.substr(0, m_library_path.size() - 4) + "-copy.dll";

		/* Create a copy of library, so original file can still be modified */
		const bool fail_if_already_exists = false; // overwrite file if already exists
		if (!CopyFile(m_library_path.c_str(), m_copied_library_path.c_str(), fail_if_already_exists)) {
			std::string error = get_winapi_error();
			LOG_ERROR("CopyFile(\"%s\", \"%s\", %s) failed: ", m_library_path.c_str(), m_copied_library_path.c_str(), fail_if_already_exists ? "true" : "false", error.c_str());
			return std::unexpected(LoadLibraryError::FailedToCopyLibrary);
		}

		/* Load copied DLL*/
		std::string copied_library_name = std::string(library_name) + "-copy";
		m_copied_library = LoadLibrary(copied_library_name.c_str());
		if (!m_copied_library) {
			std::string error = get_winapi_error();
			LOG_ERROR("LoadLibrary(\"%s\") failed: ", copied_library_name.c_str(), error.c_str());
			return std::unexpected(LoadLibraryError::FailedToLoadCopiedLibrary);
		}

		/* Read last write timestamp */
		std::optional<FILETIME> timestamp = file_last_modified(m_library_path.c_str());
		if (!timestamp.has_value()) {
			std::string error = get_winapi_error();
			LOG_ERROR("file_last_modified() failed for \"%s\"", m_library_path.c_str(), error.c_str());
			return std::unexpected(LoadLibraryError::FailedToReadLastModifiedTime);
		}
		m_last_library_write = timestamp.value();
		FreeLibrary(library); // done with original DLL, free it now

		/* Read functions */
		EngineLibrary engine_library;
		LOAD_FUNCTION(m_copied_library, engine_library, on_load);
		LOAD_FUNCTION(m_copied_library, engine_library, update);
		LOAD_FUNCTION(m_copied_library, engine_library, render);

		return engine_library;
	}

	bool EngineLibraryLoader::library_file_has_been_modified() const {
		if (std::optional<FILETIME> library_write = file_last_modified(m_library_path.c_str())) {
			return CompareFileTime(&library_write.value(), &m_last_library_write) != 0;
		}
		return false;
	}

	void EngineLibraryLoader::unload_library() const {
		/* Free copied engine DLL */
		if (!FreeLibrary(m_copied_library)) {
			std::string error = get_winapi_error();
			LOG_ERROR("FreeLibrary failed: %s", error.c_str());
		}

		/* Delete copied engine DLL */
		if (!DeleteFile(m_copied_library_path.c_str())) {
			std::string error = get_winapi_error();
			LOG_ERROR("DeleteFile(\"%s\") failed: %s", m_copied_library_path.c_str(), error.c_str());
		}
	}

} // namespace platform

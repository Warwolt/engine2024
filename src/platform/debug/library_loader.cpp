#include <platform/debug/library_loader.h>

#include <core/future.h>
#include <core/unwrap.h>
#include <core/util.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/os/imwin32.h>
#include <platform/os/win32.h>

#include <imgui/imgui.h>

#define LOAD_FUNCTION(hmodule, library, function_name)                                                                                       \
	do {                                                                                                                                     \
		decltype(library.function_name) fn = (decltype(library.function_name))(GetProcAddress(hmodule, #function_name));                     \
		ASSERT(fn != nullptr, "GetProcAddress(\"" #function_name "\") returned null. Is the function name \"" #function_name "\" correct?"); \
		library.function_name = fn;                                                                                                          \
	} while (0)

namespace platform {

	std::optional<std::string> get_dll_full_path(HMODULE dll_module) {
		char dll_full_path[MAX_PATH];
		if (GetModuleFileName(dll_module, dll_full_path, sizeof(dll_full_path)) == 0) {
			std::string error = get_win32_error();
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
			// it's expected for this to fail when trying to reload, so don't print any error
			CloseHandle(file);
			return {};
		}

		FILETIME localized_write_time;
		if (!FileTimeToLocalFileTime(&write_time, &localized_write_time)) {
			std::string error = get_win32_error();
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
		HMODULE hmodule = LoadLibrary(library_name);
		if (!hmodule) {
			std::string error = get_win32_error();
			LOG_ERROR("LoadLibrary(\"%s\") failed: %s", library_name, error.c_str());
			return std::unexpected(LoadLibraryError::FileDoesNotExist);
		}

		/* Get full path of library */
		m_library_path = get_dll_full_path(hmodule).value_or(std::string());
		if (m_library_path.empty()) {
			return std::unexpected(LoadLibraryError::FailedToGetLibraryFullPath);
		}
		m_copied_library_path = m_library_path.substr(0, m_library_path.size() - 4) + "-copy.dll";

		/* Create a copy of library, so original file can still be modified */
		constexpr bool fail_if_already_exists = false; // overwrite file if already exists
		if (!CopyFile(m_library_path.c_str(), m_copied_library_path.c_str(), fail_if_already_exists)) {
			std::string error = get_win32_error();
			LOG_ERROR("CopyFile(\"%s\", \"%s\", %s) failed: ", m_library_path.c_str(), m_copied_library_path.c_str(), fail_if_already_exists ? "true" : "false", error.c_str());
			return std::unexpected(LoadLibraryError::FailedToCopyLibrary);
		}

		/* Load copied DLL*/
		std::string copied_library_name = std::string(library_name) + "-copy";
		m_copied_library = LoadLibrary(copied_library_name.c_str());
		if (!m_copied_library) {
			std::string error = get_win32_error();
			LOG_ERROR("LoadLibrary(\"%s\") failed: ", copied_library_name.c_str(), error.c_str());
			return std::unexpected(LoadLibraryError::FailedToLoadCopiedLibrary);
		}

		/* Read last write timestamp */
		std::optional<FILETIME> timestamp = file_last_modified(m_library_path.c_str());
		if (!timestamp.has_value()) {
			std::string error = get_win32_error();
			LOG_ERROR("file_last_modified() failed for \"%s\"", m_library_path.c_str(), error.c_str());
			return std::unexpected(LoadLibraryError::FailedToReadLastModifiedTime);
		}
		m_last_library_write = timestamp.value();
		FreeLibrary(hmodule); // done with original DLL, free it now

		/* Read functions */
		EngineLibrary library;
		LOAD_FUNCTION(m_copied_library, library, set_logger);
		LOAD_FUNCTION(m_copied_library, library, set_imgui_context);
		LOAD_FUNCTION(m_copied_library, library, set_imwin32_context);
		LOAD_FUNCTION(m_copied_library, library, set_freetype_library);

		// engine interface
		LOAD_FUNCTION(m_copied_library, library, initialize_engine);
		LOAD_FUNCTION(m_copied_library, library, shutdown_engine);
		LOAD_FUNCTION(m_copied_library, library, update_engine);
		LOAD_FUNCTION(m_copied_library, library, render_engine);
		LOAD_FUNCTION(m_copied_library, library, load_engine_data);

		// editor interface
		LOAD_FUNCTION(m_copied_library, library, initialize_editor);
		LOAD_FUNCTION(m_copied_library, library, shutdown_editor);
		LOAD_FUNCTION(m_copied_library, library, update_editor);
		LOAD_FUNCTION(m_copied_library, library, render_editor);

		return library;
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
			std::string error = get_win32_error();
			LOG_ERROR("FreeLibrary failed: %s", error.c_str());
		}

		/* Delete copied engine DLL */
		if (!DeleteFile(m_copied_library_path.c_str())) {
			std::string error = get_win32_error();
			LOG_ERROR("DeleteFile(\"%s\") failed: %s", m_copied_library_path.c_str(), error.c_str());
		}
	}

	EngineLibraryHotReloader::EngineLibraryHotReloader(EngineLibraryLoader* library_loader, const char* library_name)
		: m_library_loader(library_loader)
		, m_library_name(library_name) {
	}

	void EngineLibraryHotReloader::update(EngineLibrary* library) {
		/* Check if library has been reloaded on disk */
		if (m_hot_reload_timer.elapsed_ms() >= 1000) {
			m_hot_reload_timer.reset();

			if (m_library_loader->library_file_has_been_modified()) {
				m_library_loader->unload_library();
				*library = core::unwrap(m_library_loader->load_library(m_library_name.c_str()), [&](LoadLibraryError error) {
					ABORT("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s", m_library_name.c_str(), core::util::enum_to_string(error));
				});
				on_engine_library_loaded(library);
				LOG_INFO("Engine library reloaded");
			}
		}

		/* Check rebuild command progress */
		if (m_rebuild_command_is_running) {
			if (core::future::has_value(m_rebuild_engine_future)) {
				m_last_exit_code = m_rebuild_engine_future.get();
				m_rebuild_command_is_running = false;
			}
		}
	}

	void EngineLibraryHotReloader::trigger_rebuild_command() {
		if (!m_rebuild_command_is_running) {
			m_rebuild_command_is_running = true;
			m_rebuild_engine_future = std::async(std::launch::async, [] {
				const char* cmd = "cmake --build build --target GameEngine2024Library";
				std::expected<ExitCode, std::string> result = platform::run_command(cmd);
				if (!result.has_value()) {
					LOG_ERROR("run_command failed: %s", result.error().c_str());
				}
				return result.value_or(-1);
			});
		}
	}

	bool EngineLibraryHotReloader::rebuild_command_is_running() const {
		return m_rebuild_command_is_running;
	}

	ExitCode EngineLibraryHotReloader::last_exit_code() const {
		return m_last_exit_code;
	}

	void on_engine_library_loaded(EngineLibrary* library) {
		library->set_logger(plog::verbose, plog::get());
		library->set_imgui_context(ImGui::GetCurrentContext());
		library->set_imwin32_context(ImWin32::GetCurrentContext());
		library->set_freetype_library(platform::get_ft());
	}

} // namespace platform

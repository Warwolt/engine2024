#pragma once

#include <engine.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <expected>
#include <functional>
#include <string>

namespace platform {

	using EngineUpdateFn = void(engine::EngineState*);

	struct EngineLibrary {
		std::function<EngineUpdateFn> engine_update = [](engine::EngineState*) {};
	};

	enum class LoadLibraryError {
		FileDoesNotExist,
		FailedToGetLibraryFullPath,
		FailedToCopyLibrary,
		FailedToLoadCopiedLibrary,
		FailedToReadLastModifiedTime,
	};

	const char* load_library_error_to_string(LoadLibraryError err);

	class EngineLibraryLoader {
	public:
		std::expected<EngineLibrary, LoadLibraryError> load_library(const char* library_name);
		void unload_library() const;
		bool library_file_has_been_modified() const;

	private:
		const char* m_library_name;
		std::string m_library_path;
		std::string m_copied_library_path;
		FILETIME m_last_library_write;
		HMODULE m_copied_library;
	};

} // namespace platform

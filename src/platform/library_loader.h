#pragma once

#include <engine.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <expected>
#include <functional>
#include <optional>
#include <string>

namespace platform {

	using EngineOnLoadFn = void(plog::Severity, plog::IAppender*);
	using EngineUpdateFn = void(engine::EngineState*, uint64_t);

	struct EngineLibrary {
		std::function<EngineOnLoadFn> on_load;
		std::function<EngineUpdateFn> update;
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
		EngineLibraryLoader() = default;
		~EngineLibraryLoader();

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

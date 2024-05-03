#pragma once

#include <engine.h>

#include <windows.h>

#include <expected>
#include <functional>
#include <optional>
#include <string>

namespace platform {

	using EngineSaveStateFn = void(nlohmann::json*);
	using EngineLoadStateFn = void(nlohmann::json*);
	using EngineInitLoggingFn = void(plog::Severity, plog::IAppender*);
	using EngineUpdateFn = platform::Commands(const platform::Input*);
	using EngineRenderFn = void(platform::Renderer*);

	struct EngineLibrary {
		std::function<EngineSaveStateFn> save_state;
		std::function<EngineLoadStateFn> load_state;
		std::function<EngineInitLoggingFn> init_logging;
		std::function<EngineUpdateFn> update;
		std::function<EngineRenderFn> render;
	};

	enum class LoadLibraryError {
		FileDoesNotExist,
		FailedToGetLibraryFullPath,
		FailedToCopyLibrary,
		FailedToLoadCopiedLibrary,
		FailedToReadLastModifiedTime,
	};

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

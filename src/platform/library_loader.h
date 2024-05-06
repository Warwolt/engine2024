#pragma once

#include <engine.h>
#include <platform/input/timing.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <expected>
#include <functional>
#include <optional>
#include <string>

namespace platform {

	struct EngineLibrary {
		void (*set_logger)(plog::Severity, plog::IAppender*);
		void (*initialize)(engine::State*);
		void (*deinitialize)(engine::State*);
		platform::Commands (*update)(engine::State*, const platform::Input*);
		void (*render)(platform::Renderer*, engine::State*);
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

	class EngineLibraryHotReloader {
	public:
		EngineLibraryHotReloader(EngineLibraryLoader* library_loader, const char* library_name);
		void check_hot_reloading(EngineLibrary* engine_library);

	private:
		std::string m_library_name;
		EngineLibraryLoader* m_library_loader;
		platform::Timer m_hot_reload_timer;
	};

} // namespace platform

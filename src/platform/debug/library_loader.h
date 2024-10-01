#pragma once

#include <library.h>
#include <platform/input/timing.h>
#include <platform/os/win32.h>

#include <platform/os/lean_mean_windows.h>

#include <expected>
#include <functional>
#include <future>
#include <optional>
#include <string>

namespace platform {

	struct EngineLibrary {
		// global state wiring
		void (*set_logger)(plog::Severity, plog::IAppender*);
		void (*set_imgui_context)(ImGuiContext* imgui_context);
		void (*set_imwin32_context)(ImWin32::ImWin32Context* imwin32_context);
		void (*set_freetype_library)(FT_Library ft);

		// engine interface
		engine::Engine* (*initialize_engine)(platform::OpenGLContext*);
		void (*shutdown_engine)(engine::Engine*, platform::OpenGLContext*);
		void (*update_engine)(engine::Engine*, const platform::Input&, platform::PlatformAPI*, platform::OpenGLContext*);
		void (*render_engine)(const engine::Engine&, platform::Renderer*);
		void (*load_engine_data)(engine::Engine*, const char* path);

		// editor interface
		editor::Editor* (*initialize_editor)(engine::Engine* engine, platform::OpenGLContext* gl_context, const platform::Configuration& config);
		void (*shutdown_editor)(editor::Editor* editor);
		void (*update_editor)(editor::Editor* editor, const platform::Configuration& config, const platform::Input& input, engine::Engine* engine, platform::PlatformAPI* platform, platform::OpenGLContext* gl_context);
		void (*render_editor)(const editor::Editor& editor, const engine::Engine& engine, platform::OpenGLContext* gl_context, platform::Renderer* renderer);
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
		void update(EngineLibrary* engine_library);
		void trigger_rebuild_command();
		bool rebuild_command_is_running() const;
		ExitCode last_exit_code() const;

	private:
		std::string m_library_name;
		EngineLibraryLoader* m_library_loader;
		platform::Timer m_hot_reload_timer;

		std::future<ExitCode> m_rebuild_engine_future;
		ExitCode m_last_exit_code = 0;
		bool m_rebuild_command_is_running = false;
	};

	void on_engine_library_loaded(EngineLibrary* engine_library);

} // namespace platform

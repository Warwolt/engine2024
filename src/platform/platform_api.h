#pragma once

#include <platform/window.h>

#include <string>
#include <vector>

namespace platform {

	enum class PlatformCommandType {
		ChangeResolution,
		Quit,
		RebuildEngineLibrary,
		SetRunMode,
		SetCursor,
		SetWindowMode,
		SetWindowTitle,
		ToggleFullscreen,
	};

	enum class RunMode {
		Game,
		Editor,
	};

	enum class Cursor {
		Arrow,
		SizeAll,
	};

	union PlatformCommand {
		PlatformCommandType type;

		struct ChangeResolution {
			PlatformCommandType type = PlatformCommandType::ChangeResolution;
			int width;
			int height;
		} change_resolution;

		struct Quit {
			PlatformCommandType type = PlatformCommandType::Quit;
		} quit;

		struct RebuildEngineLibrary {
			PlatformCommandType type = PlatformCommandType::RebuildEngineLibrary;
		} rebuild_engine_library;

		struct SetCursor {
			PlatformCommandType type = PlatformCommandType::SetCursor;
			Cursor cursor;
		} set_cursor;

		struct SetRunMode {
			PlatformCommandType type = PlatformCommandType::SetRunMode;
			RunMode mode;
		} set_run_mode;

		struct SetWindowMode {
			PlatformCommandType type = PlatformCommandType::SetWindowMode;
			WindowMode mode;
		} set_window_mode;

		struct SetWindowTitle {
			PlatformCommandType type = PlatformCommandType::SetWindowTitle;
			char title[128];
		} set_window_title;

		struct ToggleFullscreen {
			PlatformCommandType type = PlatformCommandType::ToggleFullscreen;
		} toggle_full_screen;
	};

	class PlatformAPI {
	public:
		const std::vector<PlatformCommand>& commands() const;
		void clear();

		void change_resolution(int width, int height);
		void quit();
		void rebuild_engine_library();
		void set_cursor(Cursor cursor);
		void set_run_mode(RunMode mode);
		void set_window_mode(WindowMode mode);
		void set_window_title(const char* title);
		void toggle_fullscreen();

	private:
		std::vector<PlatformCommand> m_commands;
	};

} // namespace platform

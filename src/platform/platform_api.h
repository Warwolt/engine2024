#pragma once

#include <platform/window.h>

#include <string>
#include <vector>

namespace platform {

	enum class PlatformCommandType {
		// application
		Quit,
		SetRunMode,

		// cursor
		SetCursor,

		// file
		RebuildEngineLibrary,
		SaveFileWithDialog,

		// window
		ChangeResolution,
		SetWindowMode,
		SetWindowTitle,
		ToggleFullscreen,
	};

	enum class Cursor {
		Arrow,
		SizeAll,
	};

	enum class RunMode {
		Game,
		Editor,
	};

	union PlatformCommand {
		PlatformCommandType type;

		// application
		struct Quit {
			PlatformCommandType type = PlatformCommandType::Quit;
		} quit;

		struct SetRunMode {
			PlatformCommandType type = PlatformCommandType::SetRunMode;
			RunMode mode;
		} set_run_mode;

		// cursor
		struct SetCursor {
			PlatformCommandType type = PlatformCommandType::SetCursor;
			Cursor cursor;
		} set_cursor;

		// file
		struct SaveFileWithDialog {
			PlatformCommandType type = PlatformCommandType::SaveFileWithDialog;
			uint8_t* data;
			size_t length;
		} save_file_with_dialog;

		struct RebuildEngineLibrary {
			PlatformCommandType type = PlatformCommandType::RebuildEngineLibrary;
		} rebuild_engine_library;

		// window
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

		struct ChangeResolution {
			PlatformCommandType type = PlatformCommandType::ChangeResolution;
			int width;
			int height;
		} change_resolution;
	};

	class PlatformAPI {
	public:
		const std::vector<PlatformCommand>& commands() const;

		void clear();

		// application
		void quit();
		void set_run_mode(RunMode mode);

		// cursor
		void set_cursor(Cursor cursor);

		// file
		void rebuild_engine_library();
		void save_file_with_dialog(const uint8_t* data, size_t length);

		// window
		void change_resolution(int width, int height);
		void set_window_mode(WindowMode mode);
		void set_window_title(const char* title);
		void toggle_fullscreen();

	private:
		std::vector<PlatformCommand> m_commands;
	};

} // namespace platform

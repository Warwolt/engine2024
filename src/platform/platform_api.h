#pragma once

#include <string>
#include <vector>

namespace platform {

	enum class CommandType {
		ChangeResolution,
		Quit,
		RebuildEngineLibrary,
		SetWindowTitle,
		SetCursor,
		ToggleFullscreen,
	};

	enum class Cursor {
		Arrow,
		SizeAll,
	};

	union Command {
		CommandType type;

		struct ChangeResolution {
			CommandType type = CommandType::ChangeResolution;
			int width;
			int height;
		} change_resolution;

		struct Quit {
			CommandType type = CommandType::Quit;
		} quit;

		struct RebuildEngineLibrary {
			CommandType type = CommandType::RebuildEngineLibrary;
		} rebuild_engine_library;

		struct SetCursor {
			CommandType type = CommandType::SetCursor;
			Cursor cursor;
		} set_cursor;

		struct SetWindowTitle {
			CommandType type = CommandType::SetWindowTitle;
			char title[128];
		} set_window_title;

		struct ToggleFullscreen {
			CommandType type = CommandType::ToggleFullscreen;
		} toggle_full_screen;
	};

	class PlatformAPI {
	public:
		const std::vector<Command>& commands() const;
		void clear();

		void change_resolution(int width, int height);
		void quit();
		void rebuild_engine_library();
		void set_cursor(Cursor cursor);
		void set_window_title(const char* title);
		void toggle_fullscreen();

	private:
		std::vector<Command> m_commands;
	};

} // namespace platform

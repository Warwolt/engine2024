#pragma once

#include <string>
#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
		ToggleFullscreen,
		ChangeResolution,
		SetWindowTitle,
		RebuildEngineLibrary,
	};

	union Command {
		CommandType type;

		struct Quit {
			CommandType type = CommandType::Quit;
		} quit;

		struct ToggleFullscreen {
			CommandType type = CommandType::ToggleFullscreen;
		} toggle_full_screen;

		struct SetWindowTitle {
			CommandType type = CommandType::SetWindowTitle;
			char title[128];
		} set_window_title;

		struct ChangeResolution {
			CommandType type = CommandType::ChangeResolution;
			int width;
			int height;
		} change_resolution;

		struct RebuildEngineLibrary {
			CommandType type = CommandType::RebuildEngineLibrary;
		} rebuild_engine_library;
	};

	class CommandAPI {
	public:
		const std::vector<Command>& commands() const;
		void clear();

		void quit();
		void toggle_fullscreen();
		void set_window_title(const char* title);
		void change_resolution(int width, int height);
		void rebuild_engine_library();

	private:
		std::vector<Command> m_commands;
	};

} // namespace platform

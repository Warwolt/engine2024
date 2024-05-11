#pragma once

#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
		ToggleFullscreen,
		ChangeResolution,
	};

	union Command {
		CommandType type;

		struct Quit {
			CommandType type = CommandType::Quit;
		} quit;

		struct ToggleFullscreen {
			CommandType type = CommandType::ToggleFullscreen;
		} toggle_full_screen;

		struct ChangeResolution {
			CommandType type = CommandType::ChangeResolution;
			int width;
			int height;
		} change_resolution;
	};

	class CommandAPI {
	public:
		const std::vector<Command>& commands() const;
		void clear();

		void quit();
		void toggle_fullscreen();
		void change_resolution(int width, int height);

	private:
		std::vector<Command> m_commands;
	};

} // namespace platform

#pragma once

#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
		ToggleFullscreen,
	};

	union Command {
		CommandType type;

		struct Quit {
			CommandType type = CommandType::Quit;
		} quit;

		struct ToggleFullscreen {
			CommandType type = CommandType::ToggleFullscreen;
		} toggle_full_screen;
	};

	class CommandAPI {
	public:
		const std::vector<Command>& commands() const;
		void clear();

		void quit();
		void toggle_fullscreen();

	private:
		std::vector<Command> m_commands;
	};

} // namespace platform

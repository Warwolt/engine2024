#pragma once

#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
		ToggleFullscreen,
	};

	struct Quit {
		CommandType type = CommandType::Quit;
	};

	struct ToggleFullscreen {
		CommandType type = CommandType::ToggleFullscreen;
	};

	union Command {
		CommandType type;
		Quit quit;
		ToggleFullscreen toggle_full_screen;
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

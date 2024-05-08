#pragma once

#include <vector>

namespace platform {

	enum class Command {
		Quit,
		ToggleFullscreen,
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

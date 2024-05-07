#pragma once

#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
		ToggleFullscreen,
	};

	class CommandAPI {
	public:
		const std::vector<CommandType>& commands() const;
		void clear();

		void quit();
		void toggle_fullscreen();

	private:
		std::vector<CommandType> m_commands;
	};

} // namespace platform

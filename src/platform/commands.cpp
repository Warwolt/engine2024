#include <platform/commands.h>

namespace platform {

	const std::vector<CommandType>& CommandAPI::commands() const {
		return m_commands;
	}

	void CommandAPI::clear() {
		m_commands.clear();
	}

	void CommandAPI::quit() {
		m_commands.push_back(CommandType::Quit);
	}

	void CommandAPI::toggle_fullscreen() {
		m_commands.push_back(CommandType::ToggleFullscreen);
	}

} // namespace platform

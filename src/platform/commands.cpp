#include <platform/commands.h>

namespace platform {

	const std::vector<Command>& CommandAPI::commands() const {
		return m_commands;
	}

	void CommandAPI::clear() {
		m_commands.clear();
	}

	void CommandAPI::quit() {
		m_commands.push_back({ .quit = Quit() });
	}

	void CommandAPI::toggle_fullscreen() {
		m_commands.push_back({ .toggle_full_screen = ToggleFullscreen() });
	}

} // namespace platform

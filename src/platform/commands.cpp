#include <platform/commands.h>

namespace platform {

	const std::vector<Command>& CommandAPI::commands() const {
		return m_commands;
	}

	void CommandAPI::clear() {
		m_commands.clear();
	}

	void CommandAPI::quit() {
		m_commands.push_back({ .quit = Command::Quit() });
	}

	void CommandAPI::toggle_fullscreen() {
		m_commands.push_back({ .toggle_full_screen = Command::ToggleFullscreen() });
	}

	void CommandAPI::change_resolution(int width, int height) {
		Command::ChangeResolution change_resolution = {
			.width = width,
			.height = height,
		};
		m_commands.push_back({ .change_resolution = change_resolution });
	}

	void CommandAPI::rebuild_engine_library() {
		m_commands.push_back({ .rebuild_engine_library = Command::RebuildEngineLibrary() });
	}

} // namespace platform

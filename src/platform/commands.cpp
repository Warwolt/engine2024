#include <platform/commands.h>

#include <string.h>

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

	void CommandAPI::set_window_title(const char* title) {
		Command::SetWindowTitle data;
		strcpy_s(data.title, title);
		m_commands.push_back({ .set_window_title = data });
	}

	void CommandAPI::change_resolution(int width, int height) {
		m_commands.push_back({ .change_resolution = {
								   .width = width,
								   .height = height,
							   } });
	}

	void CommandAPI::rebuild_engine_library() {
		m_commands.push_back({ .rebuild_engine_library = Command::RebuildEngineLibrary() });
	}

} // namespace platform

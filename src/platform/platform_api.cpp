#include <platform/platform_api.h>

#include <string.h>

namespace platform {

	const std::vector<PlatformCommand>& PlatformAPI::commands() const {
		return m_commands;
	}

	void PlatformAPI::clear() {
		m_commands.clear();
	}

	void PlatformAPI::change_resolution(int width, int height) {
		m_commands.push_back({ .change_resolution = {
								   .width = width,
								   .height = height,
							   } });
	}

	void PlatformAPI::rebuild_engine_library() {
		m_commands.push_back({ .rebuild_engine_library = PlatformCommand::RebuildEngineLibrary() });
	}

	void PlatformAPI::set_cursor(Cursor cursor) {
		PlatformCommand::SetCursor set_cursor;
		set_cursor.cursor = cursor;
		m_commands.push_back({ .set_cursor = set_cursor });
	}

	void PlatformAPI::quit() {
		m_commands.push_back({ .quit = PlatformCommand::Quit() });
	}

	void PlatformAPI::toggle_fullscreen() {
		m_commands.push_back({ .toggle_full_screen = PlatformCommand::ToggleFullscreen() });
	}

	void PlatformAPI::set_window_title(const char* title) {
		PlatformCommand::SetWindowTitle data;
		strcpy_s(data.title, title);
		m_commands.push_back({ .set_window_title = data });
	}

} // namespace platform

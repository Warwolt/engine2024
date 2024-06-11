#include <platform/platform_api.h>

#include <memory.h>
#include <string.h>

namespace platform {

	const std::vector<PlatformCommand>& PlatformAPI::commands() const {
		return m_commands;
	}

	void PlatformAPI::clear() {
		for (const PlatformCommand& cmd : m_commands) {
			if (cmd.type == PlatformCommandType::SaveFileWithDialog) {
				free(cmd.save_file_with_dialog.data);
			}
		}
		m_commands.clear();
	}

	/* Application */
	void PlatformAPI::quit() {
		m_commands.push_back({ .quit = PlatformCommand::Quit() });
	}

	void PlatformAPI::set_run_mode(RunMode mode) {
		PlatformCommand::SetRunMode set_run_mode;
		set_run_mode.mode = mode;
		m_commands.push_back({ .set_run_mode = set_run_mode });
	}

	/* Cursor */
	void PlatformAPI::set_cursor(Cursor cursor) {
		PlatformCommand::SetCursor set_cursor;
		set_cursor.cursor = cursor;
		m_commands.push_back({ .set_cursor = set_cursor });
	}

	/* File */
	void PlatformAPI::rebuild_engine_library() {
		m_commands.push_back({ .rebuild_engine_library = PlatformCommand::RebuildEngineLibrary() });
	}

	void PlatformAPI::save_file_with_dialog(const uint8_t* data, size_t length) {
		PlatformCommand::SaveFileWithDialog save_file_with_dialog;
		save_file_with_dialog.length = length;
		save_file_with_dialog.data = (uint8_t*)malloc(length);
		memcpy(save_file_with_dialog.data, data, length);
		m_commands.push_back({ .save_file_with_dialog = save_file_with_dialog });
	}

	/* Window */
	void PlatformAPI::change_resolution(int width, int height) {
		PlatformCommand::ChangeResolution change_resolution;
		change_resolution.width = width;
		change_resolution.height = height;
		m_commands.push_back({ .change_resolution = change_resolution });
	}

	void PlatformAPI::set_window_mode(WindowMode mode) {
		PlatformCommand::SetWindowMode set_window_mode;
		set_window_mode.mode = mode;
		m_commands.push_back({ .set_window_mode = set_window_mode });
	}

	void PlatformAPI::set_window_title(const char* title) {
		PlatformCommand::SetWindowTitle data;
		strcpy_s(data.title, title);
		m_commands.push_back({ .set_window_title = data });
	}

	void PlatformAPI::toggle_fullscreen() {
		m_commands.push_back({ .toggle_full_screen = PlatformCommand::ToggleFullscreen() });
	}

} // namespace platform

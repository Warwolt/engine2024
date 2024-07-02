#include <platform/platform_api.h>

#include <memory.h>
#include <string.h>

namespace platform {

	std::vector<PlatformCommand>& PlatformAPI::commands() {
		return m_commands;
	}

	void PlatformAPI::clear() {
		m_commands.clear();
	}

	void PlatformAPI::quit() {
		m_commands.push_back(cmd::app::Quit {});
	}

	void PlatformAPI::set_run_mode(RunMode run_mode) {
		m_commands.push_back(cmd::app::SetRunMode { run_mode });
	}

	void PlatformAPI::rebuild_engine_library() {
		m_commands.push_back(cmd::app::RebuildEngineLibrary {});
	}

	void PlatformAPI::set_cursor(Cursor cursor) {
		m_commands.push_back(cmd::cursor::SetCursor { cursor });
	}

	void PlatformAPI::load_file_with_dialog(FileExplorerDialog dialog, std::function<void(const std::vector<uint8_t>&)> on_file_loaded) {
		m_commands.push_back(cmd::file::LoadFileWithDialog {
			.on_file_loaded = std::move(on_file_loaded),
			.dialog = dialog,
		});
	}

	void PlatformAPI::save_file_with_dialog(std::vector<uint8_t> data, FileExplorerDialog dialog) {
		m_commands.push_back(cmd::file::SaveFileWithDialog {
			.data = data,
			.dialog = dialog,
		});
	}

	void PlatformAPI::change_resolution(int width, int height) {
		m_commands.push_back(cmd::window::ChangeResolution {
			.width = width,
			.height = height,
		});
	}

	void PlatformAPI::set_window_mode(WindowMode window_mode) {
		m_commands.push_back(cmd::window::SetWindowMode { window_mode });
	}

	void PlatformAPI::toggle_fullscreen() {
		m_commands.push_back(cmd::window::ToggleFullscreen {});
	}

	void PlatformAPI::set_window_title(const char* title) {
		m_commands.push_back(cmd::window::SetWindowTitle { title });
	}

} // namespace platform

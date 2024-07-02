#include <platform/platform_api.h>

#include <memory.h>
#include <string.h>

namespace platform {

	std::vector<PlatformCommand> PlatformAPI::drain_commands() {
		return std::move(m_commands);
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

	void PlatformAPI::load_file_with_dialog(FileExplorerDialog dialog, std::function<void(std::vector<uint8_t>, std::filesystem::path)> on_file_loaded) {
		m_commands.push_back(cmd::file::LoadFileWithDialog {
			.on_file_loaded = std::move(on_file_loaded),
			.dialog = dialog,
		});
	}

	void PlatformAPI::save_file(const std::vector<uint8_t>& data, const std::filesystem::path& path, std::function<void()> on_file_saved) {
		m_commands.push_back(cmd::file::SaveFile {
			.on_file_saved = on_file_saved,
			.path = path,
			.data = data,
		});
	}

	void PlatformAPI::save_file_with_dialog(const std::vector<uint8_t>& data, FileExplorerDialog dialog, std::function<void(std::filesystem::path)> on_file_saved) {
		m_commands.push_back(cmd::file::SaveFileWithDialog {
			.on_file_saved = on_file_saved,
			.data = data,
			.dialog = dialog,
		});
	}

	void PlatformAPI::show_unsaved_changes_dialog(const std::string& document_name, std::function<void(platform::UnsavedChangesDialogChoice)> on_dialog_choice) {
		m_commands.push_back(cmd::file::ShowUnsavedChangesDialog {
			.on_dialog_choice = on_dialog_choice,
			.document_name = document_name,
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

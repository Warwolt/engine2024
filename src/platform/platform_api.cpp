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

	std::future<SaveFileResult<std::filesystem::path>> PlatformAPI::save_file(const std::vector<uint8_t>& data, const std::filesystem::path& path) {
		std::promise<SaveFileResult<std::filesystem::path>> result_promise;
		std::future<SaveFileResult<std::filesystem::path>> result_future = result_promise.get_future();
		m_commands.push_back(cmd::file::SaveFile {
			.result_promise = std::move(result_promise),
			.path = path,
			.data = data,
		});
		return result_future;
	}

	std::future<SaveFileResult<std::filesystem::path>> PlatformAPI::save_file_with_dialog(const std::vector<uint8_t>& data, FileExplorerDialog dialog) {
		std::promise<SaveFileResult<std::filesystem::path>> result_promise;
		std::future<SaveFileResult<std::filesystem::path>> result_future = result_promise.get_future();
		m_commands.push_back(cmd::file::SaveFileWithDialog {
			.result_promise = std::move(result_promise),
			.data = data,
			.dialog = dialog,
		});
		return result_future;
	}

	std::future<platform::UnsavedChangesDialogChoice> PlatformAPI::show_unsaved_changes_dialog(const std::string& document_name) {
		std::promise<platform::UnsavedChangesDialogChoice> choice_promise;
		std::future<platform::UnsavedChangesDialogChoice> choice_future = choice_promise.get_future();
		m_commands.push_back(cmd::file::ShowUnsavedChangesDialog {
			.choice_promise = std::move(choice_promise),
			.document_name = document_name,
		});
		return choice_future;
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

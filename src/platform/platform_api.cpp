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

	std::future<LoadFileResult<LoadFileData>> PlatformAPI::load_file_with_dialog(FileExplorerDialog dialog) {
		std::promise<LoadFileResult<LoadFileData>> result_promise;
		std::future<LoadFileResult<LoadFileData>> result_future = result_promise.get_future();
		auto load_file_with_dialog = cmd::file::LoadFileWithDialog {
			.result_promise = std::move(result_promise),
			.dialog = dialog
		};
		m_commands.push_back(std::move(load_file_with_dialog));
		return result_future;
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

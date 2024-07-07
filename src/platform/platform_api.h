#pragma once

#include <core/tagged_variant.h>
#include <platform/win32.h>
#include <platform/window.h>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace platform {

	enum class PlatformCommandType {
		// app
		Quit,
		RebuildEngineLibrary,
		SetRunMode,

		// cursor
		SetCursor,

		// file
		LoadFileWithDialog,
		SaveFile,
		SaveFileWithDialog,
		ShowUnsavedChangesDialog,

		// window
		ChangeResolution,
		SetWindowMode,
		SetWindowTitle,
		ToggleFullscreen,
	};

	enum class Cursor {
		Arrow,
		SizeAll,
	};

	enum class RunMode {
		Game,
		Editor,
	};

	namespace cmd::app {

		struct Quit {
			static constexpr auto TAG = PlatformCommandType::Quit;
		};

		struct RebuildEngineLibrary {
			static constexpr auto TAG = PlatformCommandType::RebuildEngineLibrary;
		};

		struct SetRunMode {
			static constexpr auto TAG = PlatformCommandType::SetRunMode;
			RunMode mode;
		};

	} // namespace cmd::app

	namespace cmd::cursor {

		struct SetCursor {
			static constexpr auto TAG = PlatformCommandType::SetCursor;
			Cursor cursor;
		};

	} // namespace cmd::cursor

	namespace cmd::file {

		struct LoadFileWithDialog {
			static constexpr auto TAG = PlatformCommandType::LoadFileWithDialog;
			std::function<void(std::vector<uint8_t>, std::filesystem::path)> on_file_loaded;
			FileExplorerDialog dialog;
		};

		struct SaveFile {
			static constexpr auto TAG = PlatformCommandType::SaveFile;
			std::function<void()> on_file_saved;
			std::filesystem::path path;
			std::vector<uint8_t> data;
		};

		struct SaveFileWithDialog {
			static constexpr auto TAG = PlatformCommandType::SaveFileWithDialog;
			std::function<void(std::filesystem::path)> on_file_saved;
			std::vector<uint8_t> data;
			FileExplorerDialog dialog;
		};

		struct ShowUnsavedChangesDialog {
			static constexpr auto TAG = PlatformCommandType::ShowUnsavedChangesDialog;
			std::function<void(platform::UnsavedChangesDialogChoice)> on_dialog_choice;
			std::string document_name;
		};

	} // namespace cmd::file

	namespace cmd::window {

		struct ChangeResolution {
			static constexpr auto TAG = PlatformCommandType::ChangeResolution;
			int width;
			int height;
		};

		struct SetWindowMode {
			static constexpr auto TAG = PlatformCommandType::SetWindowMode;
			WindowMode mode;
		};

		struct SetWindowTitle {
			static constexpr auto TAG = PlatformCommandType::SetWindowTitle;
			std::string title;
		};

		struct ToggleFullscreen {
			static constexpr auto TAG = PlatformCommandType::ToggleFullscreen;
		};

	} // namespace cmd::window

	using PlatformCommand = core::TaggedVariant<
		PlatformCommandType,
		cmd::app::Quit,
		cmd::app::RebuildEngineLibrary,
		cmd::app::SetRunMode,
		cmd::cursor::SetCursor,
		cmd::file::LoadFileWithDialog,
		cmd::file::SaveFile,
		cmd::file::SaveFileWithDialog,
		cmd::file::ShowUnsavedChangesDialog,
		cmd::window::ChangeResolution,
		cmd::window::SetWindowMode,
		cmd::window::SetWindowTitle,
		cmd::window::ToggleFullscreen>;

	class PlatformAPI {
	public:
		std::vector<PlatformCommand> drain_commands();

		// application
		void quit();
		void rebuild_engine_library();
		void set_run_mode(RunMode mode);

		// cursor
		void set_cursor(Cursor cursor);

		// file
		void load_file_with_dialog(FileExplorerDialog dialog, std::function<void(std::vector<uint8_t>, std::filesystem::path)> on_file_loaded);
		void save_file(const std::vector<uint8_t>& data, const std::filesystem::path& path, std::function<void()> on_file_saved);
		void save_file_with_dialog(const std::vector<uint8_t>& data, FileExplorerDialog dialog, std::function<void(std::filesystem::path)> on_file_saved);
		void show_unsaved_changes_dialog(const std::string& document_name, std::function<void(platform::UnsavedChangesDialogChoice)> on_dialog_choice);

		// window
		void change_resolution(int width, int height);
		void set_window_mode(WindowMode mode);
		void set_window_title(const std::string& title);
		void toggle_fullscreen();

	private:
		std::vector<PlatformCommand> m_commands;
	};

} // namespace platform

#pragma once

#include <platform/tagged_variant.h>
#include <platform/win32.h>
#include <platform/window.h>

#include <future>
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
		SaveFileWithDialog,

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
			std::promise<std::vector<uint8_t>> promise;
			FileExplorerDialog dialog;
		};

		struct SaveFileWithDialog {
			static constexpr auto TAG = PlatformCommandType::SaveFileWithDialog;
			std::vector<uint8_t> data;
			FileExplorerDialog dialog;
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

	using PlatformCommand = TaggedVariant<
		PlatformCommandType,
		cmd::app::Quit,
		cmd::app::RebuildEngineLibrary,
		cmd::app::SetRunMode,
		cmd::cursor::SetCursor,
		cmd::file::LoadFileWithDialog,
		cmd::file::SaveFileWithDialog,
		cmd::window::ChangeResolution,
		cmd::window::SetWindowMode,
		cmd::window::SetWindowTitle,
		cmd::window::ToggleFullscreen>;

	class PlatformAPI {
	public:
		const std::vector<PlatformCommand>& commands() const;

		void clear();

		// application
		void quit();
		void rebuild_engine_library();
		void set_run_mode(RunMode mode);

		// cursor
		void set_cursor(Cursor cursor);

		// file
		std::future<std::vector<uint8_t>> load_file_with_dialog(FileExplorerDialog dialog);
		void save_file_with_dialog(std::vector<uint8_t> data, FileExplorerDialog dialog);

		// window
		void change_resolution(int width, int height);
		void set_window_mode(WindowMode mode);
		void set_window_title(const char* title);
		void toggle_fullscreen();

	private:
		std::vector<PlatformCommand> m_commands;
	};

} // namespace platform

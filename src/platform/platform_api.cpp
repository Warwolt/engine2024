#include <GL/glew.h>

#include <platform/platform_api.h>

#include <platform/debug/logging.h>
#include <platform/graphics/font.h>

#include <memory.h>
#include <string.h>
#include <utility>

namespace {
	plog::Severity opengl_severity_to_plog_severity(GLenum severity) {
		switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH:
				return plog::Severity::error;
			case GL_DEBUG_SEVERITY_MEDIUM:
			case GL_DEBUG_SEVERITY_LOW:
				return plog::Severity::warning;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				return plog::Severity::verbose;
		}
		return plog::none;
	}

	void GLAPIENTRY on_opengl_error(
		GLenum /*source*/,
		GLenum /*type*/,
		GLuint /*id*/,
		GLenum gl_severity,
		GLsizei /*length*/,
		const GLchar* message,
		const void* /*userParam*/
	) {
		plog::Severity log_severity = opengl_severity_to_plog_severity(gl_severity);
		LOG(log_severity, "%s", message);
	}
}

namespace platform {

	bool initialize() {
		/* Initialize SDL */
		if (SDL_Init(SDL_INIT_VIDEO)) {
			LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
			return false;
		}

		/* Initialize SDL OpenGL support */
		glEnable(GL_DEBUG_OUTPUT);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		/* Initialize fonts */
		if (!initialize_fonts()) {
			LOG_ERROR("initialize_fonts failed");
			return false;
		}

		return true;
	}

	void shutdown(SDL_GLContext gl_context) {
		shutdown_fonts();
		SDL_GL_DeleteContext(gl_context);
		SDL_Quit();
	}

	std::expected<SDL_GLContext, CreateGLContextError> create_gl_context(SDL_Window* window) {
		/* Create GL Context */
		printf("SDL_GL_CreateContext\n");
		SDL_GLContext gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			LOG_ERROR("SDL_GL_CreateContext failed: %s", SDL_GetError());
			return std::unexpected(CreateGLContextError::FailedToCreateContext);
		}
		printf("SDL_GL_MakeCurrent\n");
		SDL_GL_MakeCurrent(window, gl_context);

		/* Initialize GLEW */
		printf("glewInit\n");
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			LOG_ERROR("glewInit failed: %s", glewGetErrorString(glewError));
			return std::unexpected(CreateGLContextError::FailedToInitializeGlew);
		}

		// /* Set OpenGL error callback */
		// printf("glDebugMessageCallback\n");
		// glDebugMessageCallback(on_opengl_error, 0);

		// /* Enable v-sync */
		// printf("SDL_GL_SetSwapInterval\n");
		// if (SDL_GL_SetSwapInterval(1)) {
		// 	LOG_ERROR("SDL_GL_SetSwapInterval failed: %s", SDL_GetError());
		// 	return std::unexpected(CreateGLContextError::FailedToSetVSync);
		// }

		// /* Enable alpha channel */
		// printf("glEnable(GL_BLEND)\n");
		// glEnable(GL_BLEND);
		// printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)\n");
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return gl_context;
	}

	bool PlatformAPI::has_commands() {
		return !m_commands.empty();
	}

	std::vector<PlatformCommand> PlatformAPI::drain_commands() {
		return std::exchange(m_commands, {});
	}

	void PlatformAPI::clear_log() {
		m_commands.push_back(cmd::app::ClearLog {});
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

	void PlatformAPI::set_window_title(const std::string& title) {
		m_commands.push_back(cmd::window::SetWindowTitle { title });
	}

} // namespace platform

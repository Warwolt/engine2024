#include <GL/glew.h>

#include <platform/platform.h>

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

	const char* create_gl_context_error_to_string(CreateGLContextError error) {
		switch (error) {
			case CreateGLContextError::FailedToCreateContext:
				return "CreateGLContextError::FailedToCreateContext";
			case CreateGLContextError::FailedToInitializeGlew:
				return "CreateGLContextError::FailedToInitializeGlew";
			case CreateGLContextError::FailedToSetVSync:
				return "CreateGLContextError::FailedToSetVSync";
		}
		return "";
	}

	bool initialize() {
		/* Initialize SDL */
		if (SDL_Init(SDL_INIT_VIDEO)) {
			LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
			return false;
		}

		/* Initialize SDL OpenGL support */
		glEnable(GL_DEBUG_OUTPUT);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		return true;
	}

	SDL_Window* create_window() {
		/* Create window */
		SDL_Window* window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			680,
			480,
			SDL_WINDOW_OPENGL
		);
		if (!window) {
			LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
		}
		return window;
	}

	std::expected<SDL_GLContext, CreateGLContextError> create_gl_context(SDL_Window* window) {
		/* Create GL Context */
		SDL_GLContext gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			LOG_ERROR("SDL_GL_CreateContext failed: %s", SDL_GetError());
			return std::unexpected(CreateGLContextError::FailedToCreateContext);
		}

		/* Initialize GLEW */
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			LOG_ERROR("glewInit failed: %s", glewGetErrorString(glewError));
			return std::unexpected(CreateGLContextError::FailedToInitializeGlew);
		}

		/* Set OpenGL error callback */
		glDebugMessageCallback(on_opengl_error, 0);

		/* Enable v-sync */
		if (SDL_GL_SetSwapInterval(1)) {
			LOG_ERROR("SDL_GL_SetSwapInterval failed: %s", SDL_GetError());
			return std::unexpected(CreateGLContextError::FailedToSetVSync);
		}

		return gl_context;
	}

} // namespace platform

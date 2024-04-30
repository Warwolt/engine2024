#include <GL/glew.h>

#include <engine.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/renderer.h>
#include <platform/timing.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;

using Primitive = platform::Primitive;
using Renderer = platform::Renderer;
using ShaderProgram = platform::ShaderProgram;
using ShaderProgramError = platform::ShaderProgramError;
using Vertex = platform::Vertex;
using VertexSection = platform::VertexSection;

const char* VERTEX_SHADER_SRC =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
	"    vertexColor = vec4(aColor, 1.0);\n"
	"}";

const char* FRAGMENT_SHADER_SRC =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec4 vertexColor;\n"
	"void main() {\n"
	"    FragColor = vertexColor;\n"
	"}";

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

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	/* Initialize SDL + OpenGL*/
	SDL_Window* window;
	SDL_GLContext gl_context;
	{
		/* Initialize SDL */
		if (SDL_Init(SDL_INIT_VIDEO)) {
			LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
			exit(1);
		}

		/* Initialize OpenGL */
		glEnable(GL_DEBUG_OUTPUT);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		/* Create window */
		window = SDL_CreateWindow(
			"Game Engine 2024",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			680,
			480,
			SDL_WINDOW_OPENGL
		);
		if (!window) {
			LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
			exit(1);
		}

		/* Create GL Context */
		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			LOG_ERROR("SDL_GL_CreateContext failed: %s", SDL_GetError());
			exit(1);
		}

		/* Initialize GLEW */
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			LOG_ERROR("glewInit failed: %s", glewGetErrorString(glewError));
			exit(1);
		}

		/* Set OpenGL error callback */
		glDebugMessageCallback(on_opengl_error, 0);

		/* Enable v-sync */
		if (SDL_GL_SetSwapInterval(1)) {
			LOG_ERROR("SDL_GL_SetSwapInterval failed: %s", SDL_GetError());
		}
	}

	/* Initialize OpenGL */
	Renderer renderer;
	ShaderProgram shader_program;
	{
		std::expected<ShaderProgram, ShaderProgramError> result = renderer.add_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
		if (!result.has_value()) {
			const char* error_msg = platform::shader_program_error_to_string(result.error());
			LOG_ERROR("Renderer::add_program() failed with: %s", error_msg);
			exit(1);
		}
		shader_program = result.value();
	}

	/* Load engine DLL */
	const char* library_name = "GameEngine2024";
	EngineLibraryLoader library_loader;
	EngineLibrary engine_library;
	{
		std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
		if (!load_result.has_value()) {
			const char* error_str = load_library_error_to_string(load_result.error());
			LOG_ERROR("EngineLibraryLoader::load_library(%s) failed with: %s", library_name, error_str);
			exit(1);
		}
		engine_library = load_result.value();
	};
	LOG_INFO("Engine library loaded");

	/* Main loop */
	timing::Timer frame_timer;
	timing::Timer hot_reload_timer;
	engine::EngineState engine_state;
	bool quit = false;
	while (!quit) {
		const uint64_t delta_ms = frame_timer.elapsed_ms();
		frame_timer.reset();

		/* Hot reloading */
		if (hot_reload_timer.elapsed_ms() >= 1000) {
			hot_reload_timer.reset();

			if (library_loader.library_file_has_been_modified()) {
				library_loader.unload_library();
				{
					std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
					if (!load_result.has_value()) {
						const char* error_msg = load_library_error_to_string(load_result.error());
						LOG_ERROR("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s", library_name, error_msg);

					} else {
						LOG_INFO("Engine library reloaded");
						engine_library = load_result.value();
					}
				}
			}
		}

		/* Input */
		{
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						quit = true;
					case SDL_KEYDOWN:
						if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
							quit = true;
						}
				}
			}
		}

		/* Update */
		engine_library.engine_update(&engine_state, delta_ms);
		renderer.draw_rect_fill({ -0.5f, 0.5f }, { 0.5f, -0.5f }, { 1.0f, 0.5f, 0.0f });

		/* Render */
		renderer.render(window, shader_program);
	}

	/* Unload and delete copied engine DLL */
	// FIXME: `unload_library` should be called in the destructor of LibraryLoader
	library_loader.unload_library();

	/* Shutdown SDL */
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	return 0;
}

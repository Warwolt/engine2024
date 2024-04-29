#include <engine.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/timing.h>

#include <GL/glew.h>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;

const char* vertex_shader_src =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = vec4(aPos, 1.0);\n"
	"    vertexColor = vec4(aColor, 1.0);\n"
	"}";

const char* fragment_shader_src =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec4 vertexColor;\n"
	"void main() {\n"
	"    FragColor = vertexColor;\n"
	"}";

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
	LOG(log_severity, "%s\n", message);
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
			LOG_ERROR("SDL_Init failed: %s\n", SDL_GetError());
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
			LOG_ERROR("SDL_CreateWindow failed: %s\n", SDL_GetError());
			exit(1);
		}

		/* Create GL Context */
		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context) {
			LOG_ERROR("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
			exit(1);
		}

		/* Initialize GLEW */
		const GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			LOG_ERROR("glewInit failed: %s\n", glewGetErrorString(glewError));
			exit(1);
		}

		/* Set VSync */
		if (SDL_GL_SetSwapInterval(1)) {
			LOG_ERROR("SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
		}

		/* Set OpenGL error callback */
		glDebugMessageCallback(on_opengl_error, 0);
	}

	/* Initialize shader */
	GLuint shader_program = 0;
	{
		shader_program = glCreateProgram();
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

		/* Vertex shader */
		glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
		glCompileShader(vertex_shader);
		GLint vertex_shader_compiled = GL_FALSE;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
		if (vertex_shader_compiled != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
			LOG_ERROR("Vertex shader failed to compile:\n%s\n", info_log);
			exit(1);
		}
		glAttachShader(shader_program, vertex_shader);

		/* Fragment shader */
		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
		glCompileShader(fragment_shader);
		GLint fragment_shader_compiled = GL_FALSE;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
		if (fragment_shader_compiled != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
			LOG_ERROR("Fragment shader failed to compile:\n%s\n", info_log);
			exit(1);
		}
		glAttachShader(shader_program, fragment_shader);

		/* Shader program */
		glLinkProgram(shader_program);
		GLint shader_program_linked = GL_FALSE;
		glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_program_linked);
		if (shader_program_linked != GL_TRUE) {
			char info_log[512] = { 0 };
			glGetProgramInfoLog(shader_program, 512, NULL, info_log);
			LOG_ERROR("Shader program failed to link:\n%s\n", info_log);
			exit(1);
		}

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	/* Setup render data */
	GLuint vao = 0;
	GLuint vbo = 0;
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		/* Set buffer data */
		// clang-format off
		float vertices[] = {
			// positions         // colors
			0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // bottom right
			-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
			0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f    // top
		};
		// clang-format on
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		/* Configure vertex attributes */
		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
		glEnableVertexAttribArray(0);
		// color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		/* Unbind */
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
	}

	/* Load engine DLL */
	const char* library_name = "GameEngine2024";
	EngineLibraryLoader library_loader;
	EngineLibrary engine_library;
	{
		std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
		if (load_result.has_value()) {
			engine_library = load_result.value();
		} else {
			LOG_ERROR("EngineLibraryLoader::load_library(%s) failed with: %s\n", library_name, load_library_error_to_string(load_result.error()));
			exit(1);
		}
	}
	LOG_INFO("Engine library loaded\n");

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

				std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
				if (load_result.has_value()) {
					engine_library = load_result.value();
					LOG_INFO("Engine library reloaded\n");
				} else {
					LOG_ERROR("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s\n", library_name, load_library_error_to_string(load_result.error()));
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

		/* Render */
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(shader_program);
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
			glBindVertexArray(NULL);

			SDL_GL_SwapWindow(window);
		}
	}

	/* Unload and delete copied engine DLL */
	library_loader.unload_library();

	/* Shutdown SDL + OpenGL */
	{
		glDeleteBuffers(1, &vbo);
		glDeleteProgram(shader_program);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	return 0;
}

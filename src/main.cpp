#include <engine.h>
#include <platform/library_loader.h>
#include <platform/logging.h>
#include <platform/timing.h>

#include <GL/glew.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/vec3.hpp>

using EngineLibrary = platform::EngineLibrary;
using EngineLibraryLoader = platform::EngineLibraryLoader;
using LoadLibraryError = platform::LoadLibraryError;

#include <expected>
#include <vector>

struct RendererVertex {
	struct {
		float x;
		float y;
		float z;
	} pos;
	struct {
		float r;
		float g;
		float b;
	} color;
};

enum class ShaderProgramError {
	VertexShaderFailedToCompile,
	FragmentShaderFailedToCompile,
	ShaderProgramFailedToLink,
};

const char* shader_program_error_to_string(ShaderProgramError error) {
	switch (error) {
		case ShaderProgramError::VertexShaderFailedToCompile:
			return "ShaderProgramError::VertexShaderFailedToCompile";
		case ShaderProgramError::FragmentShaderFailedToCompile:
			return "ShaderProgramError::FragmentShaderFailedToCompile";
		case ShaderProgramError::ShaderProgramFailedToLink:
			return "ShaderProgramError::ShaderProgramFailedToLink";
	}
	return "";
}

struct ShaderProgram {
	GLuint id;
	GLuint vao;
	GLuint vbo;
};

class Renderer {
public:
	Renderer(SDL_Window* window);
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	std::expected<ShaderProgram, ShaderProgramError> add_program(const char* vertex_src, const char* fragment_src);

	SDL_GLContext m_gl_context = nullptr;
	std::vector<ShaderProgram> m_shader_programs;
};

const char* VERTEX_SHADER_SRC =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aColor;\n"
	"out vec4 vertexColor;\n"
	"void main() {\n"
	"    gl_Position = vec4(aPos, 1.0);\n"
	"    vertexColor = vec4(aColor, 1.0);\n"
	"}";

const char* FRAGMENT_SHADER_SRC =
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
	LOG(log_severity, "%s", message);
}

Renderer::Renderer(SDL_Window* window) {
	/* Create GL Context */
	m_gl_context = SDL_GL_CreateContext(window);
	if (!m_gl_context) {
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

Renderer::~Renderer() {
	for (const ShaderProgram& shader_program : m_shader_programs) {
		glDeleteVertexArrays(1, &shader_program.vao);
		glDeleteBuffers(1, &shader_program.vbo);
		glDeleteProgram(shader_program.id);
	}
}

std::expected<ShaderProgram, ShaderProgramError> Renderer::add_program(const char* vertex_src, const char* fragment_src) {
	GLuint shader_program_id = glCreateProgram();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	/* Vertex shader */
	glShaderSource(vertex_shader, 1, &vertex_src, NULL);
	glCompileShader(vertex_shader);
	GLint vertex_shader_compiled = GL_FALSE;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
	if (vertex_shader_compiled != GL_TRUE) {
		char info_log[512] = { 0 };
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		LOG_ERROR("Vertex shader failed to compile:\n%s", info_log);
		return std::unexpected(ShaderProgramError::VertexShaderFailedToCompile);
	}
	glAttachShader(shader_program_id, vertex_shader);

	/* Fragment shader */
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_src, NULL);
	glCompileShader(fragment_shader);
	GLint fragment_shader_compiled = GL_FALSE;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
	if (fragment_shader_compiled != GL_TRUE) {
		char info_log[512] = { 0 };
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		LOG_ERROR("Fragment shader failed to compile:\n%s", info_log);
		return std::unexpected(ShaderProgramError::FragmentShaderFailedToCompile);
	}
	glAttachShader(shader_program_id, fragment_shader);

	/* Shader program */
	glLinkProgram(shader_program_id);
	GLint shader_program_linked = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &shader_program_linked);
	if (shader_program_linked != GL_TRUE) {
		char info_log[512] = { 0 };
		glGetProgramInfoLog(shader_program_id, 512, NULL, info_log);
		LOG_ERROR("Shader program failed to link:\n%s", info_log);
		return std::unexpected(ShaderProgramError::ShaderProgramFailedToLink);
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	/* Create VAO and VBO */
	GLuint vao = 0;
	GLuint vbo = 0;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

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

	ShaderProgram shader_program = { shader_program_id, vao, vbo };
	m_shader_programs.push_back(shader_program);
	return shader_program;
}

int main(int /* argc */, char** /* args */) {
	platform::init_logging();
	LOG_INFO("Game Engine 2024 initializing");

	glm::vec3 vec = glm::vec3(1.0, 2.0, 3.0);
	LOG_INFO("vec = %s", glm::to_string(vec).c_str());

	/* Initialize SDL + OpenGL*/
	SDL_Window* window;
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
	}

	/* Initialize OpenGL */
	Renderer renderer = Renderer(window);
	ShaderProgram shader_program;
	{
		std::expected<ShaderProgram, ShaderProgramError> add_result = renderer.add_program(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
		if (add_result.has_value()) {
			shader_program = add_result.value();
		} else {
			LOG_ERROR("Renderer::add_program() failed with: %s", shader_program_error_to_string(add_result.error()));
			exit(1);
		}
	}

	/* Upload vertices */
	{
		/* Set buffer data */
		RendererVertex vertices[] = {
			// positions         // colors
			{ .pos = { 0.5f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f } }, // bottom right
			{ .pos = { -0.5f, -0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f } }, // bottom left
			{ .pos = { 0.0f, 0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f } }, // top
		};

		glBindVertexArray(shader_program.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
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
			LOG_ERROR("EngineLibraryLoader::load_library(%s) failed with: %s", library_name, load_library_error_to_string(load_result.error()));
			exit(1);
		}
	}
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

				std::expected<EngineLibrary, LoadLibraryError> load_result = library_loader.load_library(library_name);
				if (load_result.has_value()) {
					engine_library = load_result.value();
					LOG_INFO("Engine library reloaded");
				} else {
					LOG_ERROR("Failed to reload engine library, EngineLibraryLoader::load_library(%s) failed with: %s", library_name, load_library_error_to_string(load_result.error()));
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
			glUseProgram(shader_program.id);
			glBindVertexArray(shader_program.vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, NULL);
			glBindVertexArray(NULL);

			SDL_GL_SwapWindow(window);
		}
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

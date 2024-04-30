#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/renderer.h>

#include <platform/logging.h>

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

	GLenum primitive_to_draw_array_mode(Primitive primitive) {
		switch (primitive) {
			case Primitive::Point:
				return GL_POINTS;
			case Primitive::Line:
				return GL_LINES;
			case Primitive::Triangle:
				return GL_TRIANGLES;
		}
		return 0;
	}

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

	void DrawData::clear() {
		m_vertices.clear();
		m_sections.clear();
	}

	void DrawData::draw_rect(glm::vec3 p0, glm::vec3 p1, glm::vec3 color) {
		// float x0 = p0.x;
		// float x1 = p1.x;
		// float y0 = p0.y;
		// float y1 = p1.y;

		// m_vertices.push_back()

		// (x0, y0) -------- (x1, y0)
		//     |                |
		//     |                |
		//     |                |
		// (x0, y1) -------- (x1, y1)
		(void)(p0);
		(void)(p1);
		(void)(color);
		//
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

	void Renderer::render(SDL_Window* window, ShaderProgram shader_program, const DrawData* draw_data) {
		// clear screen
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader_program.id);
		glBindVertexArray(shader_program.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);

		// upload vertices
		{
			glBufferData(GL_ARRAY_BUFFER, draw_data->m_vertices.size() * sizeof(Vertex), draw_data->m_vertices.data(), GL_STATIC_DRAW);
		}

		// draw vertices
		{
			GLint offset = 0;
			for (const VertexSection& section : draw_data->m_sections) {
				GLenum mode = platform::primitive_to_draw_array_mode(section.primitive);
				glDrawArrays(mode, offset, section.length);
				offset += section.length;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
		glUseProgram(NULL);

		SDL_GL_SwapWindow(window);
	}

} // namespace platform

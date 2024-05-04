#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/renderer.h>

#include <platform/logging.h>
#include <stb_image/stb_image.h>

namespace platform {

	static GLenum primitive_to_draw_array_mode(Primitive primitive) {
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

	Renderer::Renderer(SDL_GLContext /* gl_context */) {
		unsigned char data[] = { 0xFF, 0xFF, 0xFF };
		m_white_texture = add_texture(data, 1, 1);
	}

	Renderer::~Renderer() {
		for (const Texture& texture : m_textures) {
			glDeleteTextures(1, &texture.id);
		}

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
		glVertexAttribPointer(
			0,
			sizeof(Vertex::pos) / sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Vertex),
			(void*)(0)
		);
		glEnableVertexAttribArray(0);
		// color
		glVertexAttribPointer(
			1,
			sizeof(Vertex::color) / sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Vertex),
			(void*)(sizeof(Vertex::pos))
		);
		glEnableVertexAttribArray(1);
		// texture coordinates
		glVertexAttribPointer(
			2,
			sizeof(Vertex::uv) / sizeof(float),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Vertex),
			(void*)(sizeof(Vertex::pos) + sizeof(Vertex::color))
		);
		glEnableVertexAttribArray(2);

		/* Unbind */
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);

		ShaderProgram shader_program = { shader_program_id, vao, vbo };
		m_shader_programs.push_back(shader_program);
		return shader_program;
	}

	Texture Renderer::add_texture(const unsigned char* data, int width, int height) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // is this really needed?

		m_textures.push_back(Texture { texture });

		glBindTexture(GL_TEXTURE_2D, NULL);
		return Texture { texture };
	}

	void Renderer::render(SDL_Window* window, ShaderProgram shader_program) {
		// clear screen
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader_program.id);
		glBindVertexArray(shader_program.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);

		// upload vertices
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

		// draw vertices
		{
			GLint offset = 0;
			for (const VertexSection& section : m_sections) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, section.texture.id);

				GLenum mode = primitive_to_draw_array_mode(section.primitive);
				glDrawArrays(mode, offset, section.length);
				offset += section.length;
			}
		}

		m_vertices.clear();
		m_sections.clear();

		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
		glUseProgram(NULL);

		SDL_GL_SwapWindow(window);
	}

	void Renderer::draw_rect_fill(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec3 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = top_left.x;
		float y0 = top_left.y;
		float x1 = bottom_right.x;
		float y1 = bottom_right.y;

		// first triangle
		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color, .uv = { 0.0f, 1.0f } });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color, .uv = { 0.0f, 0.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color, .uv = { 1.0f, 1.0f } });

		// second triangle
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color, .uv = { 0.0f, 0.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color, .uv = { 1.0f, 1.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color, .uv = { 1.0f, 0.0f } });

		// sections
		m_sections.push_back(VertexSection { .primitive = Primitive::Triangle, .length = 6, .texture = m_white_texture });
	}

} // namespace platform

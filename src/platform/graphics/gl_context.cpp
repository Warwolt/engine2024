#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/graphics/gl_context.h>

#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/graphics/vertex.h>

namespace platform {

	static int _wrapping_mode_to_gl_int(TextureWrapping wrapping) {
		switch (wrapping) {
			case TextureWrapping::Repeat:
				return GL_REPEAT;

			case TextureWrapping::MirroredRepeat:
				return GL_MIRRORED_REPEAT;

			case TextureWrapping::ClampToEdge:
				return GL_CLAMP_TO_EDGE;

			case TextureWrapping::ClampToBorder:
				return GL_CLAMP_TO_BORDER;
		}
		return 0;
	}

	static int _filter_mode_to_gl_int(TextureFilter filter) {
		switch (filter) {
			case TextureFilter::Linear:
				return GL_LINEAR;

			case TextureFilter::Nearest:
				return GL_NEAREST;
		}
		return 0;
	}

	Texture OpenGLContext::add_texture(
		const unsigned char* data,
		int width,
		int height,
		TextureWrapping wrapping,
		TextureFilter filter
	) {
		GLuint texture_id;
		glGenTextures(1, &texture_id);

		Texture texture = Texture { texture_id, glm::vec2 { width, height } };
		glBindTexture(GL_TEXTURE_2D, texture_id);

		set_texture_filter(texture, filter);
		set_texture_wrapping(texture, wrapping);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, NULL);
		return texture;
	}

	void OpenGLContext::set_texture_wrapping(Texture texture, TextureWrapping wrapping) {
		const int wrapping_int = _wrapping_mode_to_gl_int(wrapping);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping_int);
	}

	void OpenGLContext::set_texture_filter(Texture texture, TextureFilter filter) {
		const int filter_int = _filter_mode_to_gl_int(filter);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_int);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_int);
	}

	void OpenGLContext::free_texture(Texture texture) {
		glDeleteTextures(1, &texture.id);
	}

	Canvas OpenGLContext::add_canvas(int width, int height, TextureWrapping wrapping, TextureFilter filter) {
		// create texture
		GLuint texture_id;
		glGenTextures(1, &texture_id);
		Texture texture = Texture { texture_id, glm::vec2 { width, height } };
		glBindTexture(GL_TEXTURE_2D, texture_id);

		// create buffer
		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		set_texture_filter(texture, filter);
		set_texture_wrapping(texture, wrapping);

		// attach texture to buffer and draw buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

		GLuint render_buffer;
		glGenRenderbuffers(1, &render_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		glRenderbufferStorage(
			GL_RENDERBUFFER,
			GL_DEPTH24_STENCIL8,
			width,
			height
		);

		glBindTexture(GL_TEXTURE_2D, NULL);
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);

		return Canvas { framebuffer, texture };
	}

	void OpenGLContext::free_canvas(Canvas canvas) {
		glDeleteFramebuffers(1, &canvas.framebuffer);
		glDeleteTextures(1, &canvas.texture.id);
	}

	std::expected<ShaderProgram, ShaderProgramError> OpenGLContext::add_shader_program(const char* vertex_src, const char* fragment_src) {
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

		/* Load locations */
		GLint projection_uniform = glGetUniformLocation(shader_program_id, "projection");

		/* Unbind */
		glUseProgram(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);

		return ShaderProgram {
			.id = shader_program_id,
			.vao = vao,
			.vbo = vbo,
			.uniforms {
				.projection = projection_uniform,
			},
		};
	}

	void OpenGLContext::free_shader_program(const ShaderProgram& shader_program) {
		glDeleteVertexArrays(1, &shader_program.vao);
		glDeleteBuffers(1, &shader_program.vbo);
		glDeleteProgram(shader_program.id);
	}

} // namespace platform

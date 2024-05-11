#pragma once

#include <platform/image.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>

#include <expected>
#include <vector>

namespace platform {

	struct Vertex {
		glm::vec2 pos;
		glm::vec4 color;
		glm::vec2 uv;
	};

	struct Texture {
		GLuint id;
		int width;
		int height;
	};

	struct Canvas {
		GLuint frame_buffer;
		Texture texture;
	};

	struct VertexSection {
		GLenum mode;
		GLsizei length;
		Texture texture;
	};

	struct ShaderProgram {
		GLuint id;
		GLuint vao;
		GLuint vbo;
	};

	struct Rect {
		glm::vec2 top_left;
		glm::vec2 bottom_right;
	};

	enum class ShaderProgramError {
		VertexShaderFailedToCompile,
		FragmentShaderFailedToCompile,
		ShaderProgramFailedToLink,
	};

	std::expected<ShaderProgram, ShaderProgramError> add_shader_program(const char* vertex_src, const char* fragment_src);
	void free_shader_program(ShaderProgram shader_program);

	Texture add_texture(const unsigned char* data, int width, int height);
	void free_texture(Texture texture);

	Canvas add_canvas(int width, int height);
	void free_canvas(Canvas canvas);

	class Renderer {
	public:
		Renderer(SDL_GLContext gl_context);

		void set_projection(ShaderProgram shader_program, glm::mat4 projection);

		void render_to_canvas(ShaderProgram shader_program, Canvas canvas);
		void render(ShaderProgram shader_program);

		void draw_point(glm::vec2 point, glm::vec4 color);
		void draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
		void draw_rect(Rect rect, glm::vec4 color);
		void draw_rect_fill(Rect rect, glm::vec4 color);
		void draw_circle(glm::vec2 center, float radius, glm::vec4 color);
		void draw_circle_fill(glm::vec2 center, float radius, glm::vec4 color);

		void draw_texture(Texture texture, Rect rect);

	private:
		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
		Texture m_white_texture;
	};

} // namespace platform

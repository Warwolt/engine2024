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
		Renderer(SDL_GLContext gl_context, int canvas_width, int canvas_height);

		void set_projection(ShaderProgram shader_program, glm::mat4 projection);
		void set_canvas_size(float width, float height);
		glm::vec2 canvas_size() const;

		void render_to_window(ShaderProgram shader_program, SDL_Window* window);
		void render_to_canvas(ShaderProgram shader_program, Canvas canvas);

		void draw_point(glm::vec2 point, glm::vec4 color);
		void draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
		void draw_rect(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color);
		void draw_rect_fill(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color);
		void draw_circle(glm::vec2 center, float radius, glm::vec4 color);
		void draw_circle_fill(glm::vec2 center, float radius, glm::vec4 color);
		void draw_texture(glm::vec2 top_left, glm::vec2 bottom_right, Texture texture);

	private:
		void _render(ShaderProgram shader_program);

		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
		Texture m_white_texture;
		glm::vec2 m_canvas_size;
	};

} // namespace platform

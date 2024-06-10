#pragma once

#include <platform/font.h>
#include <platform/image.h>
#include <platform/rect.h>
#include <platform/texture.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>

#include <expected>
#include <optional>
#include <vector>

namespace platform {

	struct Vertex {
		glm::vec2 pos;
		glm::vec4 color;
		glm::vec2 uv;
	};

	struct Canvas {
		GLuint frame_buffer;
		Texture texture;
	};

	struct VertexSection {
		GLenum mode;
		GLsizei length;
		Texture texture;
		std::optional<Canvas> canvas;
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

	Canvas add_canvas(int width, int height);
	void free_canvas(Canvas canvas);

	class Renderer {
	public:
		Renderer(SDL_GLContext gl_context);

		void set_projection(ShaderProgram shader_program, glm::mat4 projection);

		void set_draw_canvas(Canvas canvas);
		void reset_draw_canvas();

		void set_render_canvas(Canvas canvas);
		void reset_render_canvas();

		void render(ShaderProgram shader_program);

		void draw_point(glm::vec2 point, glm::vec4 color);
		void draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
		void draw_rect(Rect quad, glm::vec4 color);
		void draw_rect_fill(Rect quad, glm::vec4 color);
		void draw_circle(glm::vec2 center, float radius, glm::vec4 color);
		void draw_circle_fill(glm::vec2 center, float radius, glm::vec4 color);

		void draw_texture(Texture texture, Rect quad);
		void draw_texture_clipped(Texture texture, Rect quad, FlipRect uv);
		void draw_texture_clipped_with_color(Texture texture, Rect quad, FlipRect uv, glm::vec4 color);

		void draw_character(const Font* font, char character, glm::vec2 pos, glm::vec4 color);
		void draw_text(const Font* font, const char* text, glm::vec2 pos, glm::vec4 color);
		void draw_text_centered(const Font* font, const char* text, glm::vec2 pos, glm::vec4 color);


	private:
		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
		Texture m_white_texture;
		std::optional<Canvas> m_draw_canvas;
		std::optional<Canvas> m_render_canvas;
	};

} // namespace platform

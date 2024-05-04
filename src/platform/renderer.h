#pragma once

#include <platform/image.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <expected>
#include <vector>

namespace platform {

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
		glm::vec2 uv;
	};

	enum class Primitive {
		Point,
		Line,
		Triangle,
	};

	GLenum primitive_to_draw_array_mode(Primitive primitive);

	struct VertexSection {
		Primitive primitive;
		uint16_t length;
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

	struct Texture {
		GLuint id;
	};

	class Renderer {
	public:
		Renderer(SDL_GLContext gl_context);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		std::expected<ShaderProgram, ShaderProgramError> add_program(const char* vertex_src, const char* fragment_src);
		Texture add_texture(const unsigned char* data, int width, int height);
		void render(SDL_Window* window, ShaderProgram shader_program);

		void draw_rect_fill(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec3 color);

	private:
		std::vector<ShaderProgram> m_shader_programs;
		std::vector<Texture> m_textures;
		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
		Texture m_white_texture;
	};

} // namespace platform

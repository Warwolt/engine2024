#pragma once

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

	enum class ShaderProgramError {
		VertexShaderFailedToCompile,
		FragmentShaderFailedToCompile,
		ShaderProgramFailedToLink,
	};

	const char* shader_program_error_to_string(ShaderProgramError error);

	struct ShaderProgram {
		GLuint id;
		GLuint vao;
		GLuint vbo;
	};

	class DrawData {
	public:
		void clear();
		void draw_rect_fill(glm::vec2 p0, glm::vec2 p1, glm::vec3 color);

		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
	};

	class Renderer {
	public:
		Renderer(SDL_Window* window);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		std::expected<ShaderProgram, ShaderProgramError> add_program(const char* vertex_src, const char* fragment_src);
		void render(SDL_Window* window, ShaderProgram shader_program, const DrawData* draw_data);

	private:
		SDL_GLContext m_gl_context = nullptr;
		std::vector<ShaderProgram> m_shader_programs;
	};

} // namespace platform

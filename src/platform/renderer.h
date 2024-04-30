#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <expected>
#include <vector>

namespace platform {

	struct Vertex {
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
		// draw_rect(p0, p1, color)

		std::vector<Vertex> vertices;
		std::vector<VertexSection> sections;
	};

	class Renderer {
	public:
		Renderer(SDL_Window* window);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		std::expected<ShaderProgram, ShaderProgramError> add_program(const char* vertex_src, const char* fragment_src);
		void render(SDL_Window* window, ShaderProgram shader_program, const DrawData* draw_data);

		SDL_GLContext m_gl_context = nullptr;
		std::vector<ShaderProgram> m_shader_programs;
	};

} // namespace platform

#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/renderer.h>

#include <platform/logging.h>
#include <stb_image/stb_image.h>

#include <algorithm>
#include <math.h>
#include <set>
#include <vector>

#include <platform/assert.h> // DEBUGGING
#include <platform/logging.h> // DEBUGGING

namespace platform {

	static std::vector<glm::vec2> circle_octant_points(float radius) {
		/* Compute points in first octant */
		//              90°
		//         , - ~ ~ ~ - ,
		//     , '       |       ' , 45°
		//   ,           |       ⟋   ,
		//  ,            |    ⟋       ,
		// ,             | ⟋           ,
		// ,             o             ,
		// ,                           ,
		//  ,                         ,
		//   ,                       ,
		//     ,                  , '
		//       ' - , _ _ _ ,  '
		std::vector<glm::vec2> quadrant_points;
		{
			glm::vec2 point = { 0.0f, radius };
			while (point.x <= point.y) {
				quadrant_points.push_back(point);

				glm::vec2 mid_point = { point.x + 1, point.y - 0.5f };
				if (pow(mid_point.x, 2) + pow(mid_point.y, 2) > pow(radius, 2)) {
					point.y -= 1;
				}
				point.x += 1;
			}
		}
		return quadrant_points;
	}

	Texture add_texture(const unsigned char* data, int width, int height) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // is this really needed?

		glBindTexture(GL_TEXTURE_2D, NULL);
		return Texture { texture };
	}

	void free_texture(Texture texture) {
		glDeleteTextures(1, &texture.id);
	}

	std::expected<ShaderProgram, ShaderProgramError> add_shader_program(const char* vertex_src, const char* fragment_src) {
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
		glUseProgram(NULL);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);

		return ShaderProgram { shader_program_id, vao, vbo };
	}

	void free_shader_program(ShaderProgram shader_program) {
		glDeleteVertexArrays(1, &shader_program.vao);
		glDeleteBuffers(1, &shader_program.vbo);
		glDeleteProgram(shader_program.id);
	}

	Renderer::Renderer(SDL_GLContext /* gl_context */) {
		unsigned char data[] = { 0xFF, 0xFF, 0xFF };
		m_white_texture = add_texture(data, 1, 1);
	}

	void Renderer::set_projection(ShaderProgram shader_program, glm::mat4 projection) {
		glUseProgram(shader_program.id);
		GLint projection_uniform = glGetUniformLocation(shader_program.id, "projection");
		glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &projection[0][0]);
	}

	void Renderer::render(SDL_Window* window, ShaderProgram shader_program) {
		glUseProgram(shader_program.id);
		glBindVertexArray(shader_program.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);

		/* Clear screen */
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		/* Upload vertices */
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

		/* Draw vertices */
		GLint offset = 0;
		for (const VertexSection& section : m_sections) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, section.texture.id);
			glDrawArrays(section.mode, offset, section.length);

			offset += section.length;
		}
		SDL_GL_SwapWindow(window);

		/* Clear render data */
		m_vertices.clear();
		m_sections.clear();

		/* Unbind */
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
		glUseProgram(NULL);
	}

	void Renderer::draw_point(glm::vec2 point, glm::vec4 color) {
		m_vertices.push_back(Vertex { .pos = point, .color = color });
		m_sections.push_back(VertexSection { .mode = GL_POINTS, .length = 1, .texture = m_white_texture });
	}

	void Renderer::draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color) {
		m_vertices.push_back(Vertex { .pos = start, .color = color });
		m_vertices.push_back(Vertex { .pos = end, .color = color });
		m_sections.push_back(VertexSection { .mode = GL_LINES, .length = 2, .texture = m_white_texture });
	}

	void Renderer::draw_rect(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = top_left.x;
		float y0 = top_left.y;
		float x1 = bottom_right.x;
		float y1 = bottom_right.y;

		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });

		m_sections.push_back(VertexSection { .mode = GL_LINE_LOOP, .length = 4, .texture = m_white_texture });
	}

	void Renderer::draw_rect_fill(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = top_left.x;
		float y0 = top_left.y;
		float x1 = bottom_right.x;
		float y1 = bottom_right.y;

		// first triangle
		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });

		// second triangle
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color });

		// sections
		m_sections.push_back(VertexSection { .mode = GL_TRIANGLES, .length = 6, .texture = m_white_texture });
	}

	void Renderer::draw_circle(glm::vec2 center, float radius, glm::vec4 color) {
		std::vector<glm::vec2> quadrant_points = circle_octant_points(radius);
		for (const glm::vec2& point : quadrant_points) {
			float x = point.x;
			float y = point.y;
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { x, y }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { y, x }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { y, -x }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { x, -y }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { -x, -y }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { -y, -x }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { -y, x }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { -x, y }, .color = color });
		}

		m_sections.push_back(VertexSection { .mode = GL_POINTS, .length = 8 * (GLsizei)quadrant_points.size(), .texture = m_white_texture });
	}

	void Renderer::draw_circle_fill(glm::vec2 center, float radius, glm::vec4 color) {
		/* Get points of upper half circle */
		std::vector<glm::vec2> quadrant_points = circle_octant_points(radius);
		std::vector<glm::vec2> half_circle_points;
		for (const glm::vec2& point : quadrant_points) {
			float x = point.x;
			float y = point.y;
			half_circle_points.push_back(glm::vec2 { x, y });
			half_circle_points.push_back(glm::vec2 { y, x });
			half_circle_points.push_back(glm::vec2 { -x, y });
			half_circle_points.push_back(glm::vec2 { -y, x });
		}

		/* Remove points with overlapping x-coordinates to avoid overdraw */
		{
			std::vector<glm::vec2>& p = half_circle_points;
			std::sort(p.begin(), p.end(), [](const glm::vec2& lhs, const glm::vec2& rhs) { return lhs.x < rhs.x; });
			p.erase(std::unique(p.begin(), p.end(), [](const glm::vec2& lhs, const glm::vec2& rhs) { return lhs.x == rhs.x; }), p.end());
		}

		/* Draw vertical lines */
		for (const glm::vec2& point : half_circle_points) {
			float x = point.x;
			float y = point.y;
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { x, y }, .color = color });
			m_vertices.push_back(Vertex { .pos = center + glm::vec2 { x, -y }, .color = color });
		}
		m_sections.push_back(VertexSection { .mode = GL_LINES, .length = 2 * (GLsizei)half_circle_points.size(), .texture = m_white_texture });
	}

	void Renderer::draw_texture(glm::vec2 top_left, glm::vec2 bottom_right, Texture texture) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |<
		// (x0, y1) ---- (x1, y1)
		float x0 = top_left.x;
		float y0 = top_left.y;
		float x1 = bottom_right.x;
		float y1 = bottom_right.y;

		glm::vec4 white = { 1.0f, 1.0f, 1.0f, 1.0f };

		// first triangle
		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = white, .uv = { 0.0f, 1.0f } });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = white, .uv = { 0.0f, 0.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = white, .uv = { 1.0f, 1.0f } });

		// second triangle
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = white, .uv = { 0.0f, 0.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = white, .uv = { 1.0f, 1.0f } });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = white, .uv = { 1.0f, 0.0f } });

		// sections
		m_sections.push_back(VertexSection { .mode = GL_TRIANGLES, .length = 6, .texture = texture });
	}

} // namespace platform

#include <GL/glew.h> // must be included before <SDL2/SDL_opengl.h>

#include <platform/graphics/renderer.h>

#include <glm/gtc/matrix_transform.hpp> // glm::ortho
#include <imgui/backends/imgui_impl_opengl3.h>
#include <platform/debug/logging.h>
#include <platform/graphics/gl_context.h>
#include <platform/input/timing.h>
#include <stb_image/stb_image.h>

#include <algorithm>
#include <math.h>
#include <set>
#include <vector>

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

	Renderer::Renderer(OpenGLContext* gl_context) {
		unsigned char data[] = { 0xFF, 0xFF, 0xFF, 0xFF };
		m_white_texture = gl_context->add_texture(data, 1, 1);
	}

	void Renderer::set_projection(const ShaderProgram& shader_program, glm::mat4 projection) {
		glUseProgram(shader_program.id);
		glUniformMatrix4fv(shader_program.uniforms.projection, 1, GL_FALSE, &projection[0][0]);
	}

	void Renderer::push_draw_canvas(Canvas canvas) {
		m_draw_canvas_stack.push_back(canvas);
	}
	void Renderer::pop_draw_canvas() {
		m_draw_canvas_stack.pop_back();
	}

	void Renderer::set_render_canvas(Canvas canvas) {
		m_render_canvas = canvas;
	}
	void Renderer::reset_render_canvas() {
		m_render_canvas = {};
	}

	static void set_pixel_coordinate_projection(Renderer* renderer, const ShaderProgram& shader_program, int width, int height) {
		float grid_offset = 0.375f; // used to avoid missing pixels
		glm::mat4 projection = glm::ortho(grid_offset, grid_offset + width, grid_offset + height, grid_offset, -1.0f, 1.0f);
		renderer->set_projection(shader_program, projection);
	}

	void Renderer::render(const ShaderProgram& shader_program) {
		m_debug_data = {};
		Timer render_timer;

		glUseProgram(shader_program.id);
		glBindVertexArray(shader_program.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shader_program.vbo);

		/* Upload vertices */
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
		m_debug_data.num_vertices = m_vertices.size();

		/* Draw vertices */
		GLint offset = 0;
		int i = 0;
		for (const VertexSection& section : m_sections) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, section.texture.id);

			std::optional<Canvas> canvas;
			if (section.canvas) {
				canvas = section.canvas;
			}
			else if (m_render_canvas) {
				canvas = m_render_canvas;
			}

			if (canvas) {
				glBindFramebuffer(GL_FRAMEBUFFER, canvas->framebuffer);
				glViewport(0, 0, (int)canvas->texture.size.x, (int)canvas->texture.size.y);
				set_pixel_coordinate_projection(this, shader_program, (int)canvas->texture.size.x, (int)canvas->texture.size.y);
			}

			m_debug_data.num_draw_calls += 1;
			glDrawArrays(section.mode, offset, section.length);
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);

			offset += section.length;
			i++;
		}

		/* Clear render data */
		m_vertices.clear();
		m_sections.clear();

		/* Unbind */
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindVertexArray(NULL);
		glUseProgram(NULL);

		m_debug_data.render_ms = render_timer.elapsed_ms();
		m_debug_data.render_ns = render_timer.elapsed_ns();
	}

	void Renderer::draw_point(glm::vec2 point, glm::vec4 color) {
		m_vertices.push_back(Vertex { .pos = point, .color = color });
		m_sections.push_back(VertexSection { .mode = GL_POINTS, .length = 1, .texture = m_white_texture, .canvas = _current_draw_canvas() });
	}

	void Renderer::draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color) {
		m_vertices.push_back(Vertex { .pos = start, .color = color });
		m_vertices.push_back(Vertex { .pos = end, .color = color });
		m_sections.push_back(VertexSection { .mode = GL_LINES, .length = 2, .texture = m_white_texture, .canvas = _current_draw_canvas() });
	}

	void Renderer::draw_rect(core::Rect quad, glm::vec4 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = quad.top_left.x;
		float y0 = quad.top_left.y;
		float x1 = quad.bottom_right.x;
		float y1 = quad.bottom_right.y;

		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });

		m_sections.push_back(VertexSection { .mode = GL_LINE_LOOP, .length = 4, .texture = m_white_texture, .canvas = _current_draw_canvas() });
	}

	void Renderer::draw_rect_fill(core::Rect quad, glm::vec4 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = quad.top_left.x;
		float y0 = quad.top_left.y;
		float x1 = quad.bottom_right.x;
		float y1 = quad.bottom_right.y;

		// first triangle
		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });

		// second triangle
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color });

		// sections
		m_sections.push_back(VertexSection { .mode = GL_TRIANGLES, .length = 6, .texture = m_white_texture, .canvas = _current_draw_canvas() });
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

		m_sections.push_back(VertexSection { .mode = GL_POINTS, .length = 8 * (GLsizei)quadrant_points.size(), .texture = m_white_texture, .canvas = _current_draw_canvas() });
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
		m_sections.push_back(VertexSection { .mode = GL_LINES, .length = 2 * (GLsizei)half_circle_points.size(), .texture = m_white_texture, .canvas = _current_draw_canvas() });
	}

	void Renderer::draw_texture(Texture texture, core::Rect quad) {
		core::FlipRect uv = {
			.bottom_left = { 0.0f, 0.0f },
			.top_right = { 1.0f, 1.0f }
		};
		draw_texture_clipped(texture, quad, uv);
	}

	void Renderer::draw_texture_clipped(Texture texture, core::Rect quad, core::FlipRect uv) {
		glm::vec4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
		draw_texture_clipped_with_color(texture, quad, uv, white);
	}

	void Renderer::draw_texture_clipped_with_color(Texture texture, core::Rect quad, core::FlipRect uv, glm::vec4 color) {
		// (x0, y0) ---- (x1, y0)
		//     |            |
		//     |            |
		// (x0, y1) ---- (x1, y1)
		float x0 = quad.top_left.x;
		float y0 = quad.top_left.y;
		float x1 = quad.bottom_right.x;
		float y1 = quad.bottom_right.y;

		// (u0, v1) ---- (u1, v1)
		//     |            |
		//     |            |
		// (u0, v0) ---- (u1, v0)
		float u0 = uv.bottom_left.x;
		float v0 = uv.bottom_left.y;
		float u1 = uv.top_right.x;
		float v1 = uv.top_right.y;

		// first triangle
		m_vertices.push_back(Vertex { .pos = { x0, y0 }, .color = color, .uv = { u0, v1 } });
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color, .uv = { u0, v0 } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color, .uv = { u1, v1 } });

		// second triangle
		m_vertices.push_back(Vertex { .pos = { x0, y1 }, .color = color, .uv = { u0, v0 } });
		m_vertices.push_back(Vertex { .pos = { x1, y0 }, .color = color, .uv = { u1, v1 } });
		m_vertices.push_back(Vertex { .pos = { x1, y1 }, .color = color, .uv = { u1, v0 } });

		// sections
		m_sections.push_back(VertexSection { .mode = GL_TRIANGLES, .length = 6, .texture = texture, .canvas = _current_draw_canvas() });
	}

	void Renderer::draw_character(const Font& font, char character, glm::vec2 pos, glm::vec4 color) {
		const platform::Glyph& glyph = font.glyphs[character];

		core::Rect quad = {
			.top_left = { pos.x, pos.y },
			.bottom_right = { pos.x + glyph.size.x, pos.y + glyph.size.y }
		};

		float u0 = glyph.atlas_pos.x / (float)font.atlas.size.x;
		float v0 = 1 - (glyph.atlas_pos.y + glyph.size.y) / (float)font.atlas.size.y;
		float u1 = u0 + glyph.size.x / (float)font.atlas.size.x;
		float v1 = v0 + glyph.size.y / (float)font.atlas.size.y;

		core::FlipRect uv = {
			.bottom_left = { u0, v0 },
			.top_right = { u1, v1 }
		};

		draw_texture_clipped_with_color(font.atlas, quad, uv, color);
	}

	void Renderer::draw_text(const Font& font, const std::string& text, glm::vec2 pos, glm::vec4 color) {
		glm::vec2 pen = pos;
		const char* chars = text.data();
		for (char character = *chars; character != '\0'; character = *(++chars)) {
			const platform::Glyph& glyph = font.glyphs[character];

			if (character != ' ') {
				glm::vec2 glyph_pos = glm::vec2 {
					pen.x + glyph.bearing.x,
					pen.y - glyph.bearing.y,
				};
				draw_character(font, character, glyph_pos, color);
			}

			pen.x += glyph.advance;
		}
	}

	void Renderer::draw_text_centered(const Font& font, const std::string& text, glm::vec2 pos, glm::vec4 color) {
		glm::vec2 box_size = { 0.0f, 0.0f };
		box_size.y = (float)font.line_height;

		const char* chars = text.data();
		for (char character = *chars; character != '\0'; character = *(++chars)) {
			const platform::Glyph& glyph = font.glyphs[character];
			box_size.x += glyph.advance;
		}

		draw_text(font, text, pos - box_size / 2.0f, color);
	}

	RenderDebugData Renderer::debug_data() const {
		return m_debug_data;
	}

	std::optional<Canvas> Renderer::_current_draw_canvas() {
		return m_draw_canvas_stack.empty() ? std::nullopt : std::make_optional(m_draw_canvas_stack.back());
	}

} // namespace platform

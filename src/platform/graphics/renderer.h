#pragma once

#include <core/rect.h>
#include <platform/graphics/canvas.h>
#include <platform/graphics/color.h>
#include <platform/graphics/font.h>
#include <platform/graphics/image.h>
#include <platform/graphics/renderer_debug.h>
#include <platform/graphics/shader_program.h>
#include <platform/graphics/texture.h>
#include <platform/graphics/vertex.h>

#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <vector>

namespace platform {

	class GraphicsContext;

	class Renderer {
	public:
		Renderer(GraphicsContext* graphics);

		void set_projection(const ShaderProgram& shader_program, glm::mat4 projection);

		void push_draw_canvas(Canvas canvas);
		void pop_draw_canvas();

		void set_render_canvas(Canvas canvas);
		void reset_render_canvas();

		void render(const ShaderProgram& shader_program);

		void draw_point(glm::vec2 point, glm::vec4 color);
		void draw_line(glm::vec2 start, glm::vec2 end, glm::vec4 color);
		void draw_rect(core::Rect quad, glm::vec4 color);
		void draw_rect_fill(core::Rect quad, glm::vec4 color);
		void draw_circle(glm::vec2 center, float radius, glm::vec4 color);
		void draw_circle_fill(glm::vec2 center, float radius, glm::vec4 color);

		void draw_texture(Texture texture, core::Rect quad);
		void draw_texture_clipped(Texture texture, core::Rect quad, core::FlipRect uv);
		void draw_texture_clipped_with_color(Texture texture, core::Rect quad, core::FlipRect uv, glm::vec4 color);

		void draw_character(const Font& font, char character, glm::vec2 pos, glm::vec4 color);
		void draw_text(const Font& font, const std::string& text, glm::vec2 pos, glm::vec4 color);
		void draw_text_centered(const Font& font, const std::string& text, glm::vec2 pos, glm::vec4 color);

		RenderDebugData debug_data() const;

	private:
		struct VertexSection {
			GLenum mode;
			GLsizei length;
			Texture texture;
			std::optional<Canvas> canvas;
		};

		std::optional<Canvas> _current_draw_canvas();

		std::vector<Vertex> m_vertices;
		std::vector<VertexSection> m_sections;
		Texture m_white_texture;
		std::vector<Canvas> m_draw_canvas_stack;
		std::optional<Canvas> m_render_canvas;
		RenderDebugData m_debug_data;
	};

} // namespace platform

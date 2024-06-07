#include <engine/editor.h>

#include <platform/input/input.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>

#include <algorithm>

namespace engine {

	constexpr int NUM_ZOOM_MULTIPLES = 26;
	constexpr float zoom_multiples[NUM_ZOOM_MULTIPLES] = {
		0.016f,
		0.021f,
		0.031f,
		0.042f,
		0.062f,
		0.083f,
		0.125f,
		0.16f,
		0.20f,
		0.25f,
		0.33f,
		0.50f,
		1.0f,
		2.0f,
		3.0f,
		4.0f,
		5.0f,
		6.0f,
		7.0f,
		8.0f,
		12.0f,
		16.0f,
		24.0f,
		32.0f,
		48.0f,
		64.0f,
	};

	EditorState init_editor() {
		static_assert(zoom_multiples[12] == 1.0f);
		return EditorState {
			.zoom_index = 12,
			.zoom = 1.0,
		};
	}

	void update_editor(EditorState* editor, const platform::Input* input, platform::PlatformAPI* platform) {
		/* Zoom*/
		{
			editor->zoom_index = std::clamp<int>(editor->zoom_index + input->mouse.scroll_delta, 0, NUM_ZOOM_MULTIPLES - 1);
			editor->zoom = zoom_multiples[editor->zoom_index];
		}

		/* Mouse drag*/
		{
			if (input->mouse.middle_button.pressed_now()) {
				platform->set_cursor(platform::Cursor::SizeAll);
			}

			if (input->mouse.middle_button.released_now()) {
				platform->set_cursor(platform::Cursor::Arrow);
			}

			if (input->mouse.middle_button.is_pressed()) {
				editor->canvas_pos += input->mouse.pos_delta;
			}
		}
	}

	void render_editor(platform::Renderer* renderer, const EditorState* editor, platform::Canvas canvas) {
		renderer->set_draw_canvas(canvas);

		{
			const glm::vec4 light_grey = glm::vec4 { 0.75f, 0.75f, 0.75f, 1.0f };
			const glm::vec4 dark_grey = glm::vec4 { 0.50f, 0.50f, 0.50f, 1.0f };
			constexpr int tile_size = 32;

			for (int x = 0; x < canvas.texture.size.x; x += tile_size) {
				for (int y = 0; y < canvas.texture.size.y; y += tile_size) {
					const glm::vec4 color = (x / tile_size) % 2 == (y / tile_size) % 2 ? light_grey : dark_grey;
					renderer->draw_rect_fill({ { x, y }, { x + tile_size, y + tile_size } }, color);
				}
			}

			/* Draw a circle on top of background */
			renderer->draw_circle_fill(canvas.texture.size / 2.0f + glm::vec2 { 0, 0 }, 64, glm::vec4 { 0.0f, 0.8f, 0.8f, 1.0f });
		}
		renderer->reset_draw_canvas();

		/* Render canvas to screen*/
		glm::vec2 top_left = editor->canvas_pos;
		platform::Rect canvas_rect = {
			.top_left = top_left,
			.bottom_right = top_left + canvas.texture.size * editor->zoom,
		};
		renderer->draw_texture(canvas.texture, canvas_rect);
		renderer->draw_rect({ canvas_rect.top_left, canvas_rect.bottom_right + glm::vec2 { 1.0f, 1.0f } }, glm::vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
	}

} // namespace engine

#include <engine/editor.h>

#include <platform/input/input.h>
#include <platform/logging.h>
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

	EditorState init_editor(glm::vec2 canvas_size) {
		static_assert(zoom_multiples[12] == 1.0f);
		return EditorState {
			.zoom_index = 12,
			.zoom = 1.0,
			.canvas_rect = {
				.top_left = { 0, 0 },
				.bottom_right = canvas_size,
			},
		};
	}

	void update_editor(EditorState* editor, const platform::Input* input, platform::PlatformAPI* platform) {
		/* Canvas*/
		{
			// Keep canvas the same size as the curernt resolution
			editor->canvas_rect.set_size(input->window_resolution * editor->zoom);
		}

		/* Zoom*/
		{
			const int new_zoom_index = std::clamp(editor->zoom_index + input->mouse.scroll_delta, 0, NUM_ZOOM_MULTIPLES - 1);
			const float new_zoom = zoom_multiples[new_zoom_index];
			const float zoom_scale = new_zoom / editor->zoom;
			const bool zoom_changed = new_zoom != editor->zoom;

			editor->zoom_index = new_zoom_index;
			editor->zoom = new_zoom;

			if (zoom_changed) {
				// Position the canvas so that the zoom happens around the cursor,
				// by keeping the relative distance from cursor to corner the same.
				//
				// +--------------+
				// |      |       |      +-------+
				// |      |       |      |   |   |      +---+
				// |------o       |  =>  |---o   |  =>  | o |
				// |              |      |       |      +---+
				// |              |      +-------+
				// |              |
				// +--------------+
				//
				// Clamp mouse position to canvas borders so that the canvas doesn't
				// "run away" outside the screen while zooming.
				const glm::vec2 clamped_mouse_pos = glm::vec2 {
					std::clamp(input->mouse.pos.x, editor->canvas_rect.top_left.x, editor->canvas_rect.bottom_right.x),
					std::clamp(input->mouse.pos.y, editor->canvas_rect.top_left.y, editor->canvas_rect.bottom_right.y),
				};
				const platform::Rect delta = editor->canvas_rect - clamped_mouse_pos;
				editor->canvas_rect.top_left = clamped_mouse_pos + zoom_scale * delta.top_left;
				editor->canvas_rect.bottom_right = clamped_mouse_pos + zoom_scale * delta.bottom_right;
			}
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
				editor->canvas_rect += input->mouse.pos_delta;
			}
		}
	}

	void render_editor(platform::Renderer* renderer, const EditorState* editor, platform::Canvas canvas) {
		/* Draw to canvas */
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
		{
			// canvas texture
			renderer->draw_texture(canvas.texture, editor->canvas_rect);

			// canvas outline
			renderer->draw_rect({ editor->canvas_rect.top_left, editor->canvas_rect.bottom_right + glm::vec2 { 1.0f, 1.0f } }, glm::vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
		}
	}

} // namespace engine

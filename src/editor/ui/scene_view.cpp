#include <editor/ui/scene_view.h>

#include <platform/input/input.h>

#include <algorithm>

namespace editor {

	void init_scene_view(SceneViewState* scene) {
		constexpr int canvas_width = 320;
		constexpr int canvas_height = 320;
		scene->canvas_size = { canvas_width, canvas_height };
		scene->scaled_canvas_rect = core::Rect { { 0, 0 }, { canvas_width, canvas_height } };
		scene->canvas = platform::add_canvas(canvas_width, canvas_height);
		scene->zoom_index = 0;
	}

	void shutdown_scene_view(const SceneViewState& scene) {
		platform::free_canvas(scene.canvas);
	}

	static void update_canvas_zoom(
		SceneViewState* scene,
		const platform::Input& input,
		glm::vec2 window_relative_mouse_pos
	) {
		constexpr int min_zoom = -12;
		constexpr int max_zoom = 12;
		const int new_zoom_index = std::clamp(scene->zoom_index + input.mouse.scroll_delta, min_zoom, max_zoom);
		const bool zoom_has_changed = scene->zoom_index != new_zoom_index;
		scene->zoom_index = new_zoom_index;

		const float scale_up_factors[max_zoom + 1] = {
			1.0f,
			2.0f,
			3.0f,
			4.0f,
			5.0f,
			6.0f,
			8.0f,
			12.0f,
			16.0f,
			24.0f,
			32.0f,
			48.0f,
			64.0f,
		};
		const float scale_down_factors[-min_zoom] = {
			0.5f,
			0.33f,
			0.25f,
			0.20f,
			0.167f,
			0.125f,
			0.083f,
			0.062f,
			0.042f,
			0.031f,
			0.021f,
			0.016f,
		};
		const float scale = new_zoom_index < 0 ? scale_down_factors[-new_zoom_index - 1] : scale_up_factors[new_zoom_index];

		/* Move canvas when zooming */
		if (zoom_has_changed) {
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
			const glm::vec2 clamped_mouse_position = glm::vec2 {
				std::clamp(window_relative_mouse_pos.x, scene->scaled_canvas_rect.top_left.x, scene->scaled_canvas_rect.bottom_right.x),
				std::clamp(window_relative_mouse_pos.y, scene->scaled_canvas_rect.top_left.y, scene->scaled_canvas_rect.bottom_right.y),
			};
			const glm::vec2 new_scaled_size = scene->canvas_size * scale;
			const glm::vec2 distance_to_top_left = clamped_mouse_position - scene->scaled_canvas_rect.top_left;
			const float relative_scale = new_scaled_size.x / scene->scaled_canvas_rect.size().x;
			scene->scaled_canvas_rect.set_size(new_scaled_size);
			scene->scaled_canvas_rect.set_position(clamped_mouse_position - relative_scale * distance_to_top_left);
		}
	}

	static float clamp_coordinate_to_fit_window(float coordinate, float window_size, float canvas_size) {
		// Clamp so that no side passes the middle of the window
		if (canvas_size / window_size > 0.5f) {
			return std::clamp(coordinate, window_size / 2.0f - canvas_size, window_size / 2.0f);
		}
		// Clamp so that no side passes outside the window
		else {
			return std::clamp(coordinate, 0.0f, window_size - canvas_size);
		}
	}

	static glm::vec2 clamp_position_to_fit_window(glm::vec2 position, glm::vec2 window_size, glm::vec2 canvas_size) {
		return glm::vec2 {
			clamp_coordinate_to_fit_window(position.x, window_size.x, canvas_size.x),
			clamp_coordinate_to_fit_window(position.y, window_size.y, canvas_size.y)
		};
	}

	static void update_canvas_mouse_drag(
		SceneViewState* scene_view,
		const platform::Input& input,
		core::Rect scene_window_rect,
		bool scene_window_is_hovered,
		std::vector<editor::EditorCommand>* commands
	) {
		/* Drag canvas with middle mouse wheel */
		if (scene_window_is_hovered) {
			if (input.mouse.middle_button.pressed_now()) {
				commands->push_back(editor::EditorCommand::SetCursorToSizeAll);
				scene_view->is_being_dragging = true;
			}
		}

		if (scene_view->is_being_dragging) {
			const glm::vec2 window_size = scene_window_rect.size();
			const glm::vec2 canvas_position = scene_view->scaled_canvas_rect.position();
			const glm::vec2 canvas_size = scene_view->scaled_canvas_rect.size();
			const glm::vec2 dragged_position = clamp_position_to_fit_window(canvas_position + input.mouse.pos_delta, window_size, canvas_size);

			scene_view->scaled_canvas_rect.set_position(dragged_position);

			if (input.mouse.middle_button.released_now()) {
				commands->push_back(editor::EditorCommand::SetCursorToArrow);
				scene_view->is_being_dragging = false;
			}
		}
	}

	void update_scene_view(
		SceneViewState* scene_view,
		const platform::Input& input,
		glm::vec2 window_relative_mouse_pos,
		core::Rect scene_window_rect,
		std::vector<editor::EditorCommand>* commands
	) {
		const bool scene_window_is_hovered = scene_window_rect.overlaps_point(input.mouse.pos);
		if (scene_window_is_hovered) {
			update_canvas_zoom(scene_view, input, window_relative_mouse_pos);
		}

		update_canvas_mouse_drag(scene_view, input, scene_window_rect, scene_window_is_hovered, commands);
	}

	void render_scene_view(
		const SceneViewState& scene_view,
		platform::Renderer* renderer
	) {
		const glm::vec2 scene_canvas_size = scene_view.canvas.texture.size;

		// Clear scene
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, scene_canvas_size }, { 0.75f, 0.75f, 0.75f, 1.0f });

		// Coordinate Axes
		renderer->draw_line({ 0.0f, scene_canvas_size.y / 2.0f }, { scene_canvas_size.x + 1.0f, scene_canvas_size.y / 2.0f }, glm::vec4 { 1.0f, 0.0f, 0.0f, 1.0f }); // horizontal
		renderer->draw_line({ scene_canvas_size.x / 2.0f, 0.0f }, { scene_canvas_size.x / 2.0f, scene_canvas_size.y + 1.0f }, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f }); // vertical
	}

} // namespace editor

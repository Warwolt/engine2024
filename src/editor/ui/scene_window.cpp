#include <editor/ui/scene_window.h>

#include <platform/input/input.h>

#include <imgui/imgui.h>

namespace editor {

	void init_scene_window(SceneWindowState* scene_window) {
		scene_window->position_initialized = false;
		scene_window->canvas = platform::add_canvas(1, 1);
	}

	void shutdown_scene_window(const SceneWindowState& scene_window) {
		platform::free_canvas(scene_window.canvas);
	}

	void update_scene_window(
		SceneWindowState* scene_window,
		const platform::Input& input,
		std::vector<EditorCommand>* commands
	) {
		const glm::vec2 scene_window_pos = ImGui::GetCursorScreenPos();
		const glm::vec2 window_relative_mouse_pos = input.mouse.pos - scene_window_pos;
		glm::vec2 scene_window_size = ImGui::GetContentRegionAvail();

		// Initialize scene view
		{
			// Size
			if (scene_window->canvas.texture.size != input.monitor_size) {
				platform::free_canvas(scene_window->canvas);
				scene_window->canvas = platform::add_canvas((int)input.monitor_size.x, (int)input.monitor_size.y);
			}

			// Position
			static int counter = 0; // imgui needs 1 frame before window sizes are correct
			if (!scene_window->position_initialized && counter++ > 0) {
				scene_window->position_initialized = true;
				// Place scene view in center of window
				scene_window->scene_view.scaled_canvas_rect.set_position((scene_window_size - scene_window->scene_view.scaled_canvas_rect.size()) / 2.0f);
			}
		}

		// Render scene texture
		{
			const platform::Texture& scene_texture = scene_window->canvas.texture;
			glm::vec2 top_left = { 0.0f, 1.0f };
			glm::vec2 bottom_right = {
				std::clamp(scene_window_size.x / scene_texture.size.x, 0.0f, 1.0f),
				std::clamp(1.0f - scene_window_size.y / scene_texture.size.y, 0.0f, 1.0f),
			};
			ImGui::Image(scene_texture.id, scene_window_size, top_left, bottom_right);
		}

		// Update scene view
		{
			const core::Rect scene_window_rect = core::Rect::with_pos_and_size(ImGui::GetWindowPos(), ImGui::GetWindowSize());
			update_scene_view(
				&scene_window->scene_view,
				input,
				window_relative_mouse_pos,
				scene_window_rect,
				commands
			);
		}
	}

	void render_scene_window(
		const SceneWindowState& scene_window,
		platform::Renderer* renderer
	) {
		// Only render scene if ImGui scene window open
		if (scene_window.is_visible) {
			/* Render scene canvas */
			renderer->set_draw_canvas(scene_window.scene_view.canvas);
			render_scene_view(scene_window.scene_view, renderer);
			renderer->reset_draw_canvas();

			/* Render scene canvas to imgui canvas */
			renderer->set_draw_canvas(scene_window.canvas);
			renderer->draw_rect_fill(core::Rect { glm::vec2 { 0.0f, 0.0f }, scene_window.canvas.texture.size }, glm::vec4 { 0.0f, 0.5f, 0.5f, 1.0f });
			renderer->draw_texture(scene_window.scene_view.canvas.texture, scene_window.scene_view.scaled_canvas_rect);
			renderer->reset_draw_canvas();
		}
	}

} // namespace editor

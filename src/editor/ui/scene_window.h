#pragma once

#include <editor/editor_command.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

namespace platform {
	struct Input;
}

namespace editor {

	struct SceneViewState {
		int zoom_index = 0;
		bool is_being_dragging = false;
		glm::vec2 canvas_size = { 0.0f, 0.0f };
		core::Rect scaled_canvas_rect;
		platform::Canvas canvas; // used to render the scene
	};

	struct SceneWindowState {
		bool is_visible = false;
		bool position_initialized = false; // used to center scene view once we know ImGui window size
		platform::Canvas canvas; // used to render ImGui::Image
		SceneViewState scene_view; // the content of the scene window, the scene itself
	};

	void init_scene_window(SceneWindowState* scene_window);
	void shutdown_scene_window(const SceneWindowState& scene_window);

	void update_scene_window(
		SceneWindowState* scene_window,
		const platform::Input& input,
		std::vector<EditorCommand>* commands
	);

	void render_scene_window(
		const SceneWindowState& scene_window,
		platform::Renderer* renderer
	);

} // namespace editor
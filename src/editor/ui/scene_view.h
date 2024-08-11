#pragma once

#include <editor/editor_command.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

namespace platform {
	struct Input;
}

namespace editor {

	struct SceneViewState {
		bool position_initialized = false; // used to center view once we know ImGui window size
		int zoom_index = 0;
		bool is_being_dragging = false;
		glm::vec2 canvas_size = { 0.0f, 0.0f };
		core::Rect scaled_canvas_rect;
		platform::Canvas canvas; // used to render the scene
	};

	void init_scene_view(SceneViewState* scene_view);
	void shutdown_scene_view(const SceneViewState& scene_view);

	void update_scene_view(
		SceneViewState* scene_view,
		const platform::Input& input,
		glm::vec2 window_relative_mouse_pos,
		core::Rect scene_window_rect,
		std::vector<editor::EditorCommand>* commands
	);

	void render_scene_view(
		const SceneViewState& scene_view,
		platform::Renderer* renderer
	);

} // namespace editor

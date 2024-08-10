#pragma once

#include <engine/editor/editor_command.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

namespace platform {

	struct Input;

}

namespace engine {

	struct EditorSceneViewState {
		int zoom_index = 0;
		bool is_being_dragging = false;
		glm::vec2 canvas_size = { 0.0f, 0.0f };
		platform::Rect scaled_canvas_rect;
		platform::Canvas canvas; // used to render the scene
	};

	void init_editor_scene_view(EditorSceneViewState* scene_view);
	void shutdown_editor_scene_view(const EditorSceneViewState& scene_view);

	std::vector<EditorCommand> update_editor_scene_view(
		EditorSceneViewState* scene_view,
		const platform::Input& input,
		glm::vec2 window_relative_mouse_pos,
		bool scene_window_is_hovered
	);

	void render_editor_scene_view(
		const EditorSceneViewState& scene_view,
		platform::Renderer* renderer
	);

} // namespace engine

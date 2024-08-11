#pragma once

#include <editor/ui/scene_view.h>
#include <platform/graphics/renderer.h>

namespace editor {

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

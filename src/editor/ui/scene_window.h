#pragma once

namespace editor {

	struct SceneWindowState {
		bool position_initialized = false; // used to center scene view once we know ImGui window size
	};

	void init_scene_window(SceneWindowState* scene_window);

	// void update_scene_window(
	// 	const platform::Input& input,
	// 	EditorUiState* ui,
	// 	std::vector<EditorCommand>* commands
	// );

} // namespace editor

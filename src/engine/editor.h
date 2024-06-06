#pragma once

namespace platform {
	struct Input;
}

namespace engine {

	struct EditorState {
		int zoom_index;
		float zoom;
	};

	EditorState init_editor();
	void update_editor(EditorState* editor, const platform::Input* input);

} // namespace engine

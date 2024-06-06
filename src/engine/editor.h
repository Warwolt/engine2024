#pragma once

namespace platform {
	struct Input;
}

namespace engine {

	struct EditorState {
		int zoom_index = 12;
		float zoom = 1.0;
	};

	void update_editor(EditorState* editor, const platform::Input* input);

} // namespace engine

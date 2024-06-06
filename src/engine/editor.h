#pragma once

namespace platform {
	struct Input;
	class Renderer;
	struct Canvas;
}

namespace engine {

	struct EditorState {
		int zoom_index;
		float zoom;
	};

	EditorState init_editor();
	void update_editor(EditorState* editor, const platform::Input* input);
	void render_editor(platform::Renderer* renderer, const EditorState* editor, platform::Canvas canvas);

} // namespace engine

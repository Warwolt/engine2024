#pragma once

#include <glm/vec2.hpp>

namespace platform {
	struct Input;
	class Renderer;
	struct Canvas;
}

namespace engine {

	struct EditorState {
		int zoom_index;
		float zoom;
		glm::vec2 canvas_pos;
	};

	EditorState init_editor();
	void update_editor(EditorState* editor, const platform::Input* input);
	void render_editor(platform::Renderer* renderer, const EditorState* editor, platform::Canvas canvas);

} // namespace engine

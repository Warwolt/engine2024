#pragma once

#include <glm/vec2.hpp>
#include <platform/rect.h>

namespace platform {
	struct Input;
	struct Canvas;
	class Renderer;
	class PlatformAPI;
}

namespace engine {

	struct EditorState {
		int zoom_index;
		float zoom;
		platform::Rect canvas;
	};

	EditorState init_editor(glm::vec2 canvas_size);
	void update_editor(EditorState* editor, const platform::Input* input, platform::PlatformAPI* platform);
	void render_editor(platform::Renderer* renderer, const EditorState* editor, platform::Canvas canvas);

} // namespace engine

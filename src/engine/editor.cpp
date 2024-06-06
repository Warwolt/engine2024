#include <engine/editor.h>

#include <platform/input/input.h>

#include <algorithm>

namespace engine {

	constexpr int NUM_ZOOM_MULTIPLES = 26;
	constexpr float zoom_multiples[NUM_ZOOM_MULTIPLES] = {
		0.016f,
		0.021f,
		0.031f,
		0.042f,
		0.062f,
		0.083f,
		0.125f,
		0.16f,
		0.20f,
		0.25f,
		0.33f,
		0.50f,
		1.0f,
		2.0f,
		3.0f,
		4.0f,
		5.0f,
		6.0f,
		7.0f,
		8.0f,
		12.0f,
		16.0f,
		24.0f,
		32.0f,
		48.0f,
		64.0f,
	};

	EditorState init_editor() {
		static_assert(zoom_multiples[12] == 1.0f);
		return EditorState {
			.zoom_index = 12,
			.zoom = 1.0,
		};
	}

	void update_editor(EditorState* editor, const platform::Input* input) {
		editor->zoom_index = std::clamp<int>(editor->zoom_index + input->mouse.scroll_delta, 0, NUM_ZOOM_MULTIPLES - 1);
		editor->zoom = zoom_multiples[editor->zoom_index];
	}

} // namespace engine

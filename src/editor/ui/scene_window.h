#pragma once

#include <editor/editor_command.h>
#include <engine/state/scene_graph.h>
#include <platform/graphics/font.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

namespace platform {
	struct Input;
}

namespace editor {

	struct EditorScene {
		int zoom_index = 0;
		bool is_being_dragging = false;
		glm::vec2 canvas_size = { 0.0f, 0.0f };
		core::Rect scaled_canvas_rect;
		platform::Canvas canvas; // used to render the scene
		platform::Canvas grid_canvas; // used to render grid
	};

	class SceneWindow {
	public:
		SceneWindow();
		~SceneWindow();

		void update(
			engine::SceneGraph* scene_graph,
			const platform::Input& input,
			std::vector<EditorCommand>* commands
		);

		void render(
			const engine::TextSystem& text_system,
			engine::FontID system_font_id,
			platform::Renderer* renderer
		) const;

	private:
		EditorScene m_scene; // the content of the scene window, the scene itself
		platform::Canvas m_canvas; // used to render ImGui::Image
		bool m_position_initialized = false; // used to center scene view once we know ImGui window size
	};

} // namespace editor

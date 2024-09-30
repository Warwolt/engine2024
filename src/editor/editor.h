#pragma once

#include <editor/editor_command.h>
#include <editor/ui/log_window.h>
#include <editor/ui/scene_graph_window.h>
#include <editor/ui/scene_window.h>
#include <engine/system/text_system.h>
#include <platform/platform_api.h>
#include <platform/graphics/graphics_api.h>

#include <vector>

namespace platform {
	class Renderer;
	struct Configuration;
	struct Input;
}

namespace engine {
	class Engine;
	struct GameState;
	struct ProjectState;
	struct Systems;
}

namespace editor {

	class Editor {
	public:
		Editor(engine::Engine* engine, const platform::Configuration& config);

		void update(
			const platform::Input& input,
			const platform::Configuration& config,
			engine::Engine* engine,
			platform::PlatformAPI* platform,
			platform::GraphicsAPI* graphics
		);

		void render(
			const engine::Engine& engine,
			platform::Renderer* renderer
		) const;

	private:
		std::vector<editor::EditorCommand> _update_ui(
			const platform::Input& input,
			bool unsaved_changes,
			engine::Engine* engine
		);

		size_t m_project_hash; // for "unsaved changes" prompts
		bool m_show_imgui_demo = false;
		engine::FontID m_system_font_id;

		LogWindow m_log_window;
		SceneGraphWindow m_scene_graph_window;
		SceneWindow m_scene_window;
	};

} // namespace editor

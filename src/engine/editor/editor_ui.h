#pragma once

#include <engine/editor/editor_command.h>
#include <platform/graphics/renderer.h>

#include <glm/vec2.hpp>

#include <stdint.h>
#include <string>
#include <vector>

namespace platform {

	struct Input;

} // namespace platform

namespace engine {

	struct GameState;
	struct ProjectState;
	struct Resources;

	struct EditorUiState {
		size_t cached_project_hash; // for "unsaved changes" prompts
		std::string project_name_buf;
		bool run_game_windowed = false;
		bool show_imgui_demo = false;

		platform::Canvas window_canvas; // used to render ImGui::Image
		glm::vec2 scene_window_size; // size of imgui window that renders the scene
		bool scene_window_hovered = false;

		int scene_zoom_index = 0;
		glm::vec2 scene_canvas_size = { 0.0f, 0.0f };
		platform::Rect scaled_scene_canvas_rect;
		platform::Canvas scene_canvas; // used to render the scene
	};

	void init_editor_ui(
		EditorUiState* ui,
		const ProjectState& project,
		bool reset_docking
	);

	void shutdown_editor_ui(const EditorUiState& ui);

	std::vector<EditorCommand> update_editor_ui(
		EditorUiState* ui,
		GameState* game,
		ProjectState* project,
		const platform::Input& input,
		const engine::Resources& resources,
		bool unsaved_changes,
		bool game_is_running
	);

	void render_editor_ui(
		const EditorUiState& ui,
		const engine::Resources& resources,
		platform::Renderer* renderer
	);

} // namespace engine

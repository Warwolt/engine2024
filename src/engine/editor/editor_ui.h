#pragma once

#include <editor/editor_command.h>
#include <engine/editor/editor_scene_view.h>
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
		bool scene_window_visible = false;
		bool scene_view_position_initialized = false; // used to center view once we know ImGui window size

		EditorSceneViewState scene_view;
	};

	void init_editor_ui(
		EditorUiState* ui,
		const ProjectState& project,
		bool reset_docking
	);

	void shutdown_editor_ui(const EditorUiState& ui);

	std::vector<editor::EditorCommand> update_editor_ui(
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
		platform::Renderer* renderer
	);

} // namespace engine

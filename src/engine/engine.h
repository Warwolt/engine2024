#pragma once

#include <core/container/ring_buffer.h>
#include <editor/editor.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <engine/system/animation.h>
#include <engine/system/hot_reloading.h>
#include <engine/system/text_system.h>
#include <platform/graphics/font.h>
#include <platform/graphics/renderer.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace engine {

	struct DebugUiState {
		bool show_debug_ui = false;
		int resolution_index = 0;

		// frame rate measurement
		uint64_t second_counter_ms = 0;
		core::RingBuffer<float, 60> frame_render_deltas;
		float render_delta_avg_ms = 0.0f;
	};

	struct Systems {
		AnimationSystem animation;
		TextSystem text;
	};

	class EngineState {
	public:
		void load_project(const char* path);

		// TODO move all this to private
		Systems systems;
		SceneGraph scene_graph; // <-- at some point this should be a stack
		bool editor_is_running;
		glm::vec2 window_resolution;
		DebugUiState debug_ui;
		HotReloadingState hot_reloading;
		ProjectState project;
		GameState game;
		editor::EditorState editor;

	private:
	};

} // namespace engine

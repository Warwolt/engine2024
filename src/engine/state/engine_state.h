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

	// NOTE: Very likely that this struct won't be needed as we move stuff into
	// system-classes instead, since the systems should handle resource ownership.
	struct Resources {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
		std::unordered_map<std::string, platform::Canvas> canvases;
	};

	struct Systems {
		AnimationSystem animation;
		TextSystem text;
	};

	struct State {
		Resources resources;
		Systems systems;
		SceneGraph scene_graph; // <-- at some point this should be a stack
		bool editor_is_running;
		glm::vec2 window_resolution;
		DebugUiState debug_ui;
		HotReloadingState hot_reloading;
		ProjectState project;
		GameState game;
		editor::EditorState editor;
	};

} // namespace engine

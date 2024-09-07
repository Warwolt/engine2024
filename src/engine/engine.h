#pragma once

#include <core/container/ring_buffer.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <engine/state/scene_graph.h>
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

namespace platform {
	struct Input;
	struct Configuration;
	class PlatformAPI;
	class Renderer;
}

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

	class Engine {
	public:
		Engine() = default;
		explicit Engine(const platform::Configuration* config);

		void load_data(const char* path);
		void update(const platform::Input& input, platform::PlatformAPI* platform);
		void render(platform::Renderer* renderer) const;

	private:
		void _render_game(platform::Renderer* renderer) const;

		Systems m_systems;
		SceneGraph m_scene_graph; // <-- at some point this should be a stack
		bool m_game_is_running;
		glm::vec2 m_window_resolution;
		DebugUiState m_debug_ui;
		HotReloadingState m_hot_reloading;
		ProjectState m_project;
		GameState m_game;
	};

} // namespace engine

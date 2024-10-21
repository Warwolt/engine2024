#pragma once

#include <core/container/ring_buffer.h>
#include <engine/state/project_state.h>
#include <engine/state/scene_graph.h>
#include <engine/system/hot_reloading.h>
#include <engine/system/text_system.h>
#include <engine/system/timeline_system.h>
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
	class OpenGLContext;
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

	// TODO: move this out into its own header
	struct Systems {
		TimelineSystem timeline;
		TextSystem text;

		void reset() {
			// placeholder, right now no system has state that needs resetting
			// when starting/stopping game but we want this in place anyway
		}
	};

	class Engine {
	public:
		Engine(platform::OpenGLContext* gl_context);

		void load_data(const char* path);
		void update(const platform::Input& input, platform::PlatformAPI* platform, platform::OpenGLContext* gl_context);
		void render(platform::Renderer* renderer) const;
		void shutdown(platform::OpenGLContext* gl_context);

		SceneGraph& scene_graph() { return m_scene_graph; }
		const SceneGraph& scene_graph() const { return m_scene_graph; }

		Systems& systems() { return m_systems; }
		const Systems& systems() const { return m_systems; }

		ProjectState& project() { return m_project; }
		const ProjectState& project() const { return m_project; }

	private:
		Systems m_systems;
		SceneGraph m_scene_graph; // <-- at some point this should be a stack
		bool m_game_is_running;
		glm::vec2 m_window_resolution;
		DebugUiState m_debug_ui;
		HotReloadingState m_hot_reloading;
		ProjectState m_project;
	};

} // namespace engine

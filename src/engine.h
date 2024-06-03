#pragma once

#include <engine/animation.h>
#include <platform/commands.h>
#include <platform/font.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <stdint.h>
#include <string>
#include <unordered_map>

struct ImGuiContext;
typedef struct FT_LibraryRec_* FT_Library;

namespace engine {

	struct ImGuiState {
		int resolution_index = 0;
	};

	struct State {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
		AnimationSystem animations;

		float global_time_ms = 0.0;
		AnimationID window_title_animation_id;

		bool show_imgui = false;
		glm::vec2 circle_pos = { -1.0f, -1.0f };
		int circle_radius = 127;
		glm::vec2 window_resolution;

		ImGuiState imgui_state;
	};

	// global state wiring
	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void set_imgui_context(ImGuiContext* imgui_context);
	extern "C" __declspec(dllexport) void set_freetype_library(FT_Library ft);

	// engine interface
	extern "C" __declspec(dllexport) void initialize(State* state);
	extern "C" __declspec(dllexport) void shutdown(State* state);
	extern "C" __declspec(dllexport) void update(State* state, const platform::Input* input, platform::PlatformAPI* platform);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

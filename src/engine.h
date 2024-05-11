#pragma once

#include <platform/commands.h>
#include <platform/input/input.h>
#include <platform/logging.h>
#include <platform/renderer.h>

#include <stdint.h>

#include <imgui/imgui.h>

namespace engine {

	struct State {
		bool show_imgui = false;
		platform::Texture texture;
		glm::vec2 circle_pos = { -1.0f, -1.0f };
		int circle_radius = 127;
		glm::vec2 window_resolution;
	};

	extern "C" __declspec(dllexport) void set_logger(plog::Severity severity, plog::IAppender* appender);
	extern "C" __declspec(dllexport) void set_imgui_context(ImGuiContext* imgui_context);
	extern "C" __declspec(dllexport) void initialize(State* state);
	extern "C" __declspec(dllexport) void deinitialize(State* state);
	extern "C" __declspec(dllexport) void update(State* state, const platform::Input* input, platform::CommandAPI* commands);
	extern "C" __declspec(dllexport) void render(platform::Renderer* renderer, const State* engine);

} // namespace engine

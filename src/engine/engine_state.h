#pragma once

#include <engine/animation.h>
#include <engine/hot_reloading.h>
#include <platform/font.h>
#include <platform/input/input.h>
#include <platform/renderer.h>

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace engine {

	struct ImGuiState {
		int resolution_index = 0;
	};

	struct Resources {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
	};

	struct Systems {
		AnimationSystem animation;
	};

	struct State {
		Resources resources;
		Systems systems;

		bool show_imgui = false;
		glm::vec2 circle_pos = { -1.0f, -1.0f };
		int circle_radius = 127;
		glm::vec2 window_resolution;

		ImGuiState imgui_state;
		HotReloadingState hot_reloading;
	};

} // namespace engine

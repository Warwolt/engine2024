#pragma once

#include <engine/animation.h>
#include <engine/editor.h>
#include <engine/game_state.h>
#include <engine/hot_reloading.h>
#include <engine/project_state.h>
#include <platform/font.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace engine {

	struct DebugUiState {
		bool show_debug_ui = false;
		int resolution_index = 0;
	};

	struct Resources {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
		std::unordered_map<std::string, platform::Canvas> canvases;
	};

	struct Systems {
		AnimationSystem animation;
	};

	struct State {
		Resources resources;
		Systems systems;
		bool editor_is_running;
		glm::vec2 window_resolution;
		DebugUiState debug_ui;
		HotReloadingState hot_reloading;
		ProjectState project;
		GameState game;
		EditorState editor;
	};

} // namespace engine

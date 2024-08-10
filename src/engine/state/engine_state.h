#pragma once

#include <editor/editor.h>
#include <engine/state/game_state.h>
#include <engine/state/project_state.h>
#include <engine/system/animation.h>
#include <engine/system/hot_reloading.h>
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
		editor::EditorState editor;
	};

} // namespace engine

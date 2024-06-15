#pragma once

#include <engine/animation.h>
#include <engine/hot_reloading.h>
#include <platform/font.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>
#include <platform/renderer.h>

#include <future>
#include <stdint.h>
#include <string>
#include <unordered_map>

namespace engine {

	struct DebugUiState {
		bool show_debug_ui = false;
		int resolution_index = 0;
	};

	struct EditorState {
		std::future<std::vector<uint8_t>> load_project_future;
	};

	struct Resources {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
		std::unordered_map<std::string, platform::Canvas> canvases;
	};

	struct Systems {
		AnimationSystem animation;
	};

	struct GameState {
		int counter = 0;
		uint64_t time_ms = 0;
	};

	struct Project {
		std::string name;
		std::string path;
	};

	struct State {
		Resources resources;
		Systems systems;
		bool editor_is_running;
		glm::vec2 window_resolution;
		DebugUiState debug_ui;
		HotReloadingState hot_reloading;
		GameState game;
		EditorState editor;
		Project project;
	};

} // namespace engine

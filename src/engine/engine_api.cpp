#include <engine/engine_api.h>

#include <core/container.h>
#include <engine/system/hot_reloading.h>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/file/file.h>
#include <platform/os/imwin32.h>

#include <imgui/imgui.h>
#include <plog/Init.h>

namespace engine {
	static void draw_imgui(DebugUiState* debug_ui, platform::PlatformAPI* platform) {
		struct Resolution {
			glm::ivec2 value;
			const char* str;
		};
		const static Resolution resolutions[] = {
			{ { 800, 600 }, "800x600" },
			{ { 640, 480 }, "640x480" },
			{ { 960, 600 }, "960x600" },
			{ { 480, 300 }, "480x300" },

		};

		const char* combo_preview_value = resolutions[debug_ui->resolution_index].str;
		if (ImGui::BeginCombo("Resolution", combo_preview_value, 0)) {
			for (int n = 0; n < IM_ARRAYSIZE(resolutions); n++) {
				const bool current_is_selected = (debug_ui->resolution_index == n);
				if (ImGui::Selectable(resolutions[n].str, current_is_selected)) {
					debug_ui->resolution_index = n;
				}

				if (current_is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Change resolution")) {
			glm::ivec2 resolution = resolutions[debug_ui->resolution_index].value;
			platform->change_resolution(resolution.x, resolution.y);
		}
	}

	void set_logger(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	void set_imgui_context(ImGuiContext* imgui_context) {
		ImGui::SetCurrentContext(imgui_context);
	}

	void set_imwin32_context(ImWin32::ImWin32Context* imwin32_context) {
		ImWin32::SetCurrentContext(imwin32_context);
	}

	void set_freetype_library(FT_Library ft) {
		platform::set_ft(ft);
	}

	void initialize(State* state) {
		/* Initialize */
		const bool reset_docking = true; // FIXME: check if config exists
		init_editor(&state->editor, state->project, reset_docking);

		/* Add fonts */
		{
			const char* arial_font_path = "C:/windows/Fonts/Arial.ttf";
			platform::Font font = core::container::unwrap(platform::add_ttf_font(arial_font_path, 16), [&] {
				ABORT("Failed to load font \"%s\"", arial_font_path);
			});
			state->resources.fonts["arial-16"] = font;
		}
	}

	void shutdown(State* state) {
		for (const auto& [_, texture] : state->resources.textures) {
			platform::free_texture(texture);
		}
		for (const auto& [_, font] : state->resources.fonts) {
			platform::free_font(font);
		}
		for (const auto& [_, canvas] : state->resources.canvases) {
			platform::free_canvas(canvas);
		}
	}

	void load_project(State* state, const char* path_str) {
		std::filesystem::path path = std::filesystem::path(path_str);
		if (std::filesystem::is_regular_file(path)) {
			std::vector<uint8_t> data = platform::read_file_bytes(path).value();
			state->project = core::container::unwrap(engine::ProjectState::from_json_string(data, path), [&](const std::string& error) {
				ABORT("Could not parse json file \"%s\": %s", path_str, error.c_str());
			});
			init_game_state(&state->game, state->project);
			LOG_INFO("Game data loaded from \"%s\"", path_str);
		}
		else {
			LOG_ERROR("Could not load game data from path \"%s\"", path_str);
		}
	}

	void update(State* state, const platform::Input& input, platform::PlatformAPI* platform) {
		state->window_resolution = input.window_resolution;
		state->editor_is_running = input.mode == platform::RunMode::Editor;
		const bool game_is_running = input.mode == platform::RunMode::Game;

		/* Quit */
		{
			// editor handles quit in editor mode
			if (game_is_running) {
				if (input.quit_signal_received) {
					platform->quit();
				}

				if (input.keyboard.key_pressed_now(SDLK_ESCAPE)) {
					if (input.is_editor_mode) {
						platform->set_run_mode(platform::RunMode::Editor);
						platform->set_window_mode(platform::WindowMode::Windowed);
					}
					else {
						platform->quit();
					}
				}
			}
		}

		/* Update game */
		if (game_is_running) {
			state->game.time_ms += input.delta_ms;
			if (state->game.time_ms >= 1000) {
				state->game.time_ms -= 1000;
				state->game.counter += 1;
			}
		}

		/* Window */
		{
			if (input.keyboard.key_pressed_now(SDLK_F11)) {
				platform->toggle_fullscreen();
			}
		}

		/* Debug UI */
		{
			if (input.keyboard.key_pressed_now(SDLK_F3)) {
				state->debug_ui.show_debug_ui = !state->debug_ui.show_debug_ui;
			}

			if (state->debug_ui.show_debug_ui) {
				draw_imgui(&state->debug_ui, platform);
			}
		}

		/* Modules */
		{
			std::string window_title = state->project.name;
			if (input.is_editor_mode) {
				window_title += std::string(state->editor.project_has_unsaved_changes ? "*" : "") + " - Engine2024";
			}
			update_hot_reloading(&state->hot_reloading, &state->systems.animation, input, platform, &window_title);
			platform->set_window_title(window_title.c_str());
			if (input.mode == platform::RunMode::Editor) {
				update_editor(&state->editor, &state->game, &state->project, input, platform);
			}
		}
	}

	void render(platform::Renderer* renderer, const State* state) {
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, state->window_resolution }, glm::vec4 { 0.33f, 0.37f, 0.4f, 1.0f });

		platform::Font font = state->resources.fonts.at("arial-16");
		glm::vec4 text_color = glm::vec4 { 0.92f, 0.92f, 0.92f, 1.0f };
		if (state->editor_is_running) {
			renderer->draw_text_centered(font, "Editor", state->window_resolution / 2.0f, text_color);
		}
		else {
			std::string text = std::to_string(state->game.counter);
			glm::vec2 line1_pos = state->window_resolution / 2.0f;
			glm::vec2 line2_pos = state->window_resolution / 2.0f + glm::vec2 { 0.0f, font.height };
			renderer->draw_text_centered(font, "Game", line1_pos, text_color);
			renderer->draw_text_centered(font, text.c_str(), line2_pos, text_color);
		}
	}

} // namespace engine

#include <engine.h>

#include <engine/hot_reloading.h>
#include <imgui/imgui.h>
#include <platform/assert.h>
#include <platform/logging.h>
#include <plog/Init.h>
#include <util.h>

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

	void set_freetype_library(FT_Library ft) {
		platform::set_ft(ft);
	}

	void initialize(State* state) {
		/* Add fonts */
		{
			const char* arial_font_path = "C:/windows/Fonts/Arial.ttf";
			platform::Font font = util::unwrap(platform::add_ttf_font(arial_font_path, 16), [&] {
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
			platform::free_font(&font);
		}
		for (const auto& [_, canvas] : state->resources.canvases) {
			platform::free_canvas(canvas);
		}
	}

	void update(State* state, const platform::Input* input, platform::PlatformAPI* platform) {
		state->window_resolution = input->window_resolution;
		state->editor_is_running = input->mode == platform::RunMode::Editor;
		const bool game_is_running = input->mode == platform::RunMode::Game;
		const bool editor_is_running = input->mode == platform::RunMode::Editor;

		/* Quit */
		{
			if (input->quit_signal_received) {
				platform->quit();
			}

			if (input->keyboard.key_pressed_now(SDLK_ESCAPE)) {
				if (editor_is_running) {
					platform->quit();
				}

				if (game_is_running) {
					platform->set_run_mode(platform::RunMode::Editor);
					platform->set_window_mode(platform::WindowMode::Windowed);
				}
			}
		}

		/* Update game */
		if (game_is_running) {
			state->game.time_ms += input->delta_ms;
			if (state->game.time_ms >= 1000) {
				state->game.time_ms -= 1000;
				state->game.counter += 1;
			}
		}

		/* Window*/
		{
			if (input->keyboard.key_pressed_now(SDLK_F11)) {
				platform->toggle_fullscreen();
			}
		}

		/* Debug UI */
		{
			if (input->keyboard.key_pressed_now(SDLK_F3)) {
				state->debug_ui.show_debug_ui = !state->debug_ui.show_debug_ui;
			}

			if (state->debug_ui.show_debug_ui) {
				draw_imgui(&state->debug_ui, platform);
			}
		}

		/* Modules */
		{
			update_hot_reloading(&state->hot_reloading, &state->systems.animation, input, platform);
		}

		/* Editor window */
		if (editor_is_running && ImGui::Begin("Editor Window")) {
			const int step = 1;
			ImGui::InputScalar("Counter", ImGuiDataType_S16, &state->game.counter, &step, NULL, "%d");
			bool start_game = false;
			if (ImGui::Button("Run game")) {
				start_game = true;
				/* Initialize game */
				state->game.counter = 0;
				state->game.time_ms = 0;
			}
			if (ImGui::Button("Resume game")) {
				start_game = true;
			}

			if (start_game) {
				platform->set_run_mode(platform::RunMode::Game);
				platform->set_window_mode(platform::WindowMode::FullScreen);
			}

			ImGui::End();
		}
	}

	void render(platform::Renderer* renderer, const State* state) {
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, state->window_resolution }, glm::vec4 { 0.4f, 0.33f, 0.37f, 1.0f });
		platform::Font font = state->resources.fonts.at("arial-16");
		if (state->editor_is_running) {
			renderer->draw_text_centered(&font, "Editor", state->window_resolution / 2.0f, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f });
		}
		else {
			std::string text = std::to_string(state->game.counter);
			glm::vec2 line1_pos = state->window_resolution / 2.0f;
			glm::vec2 line2_pos = state->window_resolution / 2.0f + glm::vec2 { 0.0f, font.height };
			renderer->draw_text_centered(&font, "Game", line1_pos, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f });
			renderer->draw_text_centered(&font, text.c_str(), line2_pos, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f });
		}
	}

} // namespace engine

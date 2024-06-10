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
		state->is_editor_mode = input->config.is_editor_mode;

		/* Quit */
		{
			if (input->quit_signal_received || input->keyboard.key_pressed_now(SDLK_ESCAPE)) {
				platform->quit();
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

		if (state->is_editor_mode && ImGui::Begin("Editor Window")) {
			if (ImGui::Button("Run game")) {
				platform->run_game();
			}
			ImGui::End();
		}
	}

	void render(platform::Renderer* renderer, const State* state) {
		renderer->draw_rect_fill({ { 0.0f, 0.0f }, state->window_resolution }, glm::vec4 { 0.4f, 0.33f, 0.37f, 1.0f });
		const char* text = state->is_editor_mode ? "Editor" : "Game";
		renderer->draw_text_centered(&state->resources.fonts.at("arial-16"), text, state->window_resolution / 2.0f, glm::vec4 { 0.0f, 1.0f, 0.0f, 1.0f });
	}

} // namespace engine

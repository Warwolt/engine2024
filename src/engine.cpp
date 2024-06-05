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
		// Add Arial font
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
	}

	void update(State* state, const platform::Input* input, platform::PlatformAPI* platform) {
		state->window_resolution = input->window_resolution;

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

		/* Imgui */
		{
			if (input->keyboard.key_pressed_now(SDLK_F3)) {
				state->debug_ui.show_debug_ui = !state->debug_ui.show_debug_ui;
			}

			if (state->debug_ui.show_debug_ui) {
				draw_imgui(&state->debug_ui, platform);
			}
		}

		/* Hot reloading */
		update_hot_reloading(&state->hot_reloading, &state->systems.animation, input, platform);
	}

	void render(platform::Renderer* renderer, const State* state) {
		/* Render background */
		{
			const glm::vec4 light_grey = glm::vec4 { 0.75, 0.75, 0.75, 1.0 };
			const glm::vec4 dark_grey = glm::vec4 { 0.50, 0.50, 0.50, 1.0 };
			constexpr int tile_size = 32;

			for (int x = 0; x < state->window_resolution.x; x += tile_size) {
				for (int y = 0; y < state->window_resolution.y; y += tile_size) {
					const glm::vec4 color = (x / tile_size) % 2 == (y / tile_size) % 2 ? light_grey : dark_grey;
					renderer->draw_rect_fill({ { x, y }, { x + tile_size, y + tile_size } }, color);
				}
			}
		}

	} // namespace engine

}

#include <engine.h>

#include <engine/hot_reloading.h>
#include <imgui/imgui.h>
#include <platform/assert.h>
#include <platform/logging.h>
#include <plog/Init.h>
#include <util.h>

namespace engine {

	static void draw_imgui(ImGuiState* state, platform::PlatformAPI* platform) {
		struct Resolution {
			glm::ivec2 value;
			const char* str;
		};
		const static Resolution resolutions[] = {
			{ { 800, 600 }, "800x600" },
			{ { 640, 480 }, "640x480" },

		};

		const char* combo_preview_value = resolutions[state->resolution_index].str;
		if (ImGui::BeginCombo("Resolution", combo_preview_value, 0)) {
			for (int n = 0; n < IM_ARRAYSIZE(resolutions); n++) {
				const bool current_is_selected = (state->resolution_index == n);
				if (ImGui::Selectable(resolutions[n].str, current_is_selected)) {
					state->resolution_index = n;
				}

				if (current_is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Change resolution")) {
			glm::ivec2 resolution = resolutions[state->resolution_index].value;
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
		// Add container.jpg texture
		{
			const char* img_path = "resources/textures/container.jpg";
			platform::Image image = util::unwrap(platform::read_image(img_path), [&] {
				ABORT("read_file(%s) failed", img_path);
			});
			state->resources.textures["container"] = platform::add_texture(image.data.get(), image.width, image.height);
		}

		// Add Arial font
		{
			const char* font_path = "C:/windows/Fonts/Arial.ttf";
			platform::Font font = util::unwrap(platform::add_ttf_font(font_path, 16), [&] {
				ABORT("Failed to load font \"%s\"", font_path);
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

		/* Circle */
		{
			if (state->circle_pos == glm::vec2 { -1.0f, -1.0f }) {
				state->circle_pos = input->window_resolution / 2.0f;
			}

			if (input->mouse.left_button.is_pressed()) {
				state->circle_pos = input->mouse.pos;
			}

			if (input->mouse.scroll_delta) {
				state->circle_radius = std::max(state->circle_radius + 10 * input->mouse.scroll_delta, 0);
			}
		}

		/* Imgui */
		{
			if (input->keyboard.key_pressed_now(SDLK_F3)) {
				state->show_imgui = !state->show_imgui;
			}

			if (state->show_imgui) {
				draw_imgui(&state->imgui_state, platform);
			}
		}

		/* Hot reloading */
		update_hot_reloading(&state->hot_reloading, &state->systems.animation, input, platform);
	}

	void render(platform::Renderer* renderer, const State* state) {
		/* Render box */
		{
			glm::vec4 color = { 70.f / 255, 55.f / 255, 56.f / 255, 1.0f };
			glm::vec2 box_size = { 256.0f, 256.0f };
			glm::vec2 top_left = (state->window_resolution - box_size) / 2.0f;
			glm::vec2 offset = { 10.0f, 10.0f };

			renderer->draw_rect_fill({ { 0.0f, 0.0f }, state->window_resolution }, { 0.0f, 0.5f, 0.5f, 1.0f }); // background
			renderer->draw_rect_fill({ top_left + offset, top_left + box_size + offset }, color); // shadow
			renderer->draw_texture(state->resources.textures.at("container"), { top_left, top_left + box_size }); // box
		}

		/* Render circle */
		{
			glm::vec3 color = { 0.0f, 1.0f, 0.0f };
			renderer->draw_circle_fill(state->circle_pos, (float)state->circle_radius, { color, 0.75f });
			renderer->draw_circle(state->circle_pos, (float)state->circle_radius, { color, 1.0f });
		}

		/* Render text*/
		{
			const platform::Font* font = &state->resources.fonts.at("arial-16");
			glm::vec4 text_color = { 0.0f, 1.0f, 0.0f, 1.0f };
			glm::vec2 text_pos = { 300.0f, 100.0f };
			renderer->draw_text(font, "SPHINX OF BLACK QUARTZ, JUDGE MY VOW!!", text_pos, text_color);
			renderer->draw_text(font, "the quick brown fox jumps over the lazy dog", text_pos + glm::vec2 { 0, font->line_spacing }, text_color);
		}
	}

} // namespace engine

#include <engine.h>

#include <imgui/imgui.h>
#include <platform/assert.h>
#include <platform/logging.h>
#include <plog/Init.h>
#include <util.h>

using Rect = platform::Rect;

namespace engine {

	void set_logger(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
	}

	void set_imgui_context(ImGuiContext* imgui_context) {
		ImGui::SetCurrentContext(imgui_context);
	}

	void initialize(State* state) {
		const char* img_path = "resources/textures/container.jpg";
		platform::Image image = util::unwrap(platform::read_image(img_path), [&] {
			ABORT("read_file(%s) failed", img_path);
		});
		state->texture = platform::add_texture(image.data.get(), image.width, image.height);
	}

	void deinitialize(State* state) {
		platform::free_texture(state->texture);
	}

	void update(State* state, const platform::Input* input, platform::CommandAPI* commands) {
		if (state->circle_pos == glm::ivec2 { -1, -1 }) {
			state->circle_pos = input->window_resolution / 2;
		}

		if (input->mouse.left_button.is_pressed()) {
			state->circle_pos = input->mouse.pos;
		}

		if (input->keyboard.key_pressed_now(SDLK_F3)) {
			state->show_imgui = !state->show_imgui;
		}

		if (input->mouse.scroll_delta) {
			state->circle_radius = std::max(state->circle_radius + 10 * input->mouse.scroll_delta, 0);
		}

		if (input->quit_signal_received || input->keyboard.key_pressed_now(SDLK_ESCAPE)) {
			commands->quit();
		}

		if (input->keyboard.key_pressed_now(SDLK_F11)) {
			commands->toggle_fullscreen();
		}

		if (state->show_imgui) {
			bool show_demo_window = true;
			ImGui::ShowDemoWindow(&show_demo_window);
		}
	}

	void render(platform::Renderer* renderer, const State* state) {
		renderer->draw_circle_fill(state->circle_pos, (float)state->circle_radius, { 0.0f, 1.0f, 0.0f, 0.5f });
		renderer->draw_circle(state->circle_pos, (float)state->circle_radius, { 0.0f, 1.0f, 0.0f, 1.0f });
	}

} // namespace engine

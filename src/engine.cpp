#include <engine.h>

#include <imgui/imgui.h>
#include <platform/assert.h>
#include <platform/logging.h>
#include <plog/Init.h>
#include <util.h>

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
		state->window_resolution = input->window_resolution;

		if (state->circle_pos == glm::vec2 { -1.0f, -1.0f }) {
			state->circle_pos = input->window_resolution / 2.0f;
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
		glm::vec4 color = { 70.f / 255, 55.f / 255, 56.f / 255, 1.0f };
		glm::vec2 box_size = { 256.0f, 256.0f };
		glm::vec2 top_left = (state->window_resolution - box_size) / 2.0f;
		glm::vec2 offset = { 10.0f, 10.0f };

		renderer->draw_rect_fill({ { 0.0f, 0.0f }, state->window_resolution }, { 0.0f, 0.5f, 0.5f, 1.0f }); // background
		renderer->draw_rect_fill({ top_left + offset, top_left + box_size + offset }, color); // shadow
		renderer->draw_texture(state->texture, { top_left, top_left + box_size }); // box

		renderer->draw_circle_fill(state->circle_pos, (float)state->circle_radius, { 0.0f, 1.0f, 0.0f, 0.75f });
		renderer->draw_circle(state->circle_pos, (float)state->circle_radius, { 0.0f, 1.0f, 0.0f, 1.0f });
	}

} // namespace engine

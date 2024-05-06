#include <engine.h>

#include <platform/assert.h>
#include <platform/logging.h>
#include <plog/Init.h>
#include <util.h>

namespace engine {

	void set_logger(plog::Severity severity, plog::IAppender* appender) {
		plog::init(severity, appender);
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

	platform::Commands update(State* state, const platform::Input* input) {
		platform::Commands commands = { 0 };

		state->millis += input->delta_ms;
		if (state->millis >= 1000) {
			state->millis -= 1000;
			state->tick += 1;
			LOG_INFO("%zu", state->tick);
		}

		if (input->quit_signal_received || input->keyboard.key_pressed_now(SDL_SCANCODE_ESCAPE)) {
			commands.quit();
		}

		return commands;
	}

	void render(platform::Renderer* renderer, const State* state) {
		glm::vec4 color = { 70.f / 255, 55.f / 255, 56.f / 255, 1.0f };
		glm::vec2 box_size = { 256.0f, 256.0f };
		glm::vec2 top_left = (renderer->canvas_size() - box_size) / 2.0f;
		glm::vec2 window_center = renderer->canvas_size() / 2.0f;
		glm::vec2 offset = { 10.0f, 10.0f };
		renderer->draw_rect_fill(top_left + offset, top_left + box_size + offset, color); // shadow
		renderer->draw_texture(top_left, top_left + box_size, state->texture); // box

		renderer->draw_point(top_left, { 0.0f, 1.0f, 0.0f, 1.0f });
		renderer->draw_line(top_left, top_left + box_size, { 1.0f, 0.0f, 0.0f, 1.0f });

		renderer->draw_rect_fill(top_left, top_left + box_size, { 0.0f, 1.0f, 0.0f, 0.5f });
		renderer->draw_rect(top_left, top_left + box_size, { 0.0f, 1.0f, 0.0f, 1.0f });

		renderer->draw_circle_fill(window_center, box_size.x / 2.0f, { 0.0f, 0.5f, 1.0f, 0.5f });
		renderer->draw_circle(window_center, box_size.x / 2.0f, { 0.0f, 0.5f, 1.0f, 1.0f });
	}

} // namespace engine

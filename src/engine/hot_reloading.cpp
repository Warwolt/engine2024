#include <engine/hot_reloading.h>

#include <engine/engine_state.h>
#include <platform/commands.h>
#include <platform/input/input.h>

namespace engine {

	static std::string loading_window_title_animation(float t) {
		if (t < 1.0 / 3.0) {
			return "Engine2024 (rebuilding)";
		}
		if (t < 2.0 / 3.0) {
			return "Engine2024 (rebuilding.)";
		}
		else /* t < 3.0 / 3.0 */ {
			return "Engine2024 (rebuilding..)";
		}
	}

	// FIXME: Change this to rely on HotReloadingState instead
	// FIXME: Move the global_time_ms to platform::Input
	void update_hot_reloading(State* state, const platform::Input* input, platform::PlatformAPI* platform) {
		/* Input */
		const bool hot_reload_key_pressed = input->keyboard.key_pressed_now(SDLK_F5);
		const bool library_rebuild_just_started = input->engine_library_is_rebuilding.just_became(true);
		const bool library_rebuild_just_stopped = input->engine_library_is_rebuilding.just_became(false);
		const float global_time_ms = state->global_time_ms;

		/* Update */
		if (library_rebuild_just_started) {
			constexpr float period_ms = 2000.0f;
			state->hot_reloading.title_animation_id = state->systems.animation.start_animation("loading_window_title", period_ms, global_time_ms);
		}

		if (library_rebuild_just_stopped) {
			state->systems.animation.stop_animation(state->hot_reloading.title_animation_id);
		}

		if (hot_reload_key_pressed) {
			platform->rebuild_engine_library();
		}

		std::string window_title = "Engine2024";
		if (std::optional<Animation> animation = state->systems.animation.most_recent_animation("loading_window_title")) {
			if (animation->is_playing(state->global_time_ms)) {
				window_title = loading_window_title_animation(animation->local_time(state->global_time_ms));
			}
		}

		/* Render */
		platform->set_window_title(window_title.c_str());
	}

} // namespace engine

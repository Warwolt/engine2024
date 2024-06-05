#include <engine/hot_reloading.h>

#include <engine/animation.h>
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

	void update_hot_reloading(HotReloadingState* hot_reloading, AnimationSystem* animation_system, const platform::Input* input, platform::PlatformAPI* platform) {
		/* Input */
		const bool hot_reload_key_pressed = input->keyboard.key_pressed_now(SDLK_F5);
		const bool library_rebuild_just_started = input->engine_library_is_rebuilding.just_became(true);
		const bool library_rebuild_just_stopped = input->engine_library_is_rebuilding.just_became(false);
		const uint64_t global_time_ms = input->global_time_ms;

		/* Update */
		if (library_rebuild_just_started) {
			constexpr uint64_t period_ms = 2000;
			hot_reloading->title_animation_id = animation_system->start_animation("loading_window_title", period_ms, global_time_ms);
		}

		if (library_rebuild_just_stopped) {
			animation_system->stop_animation(hot_reloading->title_animation_id);
		}

		if (hot_reload_key_pressed) {
			platform->rebuild_engine_library();
		}

		std::string window_title = "Engine2024";
		if (std::optional<Animation> animation = animation_system->most_recent_animation("loading_window_title")) {
			if (animation->is_playing(global_time_ms)) {
				window_title = loading_window_title_animation(animation->local_time(global_time_ms));
			}
		}

		/* Render */
		platform->set_window_title(window_title.c_str());
	}

} // namespace engine

#include <engine/system/hot_reloading.h>

#include <engine/system/timeline_system.h>
#include <platform/input/input.h>
#include <platform/platform_api.h>

namespace engine {

	static void loading_window_title_animation(float t, std::string* window_title) {
		if (t < 1.0 / 3.0) {
			*window_title += " (rebuilding)";
		}
		else if (t < 2.0 / 3.0) {
			*window_title += " (rebuilding.)";
		}
		else /* t < 3.0 / 3.0 */ {
			*window_title += " (rebuilding..)";
		}
	}

	void update_hot_reloading(
		HotReloadingState* hot_reloading,
		TimelineSystem* timeline_system,
		const platform::Input& input,
		platform::PlatformAPI* platform,
		std::string* window_title
	) {
		/* Input */
		const bool hot_reload_key_pressed = input.keyboard.key_pressed_now_with_modifier(SDLK_F10, platform::KEY_MOD_ALT);
		const bool library_rebuild_just_started = input.engine_is_rebuilding.just_became(true);
		const bool library_rebuild_just_stopped = input.engine_is_rebuilding.just_became(false);
		const uint64_t global_time_ms = input.global_time_ms;

		/* Update */
		if (library_rebuild_just_started) {
			constexpr uint64_t period_ms = 2000;
			hot_reloading->animation_timeline_id = timeline_system->add_repeating_timeline("loading_window_title", global_time_ms, period_ms);
		}

		if (library_rebuild_just_stopped) {
			timeline_system->remove_timeline(hot_reloading->animation_timeline_id);
		}

		if (hot_reload_key_pressed) {
			platform->rebuild_engine_library();
		}

		/* Animate title while loading */
		if (std::optional<Timeline> timeline = timeline_system->most_recent_timeline("loading_window_title")) {
			if (timeline->is_active(global_time_ms)) {
				loading_window_title_animation(timeline->local_time(global_time_ms), window_title);
			}
		}

		/* Display error message if rebuild failed */
		if (!input.engine_is_rebuilding && input.engine_rebuild_exit_code != 0) {
			*window_title += " (Hot reloading failed!)";
		}
	}

} // namespace engine

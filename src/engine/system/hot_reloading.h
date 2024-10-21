#pragma once

#include <engine/system/timeline_system.h>

#include <string>

namespace platform {
	struct Input;
	class PlatformAPI;
}

namespace engine {

	class TimelineSystem;
	class Engine;

	struct HotReloadingState {
		TimelineID title_animation_id;
	};

	void update_hot_reloading(
		HotReloadingState* hot_reloading,
		TimelineSystem* animation_system,
		const platform::Input& input,
		platform::PlatformAPI* platform,
		std::string* window_title
	);

} // namespace engine

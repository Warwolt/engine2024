#pragma once

#include <engine/system/timeline_system.h>

#include <string>

namespace platform {
	struct Input;
	class PlatformAPI;
}

namespace engine {

	class AnimationSystem;
	class Engine;

	struct HotReloadingState {
		AnimationID title_animation_id;
	};

	void update_hot_reloading(
		HotReloadingState* hot_reloading,
		AnimationSystem* animation_system,
		const platform::Input& input,
		platform::PlatformAPI* platform,
		std::string* window_title
	);

} // namespace engine

#pragma once

#include <engine/animation.h>

namespace platform {
	struct Input;
	class PlatformAPI;
}

namespace engine {

	class AnimationSystem;
	struct State;

	struct HotReloadingState {
		AnimationID title_animation_id;
	};

	void update_hot_reloading(HotReloadingState* hot_reloading, AnimationSystem* animation_system, const platform::Input* input, platform::PlatformAPI* platform);

} // namespace engine

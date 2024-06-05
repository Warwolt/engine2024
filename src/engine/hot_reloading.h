#pragma once

namespace platform {
	struct Input;
	class PlatformAPI;
}

struct State;

namespace engine {

	void update_hot_reloading(State* state, const platform::Input* input, platform::PlatformAPI* platform);

} // namespace engine

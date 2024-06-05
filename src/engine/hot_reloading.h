#pragma once

#include <platform/input/input.h>
#include <platform/platform.h>

namespace engine {

	void update_hot_reloading(State* state, const platform::Input* input, platform::PlatformAPI* platform);

} // namespace engine

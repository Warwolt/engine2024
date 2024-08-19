#pragma once

#include <stddef.h>

namespace platform {

	struct RenderDebugData {
		size_t num_draw_calls = 0;
		size_t num_vertices = 0;
	};

} // namespace platform

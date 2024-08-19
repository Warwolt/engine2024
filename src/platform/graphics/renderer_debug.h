#pragma once

#include <stdint.h>

namespace platform {

	struct RenderDebugData {
		size_t num_draw_calls = 0;
		size_t num_vertices = 0;
		uint64_t render_ms = 0;
		uint64_t render_ns = 0;
	};

} // namespace platform

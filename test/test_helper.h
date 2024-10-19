#pragma once

#include <chrono>

#define WAIT_FOR(condition, wait_period)                                                                        \
	for (std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock().now(); !condition;) \
		if (std::chrono::high_resolution_clock().now() - start >= wait_period) {                                \
			FAIL() << "WAIT_FOR(" << #condition << ", " << #wait_period << ") timed out!";                      \
		}                                                                                                       \
		else

namespace platform {
	class OpenGLContext;
} // namespace platform

namespace testing {
	platform::OpenGLContext* gl_context();
} // namespace testing

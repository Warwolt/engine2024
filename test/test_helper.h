#pragma once

#include <chrono>

// @param bool condition
// @param std::chrono::duration wait_period
//
// Wait for `condition` to be true for `wait_period` time. If work takes longer
// than `wait_period`, immediately fail test.
//
// While waiting, allows executing a block of code, e.g:
//   WAIT_FOR(my_var == some_value, std::chrono::seconds(1)) {
//       do_some_work();
//   }
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

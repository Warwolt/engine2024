#pragma once

#include <chrono>
#include <stdint.h>

namespace platform {

	class Timer {
	public:
		Timer();

		void reset();
		uint64_t elapsed_ms();
		uint64_t elapsed_ns();

	private:
		std::chrono::steady_clock::time_point m_start;
		std::chrono::high_resolution_clock m_clock;
	};

} // namespace platform

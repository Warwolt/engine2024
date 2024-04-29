#pragma once

#include <chrono>
#include <stdint.h>

namespace timing {

	class Timer {
	public:
		Timer();

		void reset();
		uint64_t elapsed_ms();

	private:
		std::chrono::steady_clock::time_point m_start;
		std::chrono::high_resolution_clock m_clock;
	};

} // namespace timing

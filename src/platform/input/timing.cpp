#include <platform/input/timing.h>

namespace platform {

	Timer::Timer() {
		m_start = m_clock.now();
	}

	void Timer::reset() {
		m_start = m_clock.now();
	}

	uint64_t Timer::elapsed_ms() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now() - m_start).count();
	}

} // namespace platform

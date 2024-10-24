#pragma once

namespace core {

	// Helper struct for values that should continously change via lerping
	template <typename T>
	struct Lerped {
		T current;
		T start;
		T end;

		Lerped() = default;
		Lerped(const T& value)
			: current(value)
			, start(value)
			, end(T()) {
		}

		void set_target(const T& target) {
			this->start = this->current;
			this->end = target;
		}
	};

	template <typename T>
	T lerp(T start, T end, float t) {
		return start + (end - start) * t;
	}

} // namespace core

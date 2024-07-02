#pragma once

#include <future>
#include <optional>

namespace core::future {

	template <typename T>
	bool has_value(const std::future<T>& future) {
		if (future.valid()) {
			return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
		return false;
	}

} // namespace core::future

#pragma once

#include <future>
#include <optional>

namespace core {

	template <typename T>
	bool future_is_ready(const std::future<T>& future) {
		if (future.valid()) {
			return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
		return false;
	}

} // namespace

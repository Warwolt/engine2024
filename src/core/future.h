#pragma once

#include <future>
#include <vector>

namespace core {

	template <typename T>
	bool future_is_ready(const std::future<T>& future) {
		if (future.valid()) {
			return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
		}
		return false;
	}

	template <typename Iter, typename F>
	auto batch_async(std::launch policy, Iter first, Iter last, F&& fn) {
		using result_type = typename std::invoke_result<F, decltype(*first)>::type;
		std::vector<std::future<result_type>> batch;

		for (Iter it = first; it != last; ++it) {
			batch.push_back(std::async(policy, std::forward<F>(fn), *it));
		}

		return batch;
	}

	template <typename T, typename OutputIt>
	void get_all_batch_values(std::vector<std::future<T>>& batch, OutputIt out_first) {
		for (std::future<T>& future : batch) {
			*out_first = future.get();
			++out_first;
		}
	}

	template <typename T, typename OutputIt>
	void get_ready_batch_values(std::vector<std::future<T>>& batch, OutputIt out_first) {
		for (std::future<T>& future : batch) {
			if (future_is_ready(future)) {
				*out_first = future.get();
				++out_first;
			}
		}
	}

} // namespace

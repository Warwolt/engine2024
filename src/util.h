#pragma once

#include <platform/assert.h>

#include <magic_enum/magic_enum.h>

#include <algorithm>
#include <expected>
#include <future>
#include <optional>

namespace util {

	template <typename T, typename E, typename F>
	T unwrap(std::expected<T, E>&& result, F&& on_error_fn) {
		if (!result.has_value()) {
			on_error_fn(result.error());
			ABORT("util::unwrap called with std::expected not holding a value");
		}
		return std::move(result.value());
	}

	template <typename T, typename F>
	T unwrap(std::optional<T>&& result, F&& on_error_fn) {
		if (!result.has_value()) {
			on_error_fn();
			ABORT("util::unwrap called with std::optional not holding a value");
		}
		return std::move(result.value());
	}

	template <typename T>
	bool future_has_value(const std::future<T>& future) {
		return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	template <typename Container, typename T>
	bool contains(const Container& container, const T& value) {
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template <typename Container, typename Predicate>
	Container filter(const Container& container, Predicate predicate) {
		Container result;
		std::copy_if(container.begin(), container.end(), std::back_inserter(result), predicate);
		return result;
	}

	template <typename T>
	const char* enum_to_string(T value) {
		return magic_enum::enum_name(value).data();
	}

} // namespace util

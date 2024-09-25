#pragma once

#include <platform/debug/assert.h>

#include <expected>
#include <optional>

namespace core {

	template <typename T, typename E, typename F>
	T unwrap(std::expected<T, E>&& result, F&& on_error_fn) {
		if (!result.has_value()) {
			on_error_fn(result.error());
			ABORT("core::unwrap called with std::expected not holding a value");
		}
		return std::move(result.value());
	}

	template <typename T, typename F>
	T unwrap(std::optional<T>&& result, F&& on_error_fn) {
		if (!result.has_value()) {
			on_error_fn();
			ABORT("core::unwrap called with std::optional not holding a value");
		}
		return std::move(result.value());
	}

} // namespace core::container

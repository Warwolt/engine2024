#pragma once

#include <expected>
#include <magic_enum/magic_enum.h>

namespace util {

	template <typename T, typename E, typename F>
	T unwrap(std::expected<T, E> result, F&& on_error_fn) {
		if (!result.has_value()) {
			on_error_fn(result.error());
			ABORT("util::unwrap called with std::expected not holding a value");
		}
		return result.value();
	}

	template <typename T>
	const char* enum_to_string(T value) {
		return magic_enum::enum_name(value).data();
	}

} // namespace util

#pragma once

#include <magic_enum/magic_enum.h>

namespace core::util {

	template <typename T>
	const char* enum_to_string(T value) {
		return magic_enum::enum_name(value).data();
	}

} // namespace core::util

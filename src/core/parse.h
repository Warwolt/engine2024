#pragma once

#include <optional>
#include <stdlib.h>

namespace core {

	std::optional<int> string_to_number(char const* str, int base = 0) {
		// based on https://stackoverflow.com/a/6154614/3157744
		char* end;
		long number;
		errno = 0;
		number = strtol(str, &end, base);
		if ((errno == ERANGE && number == LONG_MAX) || number > INT_MAX) {
			return {};
		}
		if ((errno == ERANGE && number == LONG_MIN) || number < INT_MIN) {
			return {};
		}
		if (*str == '\0' || *end != '\0') {
			return {};
		}
		return number;
	}
} // namespace core

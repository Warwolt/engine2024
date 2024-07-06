#include <core/parse.h>

#include <stdlib.h>

namespace core::parse {

	std::optional<int> string_to_number(const std::string& str) {
		// based on https://stackoverflow.com/a/6154614/3157744
		char* end;
		long number;
		errno = 0;
		int base = 0;
		number = strtol(str.c_str(), &end, base);
		if ((errno == ERANGE && number == LONG_MAX) || number > INT_MAX) {
			return {};
		}
		if ((errno == ERANGE && number == LONG_MIN) || number < INT_MIN) {
			return {};
		}
		if (*str.c_str() == '\0' || *end != '\0') {
			return {};
		}
		return number;
	}

	std::optional<bool> string_to_bool(const std::string& str) {
		// check against "1" and "0" to match std::to_string behavior
		if (str == "1" || str == "true") {
			return true;
		}
		if (str == "0" || str == "false") {
			return false;
		}
		return {};
	}

} // namespace core::parse

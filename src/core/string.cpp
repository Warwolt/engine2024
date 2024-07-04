#include <core/string.h>

#include <string>

namespace core::string {

	bool equals(const char* lhs, const char* rhs) {
		return std::string(lhs) == std::string(rhs);
	}

	bool starts_with(const char* str, const char* prefix) {
		return strncmp(str, prefix, strlen(prefix)) == 0;
	}

} // namespace core::string

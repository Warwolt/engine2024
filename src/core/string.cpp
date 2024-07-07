#include <core/string.h>

#include <sstream>

namespace core::string {

	bool equals(const char* lhs, const char* rhs) {
		return std::string(lhs) == std::string(rhs);
	}

	bool starts_with(const char* str, const char* prefix) {
		return strncmp(str, prefix, strlen(prefix)) == 0;
	}

	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::stringstream ss(str);

		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

} // namespace core::string

#pragma once

#include <string>
#include <vector>

namespace core::string {

	bool equals(const char* lhs, const char* rhs);
	bool starts_with(const char* str, const char* prefix);
	std::vector<std::string> split(const std::string&, char delimiter);

} // namespace core::string

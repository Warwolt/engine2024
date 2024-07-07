#pragma once

#include <optional>
#include <string>

namespace core::parse {

	std::optional<int> string_to_number(const std::string& str);
	std::optional<bool> string_to_bool(const std::string& str);

} // namespace core::parse

#pragma once

#include <functional>
#include <stdint.h>

namespace core::hash {

	template <class T>
	inline void add_to_hash(std::size_t* hash, const T& v) {
		std::hash<T> hasher;
		constexpr size_t golden_ratio = 0x9e3779b9; // https://softwareengineering.stackexchange.com/a/402543/440432
		*hash ^= hasher(v) + golden_ratio + (*hash << 6) + (*hash >> 2); // https://stackoverflow.com/a/35991300/3157744
	}

} // namespace core::hash

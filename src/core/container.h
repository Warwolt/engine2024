#pragma once

#include <platform/debug/assert.h>

#include <algorithm>
#include <expected>
#include <future>
#include <optional>

#include <nlohmann/json.hpp>

namespace core::container {

	template <typename Container, typename T>
	bool contains(const Container& container, const T& value) {
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template <typename Container, typename Predicate>
	Container filter(const Container& container, Predicate predicate) {
		Container result;
		std::copy_if(container.begin(), container.end(), std::back_inserter(result), predicate);
		return result;
	}

	template <typename K, typename V>
	std::optional<V> map_get(const std::unordered_map<K, V>& map, const K& key) {
		auto it = map.find(key);
		return it == map.cend() ? std::nullopt : std::make_optional(it->second);
	}

	template <typename T>
	std::optional<T> json_get(const nlohmann::json& json_object, const std::string& key) {
		if (json_object.contains(key)) {
			return json_object[key];
		}
		return {};
	}

} // namespace core::container

#pragma once

#include <initializer_list>
#include <stdint.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace core {

	template <typename Key, typename T>
	class VecMap {
	public:
		VecMap() = default;
		VecMap(std::initializer_list<std::pair<const Key, T>> init) {
			size_t i = 0;
			for (const auto& [key, value] : init) {
				m_values.push_back(value);
				m_key_to_index.emplace(key, i++);
			}
		}

		size_t size() const {
			return m_values.size();
		}

		bool empty() const {
			return m_values.empty();
		}

	private:
		std::vector<T> m_values;
		std::unordered_map<Key, size_t> m_key_to_index;
	};

} // namespace core

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

		bool operator==(const VecMap<Key, T>& rhs) const {
			if (m_values.size() != rhs.size()) {
				return false;
			}

			for (auto& [key, index] : m_key_to_index) {
				if (!rhs.m_key_to_index.contains(key)) {
					return false;
				}
				size_t rhs_index = rhs.m_key_to_index.at(key);
				if (m_values.at(index) != rhs.m_values.at(rhs_index)) {
					return false;
				}
			}

			return true;
		}

		bool operator!=(const VecMap<Key, T>& rhs) const {
			return !(*this == rhs);
		}

		T& operator[](const Key& key) {
			if (m_key_to_index.contains(key)) {
				return m_values[m_key_to_index[key]];
			}
			m_values.push_back(T());
			m_key_to_index[key] = m_values.size() - 1;
			return m_values.back();
		}

		T& at(const Key& key) {
			return m_values[m_key_to_index.at(key)];
		}

		const T& at(const Key& key) const {
			return m_values[m_key_to_index.at(key)];
		}

	private:
		std::vector<T> m_values;
		std::unordered_map<Key, size_t> m_key_to_index;
	};

} // namespace core
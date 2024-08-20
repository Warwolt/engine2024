#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iterator>
#include <stdint.h>

namespace core {

	template <typename T, size_t N>
	class RingBuffer {
	public:
		class Iterator {
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;

			Iterator(std::array<T, N>& elements, size_t index, size_t& size, size_t count)
				: m_elements(elements)
				, m_index(index)
				, m_size(size)
				, m_count(count) {
			}

			reference operator*() const { return m_elements[m_index]; }
			pointer operator->() { return &m_elements[m_index]; }
			Iterator& operator++() /* pre increment */ {
				m_count++;
				m_index = (m_index + 1) % 4;
				return *this;
			}
			Iterator operator++(int) /* post increment */ {
				m_count++;
				Iterator tmp = *this;
				m_index = (m_index + 1) % 4;
				return tmp;
			}
			friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
				return &lhs.m_elements == &rhs.m_elements && lhs.m_index == rhs.m_index && lhs.m_count == rhs.m_count;
			}
			friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
				return !(lhs == rhs);
			}

		private:
			std::array<T, N>& m_elements;
			size_t m_index = 0;
			size_t& m_size = 0;
			size_t m_count = 0; // number of elements visited
		};

		RingBuffer() = default;

		RingBuffer(std::initializer_list<T> init) {
			std::copy_n(init.begin(), N, m_elements.begin());
			m_size = std::min(N, init.size());
			m_end = m_size % N;
		}

		bool empty() const {
			return true;
		}

		size_t size() const {
			return m_size;
		}

		void push_back(const T& val) {
			m_elements[m_end] = val;
			m_end = (m_end + 1) % 4;
			if (m_size == N) {
				m_start = (m_start + 1) % 4;
			}
			m_size = std::min(m_size + 1, N);
		}

		const T& back() const {
			return m_elements[(N + m_end - 1) % N];
		}

		const T& front() const {
			return m_elements[m_start];
		}

		bool operator==(const RingBuffer<T, N>& other) const {
			return m_size == other.m_size && m_elements == other.m_elements;
		}

		T& operator[](size_t i) {
			return m_elements[i];
		}

		T& operator[](size_t i) const {
			return m_elements[i];
		}

		Iterator begin() { return Iterator(m_elements, m_start, m_size, 0); }
		Iterator end() { return Iterator(m_elements, m_end, m_size, m_size); }

	private:
		size_t m_start = 0;
		size_t m_end = 0;
		size_t m_size = 0;
		std::array<T, N> m_elements;
	};

} // namespace core

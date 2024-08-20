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

			Iterator(std::array<T, N>& elements, size_t index)
				: m_elements(elements)
				, m_index(index) {
			}

			reference operator*() const { return m_elements[m_index]; }
			pointer operator->() { return &m_elements[m_index]; }

		private:
			std::array<T, N>& m_elements;
			size_t m_index = 0;
		};

		RingBuffer() = default;

		RingBuffer(std::initializer_list<T> init) {
			std::copy_n(init.begin(), N, m_elements.begin());
			m_size = init.size();
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

	private:
		size_t m_start = 0;
		size_t m_end = 0;
		size_t m_size = 0;
		std::array<T, N> m_elements;
	};

} // namespace core

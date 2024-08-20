#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <stdint.h>

namespace core {

	template <typename T, size_t N>
	class RingBuffer {
	public:
		RingBuffer() = default;

		RingBuffer(std::initializer_list<T> init) {
			std::copy(init.begin(), init.end(), m_elements.begin());
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
			return m_elements[m_end - 1];
		}

		const T& front() const {
			return m_elements[m_start - 1];
		}

		bool operator==(const RingBuffer<T, N>& other) const {
			return m_elements == other.m_elements;
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

#pragma once

namespace core {

	template <typename T>
	class Signal {
	public:
		Signal()
			: m_current {}
			, m_previous {} {}

		Signal(T x)
			: m_current(x)
			, m_previous(x) {}

		Signal(const Signal& other)
			: m_current(other.m_current)
			, m_previous(m_current) {}

		Signal(const Signal&& other)
			: m_current(other.m_current)
			, m_previous(m_current) {}

		Signal& operator=(const Signal& other) {
			m_previous = m_current;
			m_current = other.m_current;
			return *this;
		}

		Signal& operator=(const Signal&& other) {
			m_previous = m_current;
			m_current = other.m_current;
			return *this;
		}

		operator T() const { return m_current; }

		bool just_became(const T& x) const {
			return m_current == x && m_previous != x;
		}

		bool just_changed() const {
			return m_current != m_previous;
		}

	private:
		T m_current;
		T m_previous;
	};

} // namespace core

#pragma once

namespace core {

	template <typename T>
	struct NewType {
		T value = T();

		NewType() = default;

		NewType(const NewType& other)
			: value(other.value) {
		}

		constexpr explicit NewType(const T& value)
			: value(value) {
		}

		NewType& operator=(const NewType& rhs) {
			value = rhs.value;
			return *this;
		}

		bool operator==(const NewType& rhs) const {
			return this->value == rhs.value;
		}

		bool operator!=(const NewType& rhs) const {
			return !(*this == rhs);
		}

		operator const T&() const {
			return value;
		}
	};

} // namespace core

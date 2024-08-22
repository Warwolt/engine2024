#pragma once

#include <functional>

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

// Note! Macro must be expanded in top-level namespace
// since we specialize inside the std namespace.
#define DEFINE_NEWTYPE_HASH_IMPL(name, type)         \
	namespace std {                                  \
		template <> struct hash<name> {              \
			size_t operator()(const name& x) const { \
				return std::hash<type>()(x.value);   \
			}                                        \
		};                                           \
	}

#pragma once

#include <variant>

namespace platform {

	template <typename Tag, typename First, typename... Types>
	class TaggedVariant : public std::variant<First, Types...> {
	public:
		TaggedVariant() = default;

		template <typename T>
		TaggedVariant(const T& t)
			: std::variant<First, Types...>(t)
			, m_tag(T::TAG) {
		}

		template <typename T>
		TaggedVariant(T&& t)
			: std::variant<First, Types...>(std::move(t))
			, m_tag(T::TAG) {
		}

		Tag tag() const {
			return m_tag;
		}

	private:
		Tag m_tag = First::TAG;
	};

} // namespace platform

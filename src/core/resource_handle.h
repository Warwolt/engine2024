#pragma once

#include <functional>
#include <optional>
#include <utility>

namespace core {

	template <typename Resource, typename Deleter>
	class ResourceHandle;

	template <typename Resource, typename Result, typename... Args>
	class ResourceHandle<Resource, Result(Args...)> {
	public:
		ResourceHandle() = default;

		ResourceHandle(Resource resource, std::function<Result(Args...)> deleter)
			: m_resource(resource)
			, m_deleter(deleter) {
		}

		~ResourceHandle() {
			if (m_deleter && m_resource) {
				m_deleter(m_resource.value());
			}
		}

		// delete copy constructors and assignment
		ResourceHandle(const ResourceHandle&) = delete;
		ResourceHandle& operator=(const ResourceHandle&) = delete;

		// move constructor and assignment
		ResourceHandle(ResourceHandle&& other) noexcept
			: m_resource(std::exchange(other.m_resource, std::nullopt))
			, m_deleter(std::exchange(other.m_deleter, nullptr)) {
		}

		ResourceHandle& operator=(ResourceHandle&& other) noexcept {
			std::swap(m_resource, other.m_resource);
			std::swap(m_deleter, other.m_deleter);
			return *this;
		}

		Resource& get() {
			return m_resource.value();
		}

		const Resource& get() const {
			return m_resource.value();
		}

		Resource& operator*() {
			return m_resource.value();
		}

		const Resource& operator*() const {
			return m_resource.value();
		}

		Resource* operator->() {
			return &m_resource.value();
		}

		const Resource* operator->() const {
			return &m_resource.value();
		}

		operator bool() {
			return m_resource.has_value() && m_deleter;
		}

	private:
		std::optional<Resource> m_resource;
		std::function<Result(Args...)> m_deleter;
	};

} // namespace core

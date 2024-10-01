#pragma once

#include <core/future.h>

#include <future>
#include <vector>

namespace core {

	template <typename F>
	class AsyncBatch;

	template <typename Result, typename Argument>
	class AsyncBatch<Result(Argument)> {
	public:
		AsyncBatch() = default;

		template <typename F>
		AsyncBatch(std::vector<Argument>& xs, F&& f) {
			for (const auto& x : xs) {
				m_futures.push_back(std::async(std::launch::async, std::forward<F>(f), x));
			}
		}

		void update() {
			for (auto it = m_futures.begin(); it != m_futures.end();) {
				if (core::future::has_value(*it)) {
					m_values.push_back(it->get());
					it = m_futures.erase(it);
				}
				else {
					it++;
				}
			}
		}

		bool is_done() const {
			return m_futures.empty();
		}

		std::vector<Result>& values() {
			return m_values;
		}

	private:
		std::vector<Result> m_values;
		std::vector<std::future<Result>> m_futures;
	};

} // namespace core

#pragma once

#include <vector>

namespace platform {

	enum class CommandType {
		Quit,
	};

	class CommandAPI {
	public:
		const std::vector<CommandType>& commands() const;
		void clear();

		void quit();

	private:
		std::vector<CommandType> m_commands;
	};

} // namespace platform

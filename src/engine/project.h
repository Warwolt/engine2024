#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace engine {

	/**
	 * The meta data for the game project.
	 */
	struct ProjectState {
		std::string name = "Untitled Project";
		std::filesystem::path path;
	};

} // namespace engine

namespace std {
	template <> struct hash<engine::ProjectState> {
		std::size_t operator()(const engine::ProjectState& project) const noexcept {
			return std::hash<std::string>()(project.name);
		}
	};

} // namespace std

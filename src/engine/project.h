#pragma once

#include <expected>
#include <filesystem>
#include <functional>
#include <string>

// TODO move to core
template <class T>
inline void add_to_hash(std::size_t* hash, const T& v) {
	std::hash<T> hasher;
	constexpr size_t golden_ratio = 0x9e3779b9; // https://softwareengineering.stackexchange.com/a/402543/440432
	*hash ^= hasher(v) + golden_ratio + (*hash << 6) + (*hash >> 2); // https://stackoverflow.com/a/35991300/3157744
}

namespace engine {

	/**
	 * Project data that defines the game.
	 *
	 * Run-time information is stored in GameState
	 */
	struct ProjectState {
		std::filesystem::path path;
		std::string name = "Untitled Project";
		int counter = 0;

		static std::string to_json_string(const ProjectState& project);
		static std::expected<ProjectState, std::string> from_json_string(const std::vector<uint8_t>& json_bytes, const std::filesystem::path& path);
	};

} // namespace engine

namespace std {
	template <> struct hash<engine::ProjectState> {
		std::size_t operator()(const engine::ProjectState& project) const noexcept {
			size_t hash = 0;
			add_to_hash(&hash, project.name);
			add_to_hash(&hash, project.path);
			add_to_hash(&hash, project.counter);
			return hash;
		}
	};

} // namespace std

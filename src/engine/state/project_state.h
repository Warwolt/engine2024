#pragma once

#include <core/hash.h>

#include <expected>
#include <filesystem>
#include <functional>
#include <string>

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
			using namespace core::hash;
			size_t hash = 0;
			add_to_hash(&hash, project.name);
			add_to_hash(&hash, project.path);
			add_to_hash(&hash, project.counter);
			return hash;
		}
	};

} // namespace std

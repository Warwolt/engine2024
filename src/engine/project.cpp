#include <engine/project.h>

#include <nlohmann/json.hpp>

namespace engine {

	std::string ProjectState::to_json_string(const ProjectState& project) {
		nlohmann::json json_object = {
			{ "project_name", project.name },
			{ "counter", project.counter },
		};
		return json_object.dump();
	}

	std::optional<ProjectState> ProjectState::from_json_string(const std::vector<uint8_t>& json_bytes, const std::filesystem::path& path) {
		constexpr bool allow_exceptions = false;
		nlohmann::json json_object = nlohmann::json::parse(json_bytes, nullptr, allow_exceptions);

		if (json_object.empty()) {
			return {};
		}

		return ProjectState {
			.name = json_object["project_name"],
			.path = path,
			.counter = json_object["counter"],
		};
	}

} // namespace engine

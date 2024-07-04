#include <engine/project.h>

#include <core/container.h>

#include <nlohmann/json.hpp>

namespace engine {

	std::string ProjectState::to_json_string(const ProjectState& project) {
		nlohmann::json json_object = {
			{ "project_name", project.name },
			{ "counter", project.counter },
		};
		return json_object.dump();
	}

	std::expected<ProjectState, std::string> ProjectState::from_json_string(const std::vector<uint8_t>& json_bytes, const std::filesystem::path& path) {
		try {
			using namespace core::container;
			nlohmann::json json_object = nlohmann::json::parse(json_bytes);
			return ProjectState {
				.path = path,
				.name = json_get<std::string>(json_object, "project_name").value_or(""),
				.counter = json_get<int>(json_object, "counter").value_or(0),
			};
		}
		catch (const nlohmann::json::exception& e) {
			return std::unexpected(e.what());
		}
	}

} // namespace engine

#include <platform/file/config.h>

#include <core/parse.h>
#include <core/string.h>
#include <mini/ini.h>

#include <platform/debug/logging.h>

namespace platform {

	static std::optional<std::string> get_ini_value(const mINI::INIStructure& ini, const std::string& section, const std::string& key) {
		if (ini.has(section) && ini.get(section).has(key)) {
			return ini.get(section).get(key);
		}
		return {};
	}

	static std::optional<glm::ivec2> string_to_ivec2(const std::string& str) {
		std::optional<glm::ivec2> int_pair;

		if (!str.empty() && str.front() == '(' && str.back() == ')') {
			std::vector<std::string> parts = core::string::split(str, ',');
			if (parts.size() == 2) {
				std::optional<int> first = core::parse::string_to_number(parts[0].substr(1)); // skip '('
				std::optional<int> second = core::parse::string_to_number(parts[1].substr(0, parts[1].size() - 1)); // skip ')'
				if (first.has_value() && second.has_value()) {
					int_pair = glm::ivec2 { first.value(), second.value() };
				}
			}
		}

		return int_pair;
	}

	static Configuration parse_config(const mINI::INIStructure& ini) {
		using namespace core::parse;
		Configuration config;
		config.window.docking_initialized = get_ini_value(ini, "window", "docking_initialized").and_then(string_to_bool).value_or(false);
		config.window.fullscreen = get_ini_value(ini, "window", "fullscreen").and_then(string_to_bool).value_or(false);
		config.window.maximized = get_ini_value(ini, "window", "maximized").and_then(string_to_bool).value_or(false);
		config.window.position = get_ini_value(ini, "window", "position").and_then(string_to_ivec2).value_or(glm::ivec2 { 0, 0 });
		config.window.size = get_ini_value(ini, "window", "size").and_then(string_to_ivec2).value_or(glm::ivec2 { 0, 0 });
		return config;
	}

	std::optional<Configuration> load_configuration(const std::filesystem::path& path) {
		if (std::filesystem::is_regular_file(path)) {
			mINI::INIFile file = mINI::INIFile(path.string());
			mINI::INIStructure ini;
			file.read(ini);
			return parse_config(ini);
		}

		return {};
	}

	void save_configuration(const Configuration& config, const std::filesystem::path& path) {
		mINI::INIStructure ini;
		ini["window"]["docking_initialized"] = std::to_string(config.window.docking_initialized);
		ini["window"]["fullscreen"] = std::to_string(config.window.fullscreen);
		ini["window"]["maximized"] = std::to_string(config.window.maximized);
		ini["window"]["position"] = std::string("(") + std::to_string(config.window.position.x) + "," + std::to_string(config.window.position.y) + ")";
		ini["window"]["size"] = std::string("(") + std::to_string(config.window.size.x) + "," + std::to_string(config.window.size.y) + ")";

		mINI::INIFile file = mINI::INIFile(path.string());
		file.write(ini);
	}

} // namespace platform

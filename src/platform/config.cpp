#include <platform/config.h>

#include <core/parse.h>
#include <core/string.h>
#include <mini/ini.h>

#include <platform/logging.h>

namespace platform {

	static std::optional<std::string> ini_get_value(const mINI::INIStructure& ini, const std::string& section, const std::string& key) {
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
				std::optional<int> first = core::parse::string_to_number(parts[0].substr(1));
				std::optional<int> second = core::parse::string_to_number(parts[1].substr(0, 1));
				if (first.has_value() && second.has_value()) {
					int_pair = glm::ivec2 { first.value(), second.value() };
				}
			}
		}

		return int_pair;
	}

	static Configuration parse_config(const mINI::INIStructure& ini) {
		Configuration config;
		config.full_screen = ini_get_value(ini, "window", "full_screen").and_then(core::parse::string_to_bool).value_or(false);
		config.window_pos = ini_get_value(ini, "window", "window_pos").and_then(string_to_ivec2).value_or(glm::ivec2 { 0, 0 });
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
		ini["window"]["full_screen"] = std::to_string(config.full_screen);
		ini["window"]["window_pos"] = std::string("(") + std::to_string(config.window_pos.x) + "," + std::to_string(config.window_pos.y) + ")";
		ini["window"]["window_size"] = std::string("(") + std::to_string(config.window_size.x) + "," + std::to_string(config.window_size.y) + ")";

		mINI::INIFile file = mINI::INIFile(path.string());
		file.write(ini);
	}

} // namespace platform

#include <platform/config.h>

#include <core/parse.h>
#include <mini/ini.h>

namespace platform {

	std::optional<Configuration> load_configuration(const std::filesystem::path& path) {
		std::optional<Configuration> config;

		mINI::INIStructure ini;
		mINI::INIFile file = mINI::INIFile(path.string());
		if (std::filesystem::is_regular_file(path)) {
			file.read(ini);
			config = Configuration();
			if (ini.has("test") && ini["test"].has("counter")) {
				config->counter = core::string_to_number(ini["test"]["counter"].c_str()).value_or(0);
			}
		}

		return config;
	}

	void save_configuration(const Configuration& config, const std::filesystem::path& path) {
		mINI::INIStructure ini;
		ini["test"]["counter"] = std::to_string(config.counter);

		mINI::INIFile file = mINI::INIFile(path.string());
		file.write(ini);
	}

} // namespace platform

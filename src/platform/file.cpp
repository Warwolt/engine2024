#include <platform/file.h>

#include <fstream>

namespace platform {

	std::optional<std::string> read_file_to_string(const std::filesystem::path& path) {
		std::string line, text;
		std::ifstream in(path);
		if (!in.is_open()) {
			return {};
		}
		while (std::getline(in, line)) {
			text += line + "\n";
		}
		return text;
	}

} // namespace platform

#include <platform/file/file.h>

#include <fstream>

namespace platform {

	std::optional<std::string> read_file_to_string(const std::filesystem::path& path) {
		std::string line, text;
		std::ifstream file(path);
		if (!file.is_open()) {
			return {};
		}
		while (std::getline(file, line)) {
			text += line + "\n";
		}
		return text;
	}

	std::optional<std::vector<uint8_t>> read_bytes_from_file(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			return {};
		}

		file.seekg(0, std::ios_base::end);
		size_t length = file.tellg();
		file.seekg(0, std::ios_base::beg);

		std::vector<uint8_t> bytes = std::vector<uint8_t>(length);
		file.read((char*)bytes.data(), length);

		return bytes;
	}

} // namespace platform

#pragma once

#include <miniz/miniz.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace platform {

	enum class ArchiveMode {
		Read,
		Write,
	};

	enum class FileArchiveError {
		NoSuchFile,
		ReadFailed,
	};

	class FileArchive {
	public:
		FileArchive() = default;

		// NOTE: For some cursed reason we can't just return a `mz_zip_archive` inside a std::expected,
		// because when later using the archive we end up with some pointer not having beed copied?
		// The only safe way to initialize is by-reference. Who knows, something something UB maybe.
		static std::optional<std::string> open_from_file(FileArchive* archive, const std::filesystem::path& path);

		const std::vector<std::string> file_names() const;
		std::expected<std::vector<uint8_t>, FileArchiveError> read_file(std::string file_name);

		mz_zip_archive m_mz_archive = { 0 };
		ArchiveMode m_mode = ArchiveMode::Read;

		std::unordered_map<std::string, mz_uint> m_file_indicies;
		std::vector<std::string> m_file_names;
	};

} // namespace platform

#pragma once

#include <miniz/miniz.h>

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

	class Archive {
	public:
		// NOTE: For some cursed reason we can't just return a `mz_zip_archive` inside a std::expected,
		// because when later using the archive we end up with some pointer not having beed copied?
		// The only safe way to initialize is by-reference. Who knows, something something UB maybe.
		static std::optional<std::string> open_from_file(Archive* archive, const std::filesystem::path& path);

		mz_zip_archive m_archive = { 0 };
		std::vector<mz_zip_archive_file_stat> m_file_stats;
		std::unordered_map<std::string, mz_uint> m_file_indicies;
		ArchiveMode m_mode = ArchiveMode::Read;
	};

} // namespace platform

#pragma once

#include <miniz/miniz.h>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace platform {

	enum class FileArchiveError {
		NoSuchFile,
		ReadFailed,
		CouldNotCreateArchive,
		CouldNotWriteArchive,
		WritingFileFailed,
	};

	class FileArchive {
	public:
		FileArchive() = default;

		static std::optional<std::string> open_from_file(FileArchive* archive, const std::filesystem::path& path);

		const std::vector<std::string> file_names() const;
		std::expected<std::vector<uint8_t>, FileArchiveError> read_from_archive(const std::string& file_name);
		void write_to_archive(std::string file_name, uint8_t* data, size_t num_bytes);
		std::expected<void, FileArchiveError> write_archive_to_disk(const std::filesystem::path& path);

	private:
		mz_zip_archive m_mz_archive = { 0 };
		std::filesystem::path m_path;
		std::unordered_map<std::string, mz_uint> m_file_indicies;
		std::unordered_map<std::string, std::vector<uint8_t>> m_write_data;
		std::vector<std::string> m_file_names;
	};

} // namespace platform

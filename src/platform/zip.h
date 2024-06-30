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
		CouldNotReopenArchive,
		WritingFileFailed,
	};

	class FileArchive {
	public:
		FileArchive();
		FileArchive(const FileArchive& other) = delete;
		FileArchive(FileArchive&& other);
		FileArchive& operator=(const FileArchive& other) = delete;
		FileArchive& operator=(FileArchive&& other);

		~FileArchive();

		static std::expected<FileArchive, std::string> open_from_file(const std::filesystem::path& path);

		bool is_valid() const;
		const std::vector<std::string> file_names() const;
		std::expected<std::vector<uint8_t>, FileArchiveError> read_from_archive(const std::string& file_name);
		void write_to_archive(std::string file_name, uint8_t* data, size_t num_bytes);
		std::expected<void, FileArchiveError> write_archive_to_disk(const std::filesystem::path& path);
		void close();

	private:
		static std::expected<void, FileArchiveError> _open_from_file_in_write_mode(mz_zip_archive* mz_archive, const std::filesystem::path& path);
		std::expected<void, FileArchiveError> _write_archive_to_disk(mz_zip_archive* temp_mz_archive, const std::filesystem::path& temp_mz_archive_path);

		mz_zip_archive m_mz_archive = { 0 };
		bool m_is_valid = false;
		std::filesystem::path m_path;
		std::unordered_map<std::string, mz_uint> m_file_indicies;
		std::unordered_map<std::string, std::vector<uint8_t>> m_write_data;
		std::vector<std::string> m_file_names;
	};

} // namespace platform

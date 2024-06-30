#include <platform/zip.h>

#include <core/container.h>
#include <platform/logging.h>

#include <algorithm>
#include <cstring>
#include <filesystem>

namespace platform {

	FileArchive::FileArchive()
		: m_mz_archive { 0 }
		, m_is_valid(true) {
		mz_zip_writer_init_heap(&m_mz_archive, 0, 0);
	}

	FileArchive::FileArchive(FileArchive&& other) {
		m_mz_archive = other.m_mz_archive;
		m_mz_archive.m_pIO_opaque = &m_mz_archive;
		m_is_valid = other.m_is_valid;
		m_path = other.m_path;
		m_file_indicies = std::move(other.m_file_indicies);
		m_write_data = std::move(other.m_write_data);
		m_file_names = std::move(other.m_file_names);

		other.m_is_valid = false;
	}

	FileArchive& FileArchive::operator=(FileArchive&& other) {
		m_mz_archive = other.m_mz_archive;
		m_mz_archive.m_pIO_opaque = &m_mz_archive;
		m_is_valid = other.m_is_valid;
		m_path = other.m_path;
		m_file_indicies = std::move(other.m_file_indicies);
		m_write_data = std::move(other.m_write_data);
		m_file_names = std::move(other.m_file_names);

		other.m_is_valid = false;
		return *this;
	}

	FileArchive::~FileArchive() {
		if (m_is_valid) {
			mz_zip_end(&m_mz_archive);
		}
	}

	std::expected<FileArchive, std::string> FileArchive::open_from_file(const std::filesystem::path& path) {
		FileArchive archive;

		/* Read Zip from file */
		archive.m_mz_archive = { 0 };
		bool could_read = mz_zip_reader_init_file(&archive.m_mz_archive, path.string().c_str(), 0);
		if (!could_read) {
			archive.m_is_valid = false;
			mz_zip_error error = mz_zip_get_last_error(&archive.m_mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			return std::unexpected(error_str);
		}

		archive.m_path = path;
		archive.m_is_valid = true;

		/* Read file stats from archive */
		for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&archive.m_mz_archive); i++) {
			mz_zip_archive_file_stat file_stat;
			mz_zip_reader_file_stat(&archive.m_mz_archive, i, &file_stat);
			archive.m_file_names.push_back(file_stat.m_filename);
			archive.m_file_indicies[file_stat.m_filename] = file_stat.m_file_index;
		}

		return archive;
	}

	bool FileArchive::is_valid() const {
		return m_is_valid;
	}

	const std::vector<std::string> FileArchive::file_names() const {
		return m_file_names;
	}

	std::expected<std::vector<uint8_t>, FileArchiveError> FileArchive::read_from_archive(const std::string& file_name) {
		if (!core::container::contains(m_file_names, file_name)) {
			return std::unexpected(FileArchiveError::NoSuchFile);
		}

		/* Try to read data */
		uint8_t* data = nullptr;
		size_t num_bytes;
		data = (uint8_t*)mz_zip_reader_extract_file_to_heap(&m_mz_archive, file_name.c_str(), &num_bytes, 0);
		if (!data) {
			mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			LOG_ERROR("Could not read file \"%s\" inside archive: %s", file_name.c_str(), error_str);
			return std::unexpected(FileArchiveError::ReadFailed);
		}

		/* Copy read data and return it */
		std::vector<uint8_t> copied_data = std::vector<uint8_t>(data, data + num_bytes);
		mz_free((void*)data);
		return copied_data;
	}

	void FileArchive::write_to_archive(std::string file_name, uint8_t* data, size_t num_bytes) {
		/* Copy data */
		std::vector<uint8_t>& buf = m_write_data[file_name];
		buf.insert(buf.end(), data, data + num_bytes);

		/* Save name if new */
		if (!core::container::contains(m_file_names, file_name)) {
			m_file_names.push_back(file_name);
		}
	}

	std::expected<void, FileArchiveError> FileArchive::write_archive_to_disk(const std::filesystem::path& path) {
		if (!m_is_valid) {
			return std::unexpected(FileArchiveError::ArchiveNotValid);
		}

		/* Create name of temp archive */
		std::filesystem::path temp_archive_path = path;
		temp_archive_path.replace_filename(path.stem().string() + "-temp" + path.extension().string());

		/* Create temp archive */
		mz_zip_archive temp_mz_archive;
		std::expected<void, FileArchiveError> open_result = FileArchive::_open_from_file_in_write_mode(&temp_mz_archive, temp_archive_path);
		if (!open_result.has_value()) {
			return open_result;
		}

		/* Write to disk */
		std::expected<void, FileArchiveError> write_result = _write_archive_to_disk(&temp_mz_archive, temp_archive_path);
		if (!write_result.has_value()) {
			return write_result;
		}

		/* Replace old file with temp file */
		mz_zip_writer_end(&temp_mz_archive); // done with temp archive, free it
		mz_zip_reader_end(&m_mz_archive); // close original archive file so we can write to it
		std::filesystem::rename(temp_archive_path, path); // replace old archive with new
		if (!m_path.empty()) {
			// re-open original archive if archive created with file path
			std::expected<FileArchive, std::string> reopen_result = FileArchive::open_from_file(m_path);
			if (reopen_result.has_value()) {
				*this = std::move(reopen_result.value());
				return {};
			}
			else {
				LOG_ERROR("Could not re-open archive: %s", reopen_result.error().c_str());
				m_is_valid = false;
				return std::unexpected(FileArchiveError::CouldNotReopenArchive);
			}
		}
		else {
			// else, just re-initialize the mz_zip_archive
			m_mz_archive = { 0 };
			mz_zip_writer_init_heap(&m_mz_archive, 0, 0);
			return {};
		}
	}

	void FileArchive::close() {
		m_is_valid = false;
		mz_zip_end(&m_mz_archive);
	}

	std::expected<void, FileArchiveError> FileArchive::_open_from_file_in_write_mode(mz_zip_archive* mz_archive, const std::filesystem::path& path) {
		/* Delete archive if it already exists */
		if (std::filesystem::exists(path)) {
			std::filesystem::remove(path);
		}

		/* Create archive */
		*mz_archive = { 0 };
		mz_bool result = mz_zip_writer_init_file(mz_archive, path.string().c_str(), 0);
		if (!result) {
			mz_zip_error error = mz_zip_get_last_error(mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			LOG_ERROR("Could not create new zip file \"%s\": %s", path.string().c_str(), error_str);
			return std::unexpected(FileArchiveError::CouldNotCreateArchive);
		}

		return {};
	}

	std::expected<void, FileArchiveError> FileArchive::_write_archive_to_disk(mz_zip_archive* temp_mz_archive, const std::filesystem::path& temp_mz_archive_path) {
		/* Write new files to archive */
		for (auto& [file_name, buffer] : m_write_data) {
			bool result = mz_zip_writer_add_mem(temp_mz_archive, file_name.c_str(), buffer.data(), buffer.size(), 0);
			if (!result) {
				mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
				const char* error_str = mz_zip_get_error_string(error);
				LOG_ERROR("Could not write file \"%s\" to archive: %s", file_name.c_str(), error_str);
				return std::unexpected(FileArchiveError::WritingFileFailed);
			}
		}

		/* Copy non-modified files from original archive */
		const std::vector<std::string> non_modified_file_names = core::container::filter(m_file_names, [this](const std::string& file_name) {
			return !m_write_data.contains(file_name);
		});
		for (const std::string& file_name : non_modified_file_names) {
			bool result = mz_zip_writer_add_from_zip_reader(temp_mz_archive, &m_mz_archive, m_file_indicies[file_name]);
			if (!result) {
				mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
				const char* error_str = mz_zip_get_error_string(error);
				LOG_ERROR("Could not write file \"%s\" to archive: %s", file_name.c_str(), error_str);
				return std::unexpected(FileArchiveError::WritingFileFailed);
			}
		}

		/* Write archive to disk */
		{
			bool result = mz_zip_writer_finalize_archive(temp_mz_archive);
			if (!result) {
				LOG_ERROR("Could not finalize archive \"%s\"", temp_mz_archive_path.string().c_str());
				return std::unexpected(FileArchiveError::CouldNotWriteArchive);
			}
		}

		return {};
	}

} // namespace platform

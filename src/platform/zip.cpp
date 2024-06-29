#include <platform/zip.h>

#include <platform/logging.h>
#include <util.h>

#include <algorithm>
#include <filesystem>

namespace platform {

	std::optional<std::string> FileArchive::open_from_file(FileArchive* archive, const std::filesystem::path& path) {
		// FIXME: For some reason we can't just return an `mz_zip_archive`
		// inside a std::expected, because when we later use the archive we end
		// up with m_pState in some bad state that causes miniz to assert when
		// trying to read files from the archive. The only safe way to
		// initialize is by-reference. Who knows why, something something UB maybe.

		/* Read Zip from file */
		archive->m_mz_archive = { 0 };
		bool could_read = mz_zip_reader_init_file(&archive->m_mz_archive, path.string().c_str(), 0);
		if (!could_read) {
			mz_zip_error error = mz_zip_get_last_error(&archive->m_mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			return error_str;
		}

		archive->m_path = path;

		/* Read file stats from archive */
		for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&archive->m_mz_archive); i++) {
			mz_zip_archive_file_stat file_stat;
			mz_zip_reader_file_stat(&archive->m_mz_archive, i, &file_stat);
			archive->m_file_names.push_back(file_stat.m_filename);
			archive->m_file_indicies[file_stat.m_filename] = file_stat.m_file_index;
		}

		return {};
	}

	const std::vector<std::string> FileArchive::file_names() const {
		return m_file_names;
	}

	std::expected<std::vector<uint8_t>, FileArchiveError> FileArchive::read_from_archive(const std::string& file_name) {
		if (!util::contains(m_file_names, file_name)) {
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
		if (!util::contains(m_file_names, file_name)) {
			m_file_names.push_back(file_name);
		}
	}

	std::expected<void, FileArchiveError> FileArchive::write_archive_to_disk(const std::filesystem::path& path) {
		/* Create name of temp archive */
		std::filesystem::path temp_archive_path = path;
		temp_archive_path.replace_filename(path.filename().string() + "-temp");

		/* Delete temp archive if already exists */
		if (std::filesystem::exists(temp_archive_path)) {
			std::filesystem::remove(temp_archive_path);
		}

		/* Create temp archive */
		mz_zip_archive temp_mz_archive = { 0 };
		{
			mz_bool result = mz_zip_writer_init_file(&temp_mz_archive, temp_archive_path.string().c_str(), 0);
			if (!result) {
				mz_zip_error error = mz_zip_get_last_error(&temp_mz_archive);
				const char* error_str = mz_zip_get_error_string(error);
				LOG_ERROR("Could not create new zip file \"%s\": %s", temp_archive_path.string().c_str(), error_str);
				return std::unexpected(FileArchiveError::CouldNotCreateArchive);
			}
		}

		/* Write new files to archive */
		for (auto& [file_name, buffer] : m_write_data) {
			bool result = mz_zip_writer_add_mem(&temp_mz_archive, file_name.c_str(), buffer.data(), buffer.size(), 0);
			if (!result) {
				mz_zip_writer_end(&temp_mz_archive);
				mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
				const char* error_str = mz_zip_get_error_string(error);
				LOG_ERROR("Could not write file \"%s\" to archive: %s", file_name.c_str(), error_str);
				return std::unexpected(FileArchiveError::WritingFileFailed);
			}
		}

		/* Copy non-modified files from original archive */
		const std::vector<std::string> non_modified_file_names = util::filter(m_file_names, [this](const std::string& file_name) {
			return !m_write_data.contains(file_name);
		});
		for (const std::string& file_name : non_modified_file_names) {
			bool result = mz_zip_writer_add_from_zip_reader(&temp_mz_archive, &m_mz_archive, m_file_indicies[file_name]);
			if (!result) {
				mz_zip_writer_end(&temp_mz_archive);
				mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
				const char* error_str = mz_zip_get_error_string(error);
				LOG_ERROR("Could not write file \"%s\" to archive: %s", file_name.c_str(), error_str);
				return std::unexpected(FileArchiveError::WritingFileFailed);
			}
		}

		/* Write archive to disk */
		{
			bool result = mz_zip_writer_finalize_archive(&temp_mz_archive);
			if (!result) {
				mz_zip_writer_end(&temp_mz_archive);
				LOG_ERROR("Could not finalize archive \"%s\"", temp_archive_path.string().c_str());
				return std::unexpected(FileArchiveError::CouldNotWriteArchive);
			}
			mz_zip_writer_end(&temp_mz_archive);

			// replace old file with temp
			mz_zip_reader_end(&m_mz_archive);
			std::filesystem::rename(temp_archive_path, path);
		}

		/* Re-open archive */
		FileArchive::open_from_file(this, m_path);

		return {};
	}

} // namespace platform

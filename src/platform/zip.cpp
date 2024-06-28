#include <platform/zip.h>

#include <platform/logging.h>

namespace platform {

	std::optional<std::string> FileArchive::open_from_file(FileArchive* archive, const std::filesystem::path& path) {
		/* Read Zip from file */
		bool could_read = mz_zip_reader_init_file(&archive->m_mz_archive, path.string().c_str(), 0);
		if (!could_read) {
			mz_zip_error error = mz_zip_get_last_error(&archive->m_mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			return error_str;
		}

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

	std::expected<std::vector<uint8_t>, FileArchiveError> FileArchive::read_file(std::string file_name) {
		/* Try to read data */
		char* data = nullptr;
		size_t num_bytes;
		data = (char*)mz_zip_reader_extract_file_to_heap(&m_mz_archive, file_name.c_str(), &num_bytes, 0);
		if (!data) {
			mz_zip_error error = mz_zip_get_last_error(&m_mz_archive);
			const char* error_str = mz_zip_get_error_string(error);
			LOG_ERROR(data, "Could not read file \"%s\" inside archive: %s", file_name.c_str(), error_str);
			return std::unexpected(FileArchiveError::ReadFailed);
		}

		/* Copy read data and return it */
		std::vector<uint8_t> copied_data = std::vector<uint8_t>(data, data + num_bytes);
		mz_free((void*)data);
		return copied_data;
	}

} // namespace platform

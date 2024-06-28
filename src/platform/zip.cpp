#include <platform/zip.h>

namespace platform {

	std::optional<std::string> Archive::open_from_file(Archive* archive, const std::filesystem::path& path) {
		/* Read Zip from file */
		bool could_read = mz_zip_reader_init_file(&archive->m_archive, path.string().c_str(), 0);
		if (!could_read) {
			mz_zip_error error = mz_zip_get_last_error(&archive->m_archive);
			const char* error_str = mz_zip_get_error_string(error);
			return error_str;
		}

		/* Read file stats from archive */
		for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&archive->m_archive); i++) {
			mz_zip_archive_file_stat file_stat;
			mz_zip_reader_file_stat(&archive->m_archive, i, &file_stat);
			archive->m_file_stats.push_back(file_stat);
			archive->m_file_indicies[file_stat.m_filename] = file_stat.m_file_index;
		}

		return {};
	}

} // namespace platform

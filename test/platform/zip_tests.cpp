#include <gtest/gtest.h>

#include <platform/zip.h>

#include <filesystem>

class ZipTests : public testing::Test {
public:
	static std::filesystem::path m_work_directory;
	static std::filesystem::path m_test_archive_path;
	static void SetUpTestSuite() {
		m_work_directory = std::filesystem::current_path();
		m_test_archive_path = m_work_directory / "test/platform/zip_tests.zip";
		ASSERT_TRUE(std::filesystem::is_regular_file(m_test_archive_path)) << "The test zip archive is missing, cannot run zip tests!";
	}
};

std::filesystem::path ZipTests::m_work_directory;
std::filesystem::path ZipTests::m_test_archive_path;

TEST_F(ZipTests, IsValid_DefaultConstructedArchive_IsNotYetValid) {
	platform::FileArchive archive;
	EXPECT_FALSE(archive.is_valid());
}

TEST_F(ZipTests, IsValid_ArchiveLoadedFromZipFile_IfZipFileMissing_ArchiveNotValid) {
	platform::FileArchive archive;
	std::optional<std::string> error = platform::FileArchive::open_from_file(&archive, "asdfg");
	EXPECT_TRUE(error.has_value());
	EXPECT_FALSE(archive.is_valid());
}

TEST_F(ZipTests, IsValid_InitializedArchive_IsValid) {
	platform::FileArchive archive;
	platform::FileArchive::initialize_archive(&archive);
	EXPECT_TRUE(archive.is_valid());
}

TEST_F(ZipTests, IsValid_ArchiveLoadedFromZipFile_IfZipFileExists_ArchiveIsValid) {
	platform::FileArchive archive;
	std::optional<std::string> error = platform::FileArchive::open_from_file(&archive, m_test_archive_path.c_str());
	EXPECT_FALSE(error.has_value()) << "Opening an exising archive should not result in any errors";
	EXPECT_TRUE(archive.is_valid());
}

TEST_F(ZipTests, ReadingFromArchive_TryReadFileNotInTestArchive_GivesError) {
	platform::FileArchive archive;
	platform::FileArchive::open_from_file(&archive, m_test_archive_path.c_str());

	std::expected<std::vector<uint8_t>, platform::FileArchiveError> file_data = archive.read_from_archive("asdfg");
	ASSERT_FALSE(file_data.has_value());
	EXPECT_EQ(file_data.error(), platform::FileArchiveError::NoSuchFile);
}

TEST_F(ZipTests, ReadingFromArchive_TryReadExistingFileInTestArchive_GivesData) {
	platform::FileArchive archive;
	platform::FileArchive::open_from_file(&archive, m_test_archive_path.c_str());

	std::expected<std::vector<uint8_t>, platform::FileArchiveError> file_data = archive.read_from_archive("hello.txt");
	ASSERT_TRUE(file_data.has_value());
	std::string content = std::string(file_data->begin(), file_data->end());
	EXPECT_EQ(content, "Hello zip test!");
}

TEST_F(ZipTests, WriteToArchive_DataWrittenToDisk_CanBeReadBack) {
	platform::FileArchive archive;
	platform::FileArchive::initialize_archive(&archive);

	std::filesystem::path archive_path = m_work_directory / "write_test.zip";
	const char* file_name = "hello.txt";
	const std::string test_data = "Hello data!";

	/* Write data to archive and save to disk */
	archive.write_to_archive(file_name, (uint8_t*)test_data.data(), test_data.size());
	std::expected<void, platform::FileArchiveError> write_result = archive.write_archive_to_disk(archive_path);
	ASSERT_TRUE(write_result.has_value());

	/* Read that data back from disk */
	const std::optional<std::string> open_result = platform::FileArchive::open_from_file(&archive, archive_path);
	ASSERT_FALSE(open_result.has_value());

	const std::expected<std::vector<uint8_t>, platform::FileArchiveError> read_result = archive.read_from_archive(file_name);
	ASSERT_TRUE(read_result.has_value());

	const std::string read_data = std::string(read_result->begin(), read_result->end());
	EXPECT_EQ(test_data, read_data);

	/* Close archive and clean up data on disk */
	archive.close();
	std::filesystem::remove(archive_path);
}

// cannot write to a default constructed archive

// cannot write to a closed archive

#include <gtest/gtest.h>

#include <platform/zip.h>

#include <filesystem>

class ZipTests : public testing::Test {
public:
	static std::filesystem::path m_test_zip_path;
	static void SetUpTestSuite() {
		const std::filesystem::path current_path = std::filesystem::current_path();
		m_test_zip_path = current_path / "test/platform/zip_tests.zip";
		ASSERT_TRUE(std::filesystem::is_regular_file(m_test_zip_path)) << "The test zip archive is missing, cannot run zip tests.";
	}
};

std::filesystem::path ZipTests::m_test_zip_path;

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
	std::optional<std::string> error = platform::FileArchive::open_from_file(&archive, m_test_zip_path.c_str());
	EXPECT_FALSE(error.has_value()) << "Opening an exising archive should not result in any errors";
	EXPECT_TRUE(archive.is_valid());
}

TEST_F(ZipTests, ReadingFromArchive_TryReadFileNotInTestArchive_GivesError) {
	platform::FileArchive archive;
	platform::FileArchive::open_from_file(&archive, m_test_zip_path.c_str());

	std::expected<std::vector<uint8_t>, platform::FileArchiveError> file_data = archive.read_from_archive("asdfg");
	ASSERT_FALSE(file_data.has_value());
	EXPECT_EQ(file_data.error(), platform::FileArchiveError::NoSuchFile);
}

TEST_F(ZipTests, ReadingFromArchive_TryReadExistingFileInTestArchive_GivesData) {
	platform::FileArchive archive;
	platform::FileArchive::open_from_file(&archive, m_test_zip_path.c_str());

	std::expected<std::vector<uint8_t>, platform::FileArchiveError> file_data = archive.read_from_archive("hello.txt");
	ASSERT_TRUE(file_data.has_value());
	std::string content = std::string(file_data->begin(), file_data->end());
	EXPECT_EQ(content, "Hello zip test!");
}

// create new archive, write data, read that data back
// TEST_F(ZipTests, )

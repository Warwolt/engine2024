#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mock_gl_context.h>
#include <test_helper.h>

#include <platform/file/resource_loader.h>

#include <platform/debug/logging.h>

using namespace testing;

static unsigned char g_mock_image_data[1];

class MockResourceFileIO : public platform::IResourceFileIO {
public:
	MOCK_METHOD((std::expected<platform::FontAtlas, platform::ResourceLoadError>), load_font, (std::filesystem::path font_path, uint8_t font_size), (override));
	MOCK_METHOD((std::expected<platform::Image, platform::ResourceLoadError>), load_image, (std::filesystem::path image_path), (override));
};

static unsigned char* _load_image(const std::filesystem::path& image_path) {
	int width, height, num_channels;
	return stbi_load(image_path.string().c_str(), &width, &height, &num_channels, STBI_rgb_alpha);
}

TEST(ResourceLoaderTests, LoadManifest_WithExistingFiles_AreLoaded) {
	MockResourceFileIO mock_file_io;
	testing::MockOpenGLContext mock_gl_context;
	std::filesystem::path working_directory = std::filesystem::current_path();
	platform::ResourceLoader resource_loader(&mock_file_io);
	platform::ResourceManifest manifest = {
		.fonts = { platform::FontDeclaration {
			.name = "test_font",
			.path = working_directory / "test/platform/test_data/test_font.ttf",
			.size = 16,
		} },
		.images = { platform::ImageDeclaration {
			.name = "test_image",
			.path = working_directory / "test/platform/test_data/test_image.png",
		} }
	};

	std::filesystem::path image_path = working_directory / "test/platform/test_data/test_image.png";
	unsigned char* image_data = _load_image(image_path);
	EXPECT_CALL(mock_file_io, load_font).WillRepeatedly(Return(platform::FontAtlas {}));
	EXPECT_CALL(mock_file_io, load_image).WillRepeatedly(Return(ByMove(platform::Image { .data = platform::ImageData { image_data } })));
	EXPECT_CALL(mock_gl_context, add_texture).WillRepeatedly(Return(platform::Texture {}));
	std::shared_ptr<const platform::ResourceLoadProgress> progress = resource_loader.load_manifest(manifest);
	WAIT_FOR(progress->is_done(), std::chrono::seconds(1)) {
		resource_loader.update(&mock_gl_context);
	}

	ASSERT_TRUE(resource_loader.fonts().contains("test_font"));
	ASSERT_TRUE(resource_loader.textures().contains("test_image"));
}

// // Doesn't run in CI
// TEST(ResourceLoaderTests, DISABLED_LoadManifest_WithInvalidPaths_GivesErrors) {
// 	testing::MockOpenGLContext gl_context_mock;
// 	platform::ResourceLoader resource_loader;
// 	platform::ResourceManifest manifest = {
// 		.fonts = { platform::FontDeclaration {
// 			.name = "test_font",
// 			.path = "bad_font_path.ttf",
// 			.size = 16,
// 		} },
// 		.images = { platform::ImageDeclaration {
// 			.name = "test_image",
// 			.path = "bad_image_path.png",
// 		} }
// 	};

// 	std::shared_ptr<const platform::ResourceLoadProgress> progress = resource_loader.load_manifest(manifest);
// 	WAIT_FOR(progress->invalid_paths.size() == 2, std::chrono::seconds(1)) {
// 		fprintf(stderr, "update\n");
// 		resource_loader.update(&gl_context_mock);
// 	}

// 	EXPECT_THAT(progress->invalid_paths, UnorderedElementsAre("bad_font_path.ttf", "bad_image_path.png"));
// }

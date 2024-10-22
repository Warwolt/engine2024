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

static platform::ImageData _load_image(const std::filesystem::path& image_path) {
	int width, height, num_channels;
	return platform::ImageData {
		stbi_load(image_path.string().c_str(), &width, &height, &num_channels, STBI_rgb_alpha)
	};
}

TEST(ResourceLoaderTests, LoadManifest_WithExistingFiles_AreLoaded) {
	MockResourceFileIO mock_file_io;
	testing::MockOpenGLContext mock_gl_context;
	std::filesystem::path working_directory = std::filesystem::current_path();
	std::filesystem::path image_path = working_directory / "test/platform/test_data/test_image.png";
	platform::ResourceLoader resource_loader(&mock_file_io);
	platform::ResourceManifest manifest = {
		.fonts = { platform::FontDeclaration {
			.name = "test_font",
			.path = working_directory / "test/platform/test_data/test_font.ttf",
			.size = 16,
		} },
		.images = { platform::ImageDeclaration {
			.name = "test_image",
			.path = image_path,
		} }
	};

	EXPECT_CALL(mock_file_io, load_font).WillRepeatedly(Return(platform::FontAtlas {}));
	EXPECT_CALL(mock_file_io, load_image).WillRepeatedly(Return(ByMove(platform::Image { .data = _load_image(image_path) })));
	EXPECT_CALL(mock_gl_context, add_texture).WillRepeatedly(Return(platform::Texture {}));
	std::shared_ptr<const platform::ResourceLoadPayload> payload = resource_loader.load_manifest(manifest);
	WAIT_FOR(payload->is_done(), std::chrono::seconds(1)) {
		resource_loader.update(&mock_gl_context);
	}

	ASSERT_TRUE(payload->fonts.contains("test_font"));
	ASSERT_TRUE(payload->textures.contains("test_image"));
}

TEST(ResourceLoaderTests, LoadManifest_WithInvalidPaths_GivesErrors) {
	MockResourceFileIO mock_file_io;
	testing::MockOpenGLContext gl_context_mock;
	platform::ResourceLoader resource_loader(&mock_file_io);
	std::filesystem::path font_path = "bad_font_path.ttf";
	std::filesystem::path image_path = "bad_image_path.png";
	platform::ResourceManifest manifest = {
		.fonts = { platform::FontDeclaration {
			.name = "test_font",
			.path = font_path,
			.size = 16,
		} },
		.images = { platform::ImageDeclaration {
			.name = "test_image",
			.path = image_path,
		} }
	};

	EXPECT_CALL(mock_file_io, load_font).WillRepeatedly(Return(std::unexpected(platform::ResourceLoadError {
		.error_msg = "error message 1",
		.path = font_path,
	})));
	EXPECT_CALL(mock_file_io, load_image).WillRepeatedly(Return(ByMove(std::unexpected(platform::ResourceLoadError {
		.error_msg = "error message 2",
		.path = image_path,
	}))));
	std::shared_ptr<const platform::ResourceLoadPayload> payload = resource_loader.load_manifest(manifest);
	WAIT_FOR(payload->invalid_paths.size() == 2, std::chrono::seconds(1)) {
		resource_loader.update(&gl_context_mock);
	}

	EXPECT_THAT(payload->invalid_paths, UnorderedElementsAre("bad_font_path.ttf", "bad_image_path.png"));
}

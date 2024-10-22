#include <platform/graphics/image.h>

#include <platform/debug/assert.h>

namespace platform {

	std::expected<Image, std::string> read_image(std::filesystem::path path) {
		int width, height, num_channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &num_channels, STBI_rgb_alpha);
		if (!data) {
			return std::unexpected(stbi_failure_reason());
		}
		return Image { ImageData(data), width, height, num_channels };
	}

} // namespace platform

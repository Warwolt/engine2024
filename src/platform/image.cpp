#include <platform/image.h>

#include <platform/debug/assert.h>

namespace platform {

	std::optional<Image> read_image(const char* path) {
		int width, height, num_channels;
		unsigned char* data = stbi_load(path, &width, &height, &num_channels, STBI_rgb_alpha);
		if (!data) {
			return {};
		}
		return Image { ImageData(data), width, height, num_channels };
	}

} // namespace platform

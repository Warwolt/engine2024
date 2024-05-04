#include <platform/image.h>

namespace platform {

	std::optional<Image> read_image(const char* path) {
		int width, height, num_channels;
		unsigned char* data = stbi_load(path, &width, &height, &num_channels, 0);
		if (!data) {
			return {};
		}
		return Image { ImageData(data), width, height, num_channels };
	}

} // namespace platform

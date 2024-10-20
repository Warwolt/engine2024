#include <platform/graphics/image.h>

#include <platform/debug/assert.h>

namespace platform {

	std::optional<Image> read_image(std::filesystem::path path) {
		int width, height, num_channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &num_channels, STBI_rgb_alpha);
		if (!data) {
			return {};
		}
		return Image { ImageData(data), width, height, num_channels };
	}

} // namespace platform

#include <platform/image.h>

#include <platform/assert.h>

namespace platform {

	std::optional<Image> read_image(const char* path) {
		ABORT("FIXME: The image _MUST_ be RGBA. Find some way to force stbi_load to load into this format");
		int width, height, num_channels;
		unsigned char* data = stbi_load(path, &width, &height, &num_channels, 0);
		if (!data) {
			return {};
		}
		return Image { ImageData(data), width, height, num_channels };
	}

} // namespace platform

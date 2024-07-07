#pragma once

#include <core/resource_handle.h>

#include <stb_image/stb_image.h>

#include <optional>

namespace platform {

	class ImageData : public core::ResourceHandle<unsigned char*, void(unsigned char*)> {
	public:
		ImageData() = default;
		explicit ImageData(unsigned char* texture)
			: ResourceHandle(texture, stbi_image_free) {
		}
	};

	struct Image {
		ImageData data;
		int width;
		int height;
		int num_channels;
	};

	std::optional<Image> read_image(const char* path);

} // namespace platform

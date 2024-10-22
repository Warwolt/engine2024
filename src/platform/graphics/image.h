#pragma once

#include <core/resource_handle.h>

#include <stb_image/stb_image.h>

#include <expected>
#include <filesystem>
#include <string>

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

	std::expected<Image, std::string> read_image(std::filesystem::path path);

} // namespace platform

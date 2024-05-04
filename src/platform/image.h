#pragma once

#include <optional>

namespace platform {

	// FIXME: replace `unsigned char* data` with some RAII solution
	// right now we will leak unless we call stbi_free
	struct Image {
		unsigned char* data;
		int width;
		int height;
		int num_channels;
	};

	std::optional<Image> read_image(const char* path);

} // namespace platform

#pragma once

#include <platform/graphics/font.h>
#include <platform/graphics/renderer.h>

#include <string>
#include <unordered_map>

namespace engine {

	struct Resources {
		std::unordered_map<std::string, platform::Texture> textures;
		std::unordered_map<std::string, platform::Font> fonts;
		std::unordered_map<std::string, platform::Canvas> canvases;
	};

} // namespace engine

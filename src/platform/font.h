#pragma once

#include <ft2build.h>

#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <platform/renderer.h>

#include <optional>
#include <stddef.h>
#include <stdint.h>

namespace platform {

	struct Glyph {
		glm::ivec2 atlas_pos;
		glm::ivec2 size;
		glm::ivec2 bearing;
		int advance;
	};

	struct Font {
		static constexpr size_t NUM_GLYPHS = 127;
		Glyph glyphs[NUM_GLYPHS]; // indexed with printable ascii values
		platform::Texture atlas;
	};

	std::optional<Font> add_font(FT_Library ft, const char* font_path, uint8_t font_size);
}

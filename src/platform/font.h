#pragma once

#include <ft2build.h>

#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <platform/texture.h>

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
		Glyph glyphs[NUM_GLYPHS]; // indexed using ascii values
		platform::Texture atlas;
		int line_spacing;
	};

	void set_ft(FT_Library ft);
	FT_Library get_ft();

	bool initialize_fonts();
	void shutdown_fonts();

	std::optional<Font> add_font(const char* font_path, uint8_t font_size);
	void free_font(const Font* font);

}

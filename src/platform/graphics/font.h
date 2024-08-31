#pragma once

#include <ft2build.h>

#include <core/rect.h>

#include <freetype/freetype.h>
#include <glm/glm.hpp>
#include <platform/graphics/texture.h>

#include <optional>
#include <stddef.h>
#include <stdint.h>
#include <string>

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
		size_t size;
		int line_height; // measured from baseline
	};

	void set_ft(FT_Library ft);
	FT_Library get_ft();

	bool initialize_fonts();
	void shutdown_fonts();

	std::optional<Font> add_ttf_font(const char* font_path, uint8_t font_size);
	void free_font(const Font& font);

	core::Rect get_text_bounding_box(const Font& font, const std::string& text);
}

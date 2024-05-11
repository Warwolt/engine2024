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
		Glyph glyphs[NUM_GLYPHS]; // indexed using ascii values
		platform::Texture atlas;
		int line_spacing;
	};

	bool initialize_fonts();
	void deinitialize_fonts();

	std::optional<Font> add_font(const char* font_path, uint8_t font_size);
	void render_character(Renderer* renderer, const Font* font, char character, glm::vec2 pos, glm::vec4 color);
	void render_text(Renderer* renderer, const Font* font, const char* text, glm::vec2 pos, glm::vec4 color);
}

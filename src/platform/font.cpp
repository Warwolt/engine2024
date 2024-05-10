#include <platform/font.h>

// FIXME remove this include
#include <platform/assert.h>

namespace platform {

	struct RGB {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	// TODO make FT_library a global var to make API cleaner
	// no sensible way to pass FT_library around with abstraction leakage towards engine
	Font add_font(FT_Library ft, const char* font_path, uint8_t font_size) {
		Font font;

		/* Load font */
		FT_Face face;
		if (FT_Error error = FT_New_Face(ft, font_path, 0, &face); error != FT_Err_Ok) {
			// FIXME return an error here instead of aborting
			ABORT("FT_New_Face(\"%s\") failed: %s", font_path, FT_Error_String(error));
		}

		/* Set font size */
		int font_upscale = 64;
		FT_Set_Char_Size(face, 0, font_size * font_upscale, 96, 96);

		/* Calculate atlas dimensions to be a square */
		uint32_t glyph_height = (1 + (face->size->metrics.height / font_upscale));
		uint32_t glyph_width = glyph_height / 2; // assume 2:1 ratio
		uint32_t columns = (uint32_t)roundf(sqrtf((float)Font::NUM_GLYPHS * (float)glyph_height / (float)glyph_width));
		uint32_t rows = (uint32_t)roundf((float)Font::NUM_GLYPHS / (float)columns);
		uint32_t texture_width = columns * glyph_width;
		uint32_t texture_height = rows * glyph_height;

		/* Compute glyphs */
		std::vector<uint8_t> glyph_pixels = std::vector<uint8_t>(texture_width * texture_height);
		glm::ivec2 pen = { 0, 1 };
		for (int i = '!'; i < Font::NUM_GLYPHS; i++) {
			// load character
			FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
			FT_Bitmap* bmp = &face->glyph->bitmap;

			// render current glyph
			for (uint32_t row = 0; row < bmp->rows; row++) {
				for (uint32_t col = 0; col < bmp->width; col++) {
					uint32_t x = pen.x + col;
					uint32_t y = pen.y + row;
					glyph_pixels[y * texture_width + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}

			// save glyph info
			font.glyphs[i].atlas_pos = pen;
			font.glyphs[i].size = { bmp->width, bmp->rows };
			font.glyphs[i].bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			font.glyphs[i].advance = face->glyph->advance.x / font_upscale;

			// move pen
			pen.x += bmp->width + 1;
			if (pen.x + bmp->width >= texture_width) {
				pen.x = 0;
				pen.y += face->size->metrics.height / font_upscale + 2;
			}
		}
		FT_Done_Face(face);

		/* Generate glyph texture */
		std::vector<RGB> glyph_rgb = std::vector<RGB>(texture_width * texture_height);
		for (uint32_t y = 0; y < texture_height; y++) {
			uint32_t inv_y = (texture_height - 1) - y;
			for (uint32_t x = 0; x < texture_width; x++) {
				glyph_rgb[inv_y * texture_width + x].r |= glyph_pixels[y * texture_width + x];
				glyph_rgb[inv_y * texture_width + x].g |= glyph_pixels[y * texture_width + x];
				glyph_rgb[inv_y * texture_width + x].b |= glyph_pixels[y * texture_width + x];
			}
		}

		font.atlas = platform::add_texture((uint8_t*)glyph_rgb.data(), texture_width, texture_height);

		return font;
	}

} // namespace platform

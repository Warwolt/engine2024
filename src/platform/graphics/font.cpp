#include <platform/graphics/font.h>

#include <platform/debug/assert.h>
#include <platform/debug/logging.h>

namespace platform {

	static FT_Library g_ft;

	struct RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	void set_ft(FT_Library ft) {
		g_ft = ft;
	}

	FT_Library get_ft() {
		ASSERT(g_ft, "Trying to access g_ft before it's initialized!");
		return g_ft;
	}

	bool initialize_fonts() {
		if (FT_Error error = FT_Init_FreeType(&g_ft); error != FT_Err_Ok) {
			LOG_ERROR("FT_Init_FreeType failed: %s", FT_Error_String(error));
			return false;
		}
		return true;
	}

	void shutdown_fonts() {
		FT_Done_FreeType(g_ft);
	}

	std::optional<Font> add_ttf_font(const char* font_path, uint8_t font_size) {
		Font font;
		font.size = font_size;

		/* Load font */
		FT_Face face;
		if (FT_Error error = FT_New_Face(get_ft(), font_path, 0, &face); error != FT_Err_Ok) {
			LOG_ERROR("FT_New_Face(\"%s\") failed: %s", font_path, FT_Error_String(error));
			return std::nullopt;
		}

		/* Set font size */
		int pixels_per_point = 64;
		FT_Set_Char_Size(face, 0, font_size * pixels_per_point, 96, 96);

		/* Calculate atlas dimensions to be a square */
		uint32_t glyph_height = (1 + (face->size->metrics.height / pixels_per_point));
		uint32_t glyph_width = glyph_height / 2; // assume 2:1 ratio
		uint32_t columns = (uint32_t)roundf(sqrtf((float)Font::NUM_GLYPHS * (float)glyph_height / (float)glyph_width));
		uint32_t rows = (uint32_t)roundf((float)Font::NUM_GLYPHS / (float)columns);
		uint32_t texture_width = columns * glyph_width;
		uint32_t texture_height = rows * glyph_height;

		/* Save line spacing */
		font.line_height = (face->size->metrics.ascender - face->size->metrics.descender) / pixels_per_point;

		/* Compute glyphs */
		std::vector<uint8_t> glyph_pixels = std::vector<uint8_t>(texture_width * texture_height);
		glm::ivec2 pen = { 0, 1 };
		for (int i = ' '; i < Font::NUM_GLYPHS; i++) {
			// load character
			FT_Load_Char(face, i, FT_LOAD_RENDER);
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
			font.glyphs[i].advance = face->glyph->advance.x / pixels_per_point;

			// move pen
			pen.x += bmp->width + 1;
			if (pen.x + bmp->width >= texture_width) {
				pen.x = 0;
				pen.y += face->size->metrics.height / pixels_per_point + 2;
			}
		}
		FT_Done_Face(face);

		/* Generate glyph texture */
		std::vector<RGBA> glyph_rgb = std::vector<RGBA>(texture_width * texture_height);
		for (uint32_t y = 0; y < texture_height; y++) {
			uint32_t inv_y = (texture_height - 1) - y;
			for (uint32_t x = 0; x < texture_width; x++) {
				// Multiply alpha by some amount to match how the reference font arial.ttf renders in MS Paint
				// Without this it seems like we end up rendering the font too dark.
				uint8_t alpha = (uint8_t)std::min(std::roundf(glyph_pixels[y * texture_width + x] * 1.3f), 255.0f);
				glyph_rgb[inv_y * texture_width + x].r = 0xFF;
				glyph_rgb[inv_y * texture_width + x].g = 0xFF;
				glyph_rgb[inv_y * texture_width + x].b = 0xFF;
				glyph_rgb[inv_y * texture_width + x].a = alpha;
			}
		}

		font.atlas = platform::add_texture((uint8_t*)glyph_rgb.data(), texture_width, texture_height);

		return font;
	}

	void free_font(const Font& font) {
		free_texture(font.atlas);
	}

	core::Rect get_text_bounding_box(const Font& font, const std::string& text) {
		glm::vec2 pen = { 0.0f, 0.0f };
		float width = 0.0f;
		const char* chars = text.data();
		for (char character = *chars; character != '\0'; character = *(++chars)) {
			const Glyph& glyph = font.glyphs[character];
			width += glyph.advance;
		}
		return core::Rect {
			.top_left = { 0.0f, -(float)font.size - 1.0f },
			.bottom_right = { width + 2.0f, 2.0f }
		};
	}

} // namespace platform

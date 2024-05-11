#include <platform/font.h>

#include <platform/assert.h>
#include <platform/logging.h>

namespace platform {

	static FT_Library g_ft;

	struct RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	static FT_Library get_ft() {
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

	void deinitialize_fonts() {
		FT_Done_FreeType(g_ft);
	}

	std::optional<Font> add_font(const char* font_path, uint8_t font_size) {
		Font font;

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
		font.line_spacing = face->size->metrics.height / pixels_per_point;

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
				glyph_rgb[inv_y * texture_width + x].r |= glyph_pixels[y * texture_width + x];
				glyph_rgb[inv_y * texture_width + x].g |= glyph_pixels[y * texture_width + x];
				glyph_rgb[inv_y * texture_width + x].b |= glyph_pixels[y * texture_width + x];
				glyph_rgb[inv_y * texture_width + x].a |= glyph_pixels[y * texture_width + x];
			}
		}

		font.atlas = platform::add_texture((uint8_t*)glyph_rgb.data(), texture_width, texture_height);

		return font;
	}

	void render_character(Renderer* renderer, const Font* font, char character, glm::vec2 pos, glm::vec4 color) {
		const platform::Glyph& glyph = font->glyphs[character];

		platform::Rect quad = {
			.top_left = { pos.x, pos.y },
			.bottom_right = { pos.x + glyph.size.x, pos.y + glyph.size.y }
		};

		float u0 = glyph.atlas_pos.x / (float)font->atlas.width;
		float v0 = 1 - (glyph.atlas_pos.y + glyph.size.y) / (float)font->atlas.height;
		float u1 = u0 + glyph.size.x / (float)font->atlas.width;
		float v1 = v0 + glyph.size.y / (float)font->atlas.height;

		platform::FlipRect uv = {
			.bottom_left = { u0, v0 },
			.top_right = { u1, v1 }
		};

		renderer->draw_texture_clipped_with_color(font->atlas, quad, uv, color);
	}

	void render_text(Renderer* renderer, const Font* font, const char* text, glm::vec2 pos, glm::vec4 color) {
		glm::vec2 pen = pos;
		for (char character = *text; character != '\0'; character = *(++text)) {
			const platform::Glyph& glyph = font->glyphs[character];

			if (character != ' ') {
				glm::vec2 glyph_pos = glm::vec2 {
					pen.x,
					pen.y - glyph.bearing.y,
				};
				render_character(renderer, font, character, glyph_pos, color);
			}

			pen.x += glyph.advance;
		}
	}

} // namespace platform

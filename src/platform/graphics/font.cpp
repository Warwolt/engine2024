#include <platform/graphics/font.h>

#include <cstring>
#include <platform/debug/assert.h>
#include <platform/debug/logging.h>
#include <platform/graphics/graphics_context.h>

namespace platform {

	static FT_Library g_ft;

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

	std::expected<FontFace, std::string> load_font_face(std::filesystem::path path) {
		FT_Face face;
		std::string path_str = path.string();

		if (FT_Error error = FT_New_Face(get_ft(), path_str.c_str(), 0, &face); error != FT_Err_Ok) {
			return std::unexpected(FT_Error_String(error));
		}

		return FontFace(face);
	}

	FontAtlas generate_font_atlas(const FontFace& face, uint8_t size) {
		FontAtlas atlas;
		atlas.size = size;

		/* Set font size */
		int pixels_per_point = 64;
		FT_Set_Char_Size(face.get(), 0, size * pixels_per_point, 96, 96);

		/* Calculate atlas dimensions to be a square */
		uint32_t glyph_height = (1 + (face->size->metrics.height / pixels_per_point));
		uint32_t glyph_width = glyph_height / 2; // assume 2:1 ratio
		uint32_t columns = (uint32_t)roundf(sqrtf((float)Font::NUM_GLYPHS * (float)glyph_height / (float)glyph_width));
		uint32_t rows = (uint32_t)roundf((float)Font::NUM_GLYPHS / (float)columns);
		atlas.width = columns * glyph_width;
		atlas.height = rows * glyph_height;

		/* Save line spacing */
		atlas.line_height = (face->size->metrics.ascender - face->size->metrics.descender) / pixels_per_point;

		/* Compute glyphs */
		std::vector<uint8_t> monochrome_pixels = std::vector<uint8_t>(atlas.width * atlas.height);
		glm::ivec2 pen = { 0, 1 };
		for (int i = ' '; i < Font::NUM_GLYPHS; i++) {
			// load character
			FT_Load_Char(face.get(), i, FT_LOAD_RENDER);
			FT_Bitmap* bmp = &face->glyph->bitmap;

			// render current glyph
			for (uint32_t row = 0; row < bmp->rows; row++) {
				for (uint32_t col = 0; col < bmp->width; col++) {
					uint32_t x = pen.x + col;
					uint32_t y = pen.y + row;
					monochrome_pixels[y * atlas.width + x] = bmp->buffer[row * bmp->pitch + col];
				}
			}

			// save glyph info
			atlas.glyphs[i].atlas_pos = pen;
			atlas.glyphs[i].size = { bmp->width, bmp->rows };
			atlas.glyphs[i].bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			atlas.glyphs[i].advance = face->glyph->advance.x / pixels_per_point;

			// move pen
			pen.x += bmp->width + 1;
			if (pen.x + bmp->width >= atlas.width) {
				pen.x = 0;
				pen.y += face->size->metrics.height / pixels_per_point + 2;
			}
		}

		/* Generate pixel data */
		atlas.pixels = std::vector<RGBA>(atlas.width * atlas.height);
		for (size_t y = 0; y < atlas.height; y++) {
			size_t inv_y = (atlas.height - 1) - y;
			for (size_t x = 0; x < atlas.width; x++) {
				// Multiply alpha by some amount to match how the reference font arial.ttf renders in MS Paint
				// Without this it seems like we end up rendering the font too dark.
				uint8_t alpha = (uint8_t)std::min(std::roundf(monochrome_pixels[y * atlas.width + x] * 1.3f), 255.0f);
				atlas.pixels[inv_y * atlas.width + x].r = 0xFF;
				atlas.pixels[inv_y * atlas.width + x].g = 0xFF;
				atlas.pixels[inv_y * atlas.width + x].b = 0xFF;
				atlas.pixels[inv_y * atlas.width + x].a = alpha;
			}
		}

		return atlas;
	}

	std::expected<Font, std::string> add_font(GraphicsContext* graphics, const char* font_path, uint8_t font_size) {
		std::expected<FontFace, std::string> face = load_font_face(font_path);
		if (!face.has_value()) {
			return std::unexpected(face.error());
		}
		FontAtlas atlas = generate_font_atlas(face.value(), font_size);
		Texture texture = graphics->add_texture((uint8_t*)atlas.pixels.data(), atlas.width, atlas.height);
		Font font;
		std::memcpy(font.glyphs, atlas.glyphs, sizeof(Glyph) * Font::NUM_GLYPHS);
		font.atlas = texture;
		font.size = atlas.size;
		font.line_height = atlas.line_height;
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

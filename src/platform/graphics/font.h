#pragma once

#include <ft2build.h>

#include <core/rect.h>
#include <core/resource_handle.h>
#include <platform/graphics/graphics_context.h>
#include <platform/graphics/texture.h>

#include <freetype/freetype.h>
#include <glm/glm.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <stddef.h>
#include <stdint.h>
#include <string>

namespace platform {

	class GraphicsAPI;

	class FontFace : public core::ResourceHandle<FT_Face, FT_Error(FT_Face)> {
	public:
		FontFace() = default;
		explicit FontFace(FT_Face face)
			: ResourceHandle(face, FT_Done_Face) {
		}

		FT_Face& get() {
			return m_resource.value();
		}

		const FT_Face& get() const {
			return m_resource.value();
		}

		FT_Face& operator*() {
			return m_resource.value();
		}

		const FT_Face& operator*() const {
			return m_resource.value();
		}

		FT_Face operator->() {
			return m_resource.value();
		}

		FT_Face operator->() const {
			return m_resource.value();
		}
	};

	struct Glyph {
		glm::ivec2 atlas_pos;
		glm::ivec2 size;
		glm::ivec2 bearing;
		int advance;
	};

	struct RGBA {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	struct FontAtlas {
		static constexpr size_t NUM_GLYPHS = 127;
		Glyph glyphs[NUM_GLYPHS]; // indexed using ascii values
		std::vector<RGBA> pixels;
		size_t size;
		unsigned int width;
		unsigned int height;
		int line_height; // measured from baseline
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

	std::expected<FontFace, std::string> load_font_face(std::filesystem::path path);
	FontAtlas generate_font_atlas(const FontFace& face, uint8_t size);
	std::expected<Font, std::string> add_font(GraphicsContext* graphics, const char* font_path, uint8_t font_size);

	std::optional<Font> add_ttf_font_DEPRECATED(const char* font_path, uint8_t font_size);
	void free_font(const Font& font);

	core::Rect get_text_bounding_box(const Font& font, const std::string& text);
}

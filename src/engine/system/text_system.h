#pragma once

#include <core/container/vector_map.h>
#include <core/newtype.h>
#include <core/rect.h>
#include <platform/graphics/font.h>

#include <glm/vec2.hpp>

#include <expected>
#include <string>

namespace engine {
	DEFINE_NEWTYPE(FontID, int);
	DEFINE_NEWTYPE(TextID, int);
} // namespace engine
DEFINE_NEWTYPE_HASH_IMPL(engine::FontID, int);
DEFINE_NEWTYPE_HASH_IMPL(engine::TextID, int);

namespace platform {

	class OpenGLContext;

}

namespace engine {

	struct TextNode {
		std::string text;
		FontID font_id;
		glm::vec2 position = { 0.0f, 0.0f };
		core::Rect rect;
	};

	class TextSystem {
	public:
		TextSystem() = default;

		TextSystem(const TextSystem&) = delete;
		TextSystem& operator=(const TextSystem&) = delete;

		void shutdown(platform::OpenGLContext* gl_context);

		FontID add_font(platform::Font font);
		TextID add_text_node(FontID font, const std::string& text = "", glm::vec2 position = { 0.0f, 0.0f });
		void remove_text_node(TextID text_id);

		// deprecated: TextSystem should not load any resources, only store them
		std::expected<FontID, std::string> add_font_from_path_DEPRECATED(platform::OpenGLContext* gl_context, const char* font_path, uint8_t font_size);

		const core::vector_map<TextID, TextNode>& text_nodes() const;
		const core::vector_map<FontID, platform::Font>& fonts() const;

		void set_position(TextID id, glm::vec2 position);

	private:
		int m_next_font_id = 0;
		int m_next_text_id = 0;
		core::vector_map<FontID, platform::Font> m_fonts;
		core::vector_map<TextID, TextNode> m_nodes;
	};

} // namespace engine

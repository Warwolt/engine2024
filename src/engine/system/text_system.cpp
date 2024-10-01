#include <engine/system/text_system.h>

#include <platform/debug/assert.h>

namespace engine {

	TextSystem::~TextSystem() {
		for (const auto& [id, font] : m_fonts) {
			platform::free_font(font);
		}
	}

	std::expected<FontID, std::string> TextSystem::add_font(platform::GraphicsContext* graphics, const char* font_path, uint8_t font_size) {
		std::expected<platform::Font, std::string> font = platform::add_font(graphics, font_path, font_size);
		if (!font.has_value()) {
			return std::unexpected(font.error());
		}
		const FontID id = FontID(m_next_font_id++);
		m_fonts.insert({ id, font.value() });
		return id;
	}

	TextID TextSystem::add_text_node(FontID font, const std::string& text, glm::vec2 position) {
		ASSERT(m_fonts.contains(font), "Cannot create text node for font with id %d", font.value);

		const TextID id = TextID(m_next_text_id++);
		TextNode node = TextNode {
			.text = text,
			.font_id = font,
			.position = position,
			.rect = platform::get_text_bounding_box(m_fonts[font], text) + position,
		};

		m_nodes.insert({ id, node });

		return id;
	}

	void TextSystem::remove_text_node(TextID text_id) {
		m_nodes.erase(text_id);
	}

	const core::VecMap<TextID, TextNode>& TextSystem::text_nodes() const {
		return m_nodes;
	}

	const core::VecMap<FontID, platform::Font>& TextSystem::fonts() const {
		return m_fonts;
	}

	void TextSystem::set_position(TextID id, glm::vec2 position) {
		m_nodes[id].position = position;
	}

} // namespace engine

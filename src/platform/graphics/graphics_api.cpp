#include <platform/graphics/graphics_api.h>

namespace platform {

	bool GraphicsAPI::has_commands() {
		return !m_commands.empty();
	}

	std::vector<GraphicsCommand> GraphicsAPI::drain_commands() {
		return std::exchange(m_commands, {});
	}

	void GraphicsAPI::add_texture(
		const unsigned char* data,
		int width,
		int height,
		TextureWrapping wrapping,
		TextureFilter filter,
		std::function<void(Texture)> on_texture_created
	) {
		m_commands.push_back(cmd::texture::AddTexture {
			data,
			width,
			height,
			wrapping,
			filter,
			on_texture_created,
		});
	}

} // namespace platform

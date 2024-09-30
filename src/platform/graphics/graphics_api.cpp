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
		m_commands.push_back(graphics_cmd::AddTexture {
			data,
			width,
			height,
			wrapping,
			filter,
			on_texture_created,
		});
	}

	void GraphicsAPI::free_texture(Texture texture) {
		m_commands.push_back(graphics_cmd::FreeTexture {
			texture,
		});
	}

} // namespace platform

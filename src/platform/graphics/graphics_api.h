#pragma once

#include <core/tagged_variant.h>

#include <platform/graphics/texture.h>

#include <functional>

namespace platform {

	enum class GraphicsCommandType {
		// texture
		AddTexture,
		// SetTextureWrapping,
		// SetTextureFilter,
		// FreeTexture,
	};

	namespace graphics_cmd {

		struct AddTexture {
			static constexpr auto TAG = GraphicsCommandType::AddTexture;
			const unsigned char* data;
			int width;
			int height;
			TextureWrapping wrapping;
			TextureFilter filter;
			std::function<void(Texture)> on_texture_created;
		};

	}

	using GraphicsCommand = core::TaggedVariant<
		GraphicsCommandType,
		graphics_cmd::AddTexture>;

	class GraphicsAPI {
	public:
		bool has_commands();
		std::vector<GraphicsCommand> drain_commands();

		void add_texture(
			const unsigned char* data,
			int width,
			int height,
			TextureWrapping wrapping = TextureWrapping::ClampToEdge,
			TextureFilter filter = TextureFilter::Nearest,
			std::function<void(Texture)> on_texture_created = [](Texture) {}
		);
		// void set_texture_wrapping(Texture texture, TextureWrapping wrapping);
		// void set_texture_filter(Texture texture, TextureFilter filter);
		// void free_texture(Texture texture);

	private:
		std::vector<GraphicsCommand> m_commands;
	};

} // namespace platform

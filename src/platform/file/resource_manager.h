#pragma once

#include <core/container/vec_map.h>
#include <platform/graphics/font.h>
#include <platform/graphics/gl_context.h>
#include <platform/graphics/image.h>
#include <platform/graphics/texture.h>

#include <expected>
#include <filesystem>
#include <future>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace platform {

	struct FontDeclaration {
		std::string name;
		std::filesystem::path path;
		uint8_t size;
	};

	struct ImageDeclaration {
		std::string name;
		std::filesystem::path path;
	};

	struct ResourceManifest {
		std::vector<FontDeclaration> fonts;
		std::vector<ImageDeclaration> images;
	};

	struct ResourceLoadProgress {
		size_t total_num_fonts = 0;
		size_t total_num_images = 0;
		size_t num_loaded_fonts = 0;
		size_t num_loaded_images = 0;
		std::vector<std::filesystem::path> invalid_paths;

		size_t total_num_resources() const {
			return total_num_fonts + total_num_images;
		}

		size_t num_loaded_resources() const {
			return num_loaded_fonts + num_loaded_images;
		}

		bool is_done() const {
			return num_loaded_resources() == total_num_resources();
		}

		bool has_errors() const {
			return !invalid_paths.empty();
		}
	};

	class ResourceManager {
	public:
		std::shared_ptr<const ResourceLoadProgress> load_manifest(const ResourceManifest& manifest);
		void update(platform::OpenGLContext* gl_context);

		const core::VecMap<std::string, platform::Font>& fonts() const;
		const core::VecMap<std::string, platform::Texture>& textures() const;

	private:
		struct NamedFontAtlas {
			std::string name;
			platform::FontAtlas atlas;
		};
		struct NamedImage {
			std::string name;
			platform::Image image;
		};
		struct LoadError {
			std::string error_msg;
			std::filesystem::path path;
		};
		using LoadFontResult = std::expected<NamedFontAtlas, LoadError>;
		using LoadImageResult = std::expected<NamedImage, LoadError>;

		struct ResourceLoadJob {
			std::vector<std::future<LoadFontResult>> font_batch;
			std::vector<std::future<LoadImageResult>> image_batch;
			std::shared_ptr<ResourceLoadProgress> progress;
		};

		std::vector<ResourceLoadJob> m_jobs;
		core::VecMap<std::string, platform::Font> m_fonts;
		core::VecMap<std::string, platform::Texture> m_textures;

		static LoadFontResult _load_font(const FontDeclaration& font_decl);
		static LoadImageResult _load_image(const ImageDeclaration& image_decl);
		static void _process_fonts(
			std::vector<std::future<LoadFontResult>>* font_batch,
			core::VecMap<std::string, platform::Font>* fonts,
			platform::OpenGLContext* gl_context,
			ResourceLoadProgress* progress
		);
		static void _process_images(
			std::vector<std::future<LoadImageResult>>* image_batch,
			core::VecMap<std::string, platform::Texture>* textures,
			platform::OpenGLContext* gl_context,
			ResourceLoadProgress* progress
		);
	};

} // namespace platform

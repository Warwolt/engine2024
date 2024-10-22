#include <platform/file/resource_manager.h>

#include <core/future.h>
#include <platform/debug/logging.h>

namespace platform {

	std::shared_ptr<const ResourceLoadProgress> ResourceManager::load_manifest(const ResourceManifest& manifest) {
		auto progress = std::make_shared<ResourceLoadProgress>(ResourceLoadProgress {
			.total_num_fonts = manifest.fonts.size(),
			.total_num_images = manifest.images.size(),
		});

		m_jobs.push_back(ResourceLoadJob {
			.font_batch = core::batch_async(std::launch::async, manifest.fonts, _load_font),
			.image_batch = core::batch_async(std::launch::async, manifest.images, _load_image),
			.progress = progress,
		});

		return progress;
	}

	void ResourceManager::update(platform::OpenGLContext* gl_context) {
		for (ResourceLoadJob& job : m_jobs) {
			_process_fonts(&job.font_batch, &m_fonts, gl_context, job.progress.get());
			_process_images(&job.image_batch, &m_textures, gl_context, job.progress.get());
		}
	}

	const core::vector_map<std::string, platform::Font>& ResourceManager::fonts() const {
		return m_fonts;
	}

	const core::vector_map<std::string, platform::Texture>& ResourceManager::textures() const {
		return m_textures;
	}

	ResourceManager::LoadFontResult ResourceManager::_load_font(const FontDeclaration& font_decl) {
		std::expected<platform::FontFace, std::string> font_face = platform::load_font_face(font_decl.path);
		if (font_face.has_value()) {
			platform::FontAtlas atlas = platform::generate_font_atlas(font_face.value(), font_decl.size);
			return NamedFontAtlas { font_decl.name, atlas };
		}
		else {
			std::string error_msg = std::format(
				"Couldn't load font in manifest! name = \"{}\", path = \"{}\", size = {}. error: {}",
				font_decl.name,
				font_decl.path.string(),
				font_decl.size,
				font_face.error()
			);
			return std::unexpected(LoadError { error_msg, font_decl.path });
		}
	};

	ResourceManager::LoadImageResult ResourceManager::_load_image(const ImageDeclaration& image_decl) {
		if (std::optional<platform::Image> image = platform::read_image(image_decl.path)) {
			return NamedImage { image_decl.name, std::move(image.value()) };
		}
		else {
			std::string error_msg = std::format(
				"Couldn't load image in manifest! name = {}, path = {}",
				image_decl.name.c_str(),
				image_decl.path.string()
			);
			return std::unexpected(LoadError { error_msg, image_decl.path });
		}
	};

	void ResourceManager::_process_fonts(
		std::vector<std::future<LoadFontResult>>* font_batch,
		core::vector_map<std::string, platform::Font>* fonts,
		platform::OpenGLContext* gl_context,
		ResourceLoadProgress* progress
	) {
		for (const LoadFontResult& result : core::get_ready_batch_values(*font_batch)) {
			if (result.has_value()) {
				const auto& [name, atlas] = result.value();
				fonts->insert({ name, platform::create_font_from_atlas(gl_context, atlas) });
				progress->num_loaded_fonts++;
			}
			else {
				auto& [error_msg, invalid_path] = result.error();
				progress->invalid_paths.push_back(invalid_path);
				LOG_ERROR("%s", error_msg.c_str());
			}
		}
	}

	void ResourceManager::_process_images(
		std::vector<std::future<LoadImageResult>>* image_batch,
		core::vector_map<std::string, platform::Texture>* textures,
		platform::OpenGLContext* gl_context,
		ResourceLoadProgress* progress
	) {
		for (const LoadImageResult& result : core::get_ready_batch_values(*image_batch)) {
			if (result.has_value()) {
				const auto& [name, image] = result.value();
				textures->insert({ name, gl_context->add_texture(image.data.get(), image.width, image.height) });
				progress->num_loaded_images++;
			}
			else {
				auto& [error_msg, invalid_path] = result.error();
				progress->invalid_paths.push_back(invalid_path);
				LOG_ERROR("%s", error_msg.c_str());
			}
		}
	}

} // namespace platform

#include <platform/file/resource_loader.h>

#include <core/future.h>
#include <platform/debug/logging.h>

namespace platform {

	std::expected<platform::FontAtlas, ResourceLoadError> ResourceFileIO::load_font(std::filesystem::path font_path, uint8_t font_size) {
		std::expected<platform::FontFace, std::string> font_face = platform::load_font_face(font_path);
		if (font_face.has_value()) {
			return platform::generate_font_atlas(font_face.value(), font_size);
		}
		else {
			std::string error_msg = std::format(
				"Couldn't load font! path = \"{}\", size = {}. error: {}",
				font_path.string(),
				font_size,
				font_face.error()
			);
			return std::unexpected(ResourceLoadError { error_msg, font_path });
		}
	}

	std::expected<platform::Image, ResourceLoadError> ResourceFileIO::load_image(std::filesystem::path image_path) {
		std::expected<platform::Image, std::string> image = platform::read_image(image_path);
		if (image.has_value()) {
			return std::move(image.value());
		}
		else {
			std::string error_msg = std::format(
				"Couldn't load image! path = {}, error: {}",
				image_path.string(),
				image.error()
			);
			return std::unexpected(ResourceLoadError { error_msg, image_path });
		}
	}

	ResourceLoader::ResourceLoader(IResourceFileIO* file_io)
		: m_file_io(file_io) {
	}

	std::shared_ptr<const ResourceLoadProgress> ResourceLoader::load_manifest(const ResourceManifest& manifest) {
		auto progress = std::make_shared<ResourceLoadProgress>(ResourceLoadProgress {
			.total_num_fonts = manifest.fonts.size(),
			.total_num_images = manifest.images.size(),
		});

		m_jobs.push_back(ResourceLoadJob {
			.font_batch = core::batch_async(std::launch::async, manifest.fonts, [file_io = m_file_io](const FontDeclaration& font_decl) -> LoadFontResult {
				std::expected<platform::FontAtlas, ResourceLoadError> result = file_io->load_font(font_decl.path, font_decl.size);
				if (result.has_value()) {
					return NamedFontAtlas { .name = font_decl.name, .atlas = result.value() };
				}
				return std::unexpected(result.error());
			}),
			.image_batch = core::batch_async(std::launch::async, manifest.images, [file_io = m_file_io](const ImageDeclaration& image_decl) -> LoadImageResult {
				std::expected<platform::Image, ResourceLoadError> result = file_io->load_image(image_decl.path);
				if (result.has_value()) {
					return NamedImage { .name = image_decl.name, .image = std::move(result.value()) };
				}
				return std::unexpected(result.error());
			}),
			.progress = progress,
		});

		return progress;
	}

	void ResourceLoader::update(platform::OpenGLContext* gl_context) {
		for (ResourceLoadJob& job : m_jobs) {
			_process_fonts(&job.font_batch, &m_fonts, gl_context, job.progress.get());
			_process_images(&job.image_batch, &m_textures, gl_context, job.progress.get());
		}
	}

	const core::vector_map<std::string, platform::Font>& ResourceLoader::fonts() const {
		return m_fonts;
	}

	const core::vector_map<std::string, platform::Texture>& ResourceLoader::textures() const {
		return m_textures;
	}

	void ResourceLoader::_process_fonts(
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

	void ResourceLoader::_process_images(
		std::vector<std::future<LoadImageResult>>* image_batch,
		core::vector_map<std::string, platform::Texture>* textures,
		platform::OpenGLContext* gl_context,
		ResourceLoadProgress* progress
	) {
		for (const LoadImageResult& result : core::get_ready_batch_values(*image_batch)) {
			if (result.has_value()) {
				const auto& [name, image] = result.value();
				platform::Texture texture = gl_context->add_texture(image.data.get(), image.width, image.height);
				textures->insert({ name, texture });
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

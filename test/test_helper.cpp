#include <test_helper.h>

#include <platform/graphics/gl_context.h>

namespace testing {

	class MockOpenGLContext : public platform::OpenGLContext {
	public:
		MockOpenGLContext()
			: platform::OpenGLContext(SDL_GLContext { nullptr }) {}
		platform::Texture add_texture(
			const unsigned char* /*data*/,
			int /*width*/,
			int /*height*/,
			platform::TextureWrapping /*wrapping*/ = platform::TextureWrapping::ClampToEdge,
			platform::TextureFilter /*filter*/ = platform::TextureFilter::Nearest
		) override {
			return platform::Texture {};
		}

		void set_texture_wrapping(platform::Texture /*texture*/, platform::TextureWrapping /*wrapping*/) override {}
		void set_texture_filter(platform::Texture /*texture*/, platform::TextureFilter /*filter*/) override {}
		void free_texture(platform::Texture /*texture*/) override {}

		platform::Canvas add_canvas(int /*width*/, int /*height*/, platform::TextureWrapping /*wrapping*/ = platform::TextureWrapping::ClampToEdge, platform::TextureFilter /*filter*/ = platform::TextureFilter::Nearest) override {
			return platform::Canvas { 0 };
		}
		void free_canvas(platform::Canvas /*canvas*/) override {}

		std::expected<platform::ShaderProgram, platform::ShaderProgramError> add_shader_program(const char* /*vertex_src*/, const char* /*fragment_src*/) override {
			return platform::ShaderProgram { 0 };
		}
		void free_shader_program(const platform::ShaderProgram& /*shader_program*/) override {}
	};

	MockOpenGLContext g_gl_context_mock;

	platform::OpenGLContext* gl_context() {
		return &g_gl_context_mock;
	}

} // namespace testing

#pragma once

#include <gmock/gmock.h>

#include <platform/graphics/gl_context.h>

namespace testing {

	class MockOpenGLContext : public platform::OpenGLContext {
	public:
		MockOpenGLContext()
			: platform::OpenGLContext(SDL_GLContext { nullptr }) {}

		MOCK_METHOD(platform::Texture, add_texture, (const unsigned char* data, int width, int height, platform::TextureWrapping wrapping, platform::TextureFilter filter), (override));
		MOCK_METHOD(void, set_texture_wrapping, (platform::Texture texture, platform::TextureWrapping wrapping), (override));
		MOCK_METHOD(void, set_texture_filter, (platform::Texture texture, platform::TextureFilter filter), (override));
		MOCK_METHOD(void, free_texture, (platform::Texture texture), (override));
		MOCK_METHOD(platform::Canvas, add_canvas, (int width, int height, platform::TextureWrapping wrapping, platform::TextureFilter filter), (override));
		MOCK_METHOD(void, free_canvas, (platform::Canvas canvas), (override));
		MOCK_METHOD((std::expected<platform::ShaderProgram, platform::ShaderProgramError>), add_shader_program, (const char* vertex_src, const char* fragment_src), (override));
		MOCK_METHOD(void, free_shader_program, (const platform::ShaderProgram& shader_program), (override));
	};

} // namespace testing

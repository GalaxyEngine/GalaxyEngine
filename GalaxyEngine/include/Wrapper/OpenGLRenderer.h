#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Wrapper {
		class OpenGLRenderer : public Renderer
		{
		public:
			OpenGLRenderer();
			OpenGLRenderer& operator=(const OpenGLRenderer& other) = default;
			OpenGLRenderer(const OpenGLRenderer&) = default;
			OpenGLRenderer(OpenGLRenderer&&) noexcept = default;
			~OpenGLRenderer() override;

			void Initialize() override;
			void EnableDebugOutput() override;

			void Viewport(const Vec2i& pos, const Vec2i& size) override;
			void ClearColorAndBuffer(const Vec4f& color) override;

			// === Texture === //
			void CreateTexture(Resource::Texture* texture) override;
			void DestroyTexture(Resource::Texture* texture) override;

			void BindTexture(Resource::Texture* texture, uint32_t id = 0) override;
			void UnbindTexture() override;

			uint32_t TextureFormatToAPI(Resource::TextureFormat format) override;
			uint32_t TextureFilteringToAPI(Resource::TextureFiltering filtering) override;

			// === Shader === //
			bool CompileVertexShader(Resource::VertexShader* vertex) override;
			bool CompileFragmentShader(Resource::FragmentShader* fragment) override;
			bool LinkShaders(Resource::Shader* shader) override;

			void UseShader(Resource::Shader* shader) override;

			int GetShaderLocation(uint32_t id, const std::string& locationName) override;

			void ShaderSendInt(uint32_t location, int value) override;
			void ShaderSendFloat(uint32_t location, float value) override;
			void ShaderSendDouble(uint32_t location, double value) override;
			void ShaderSendVec2f(uint32_t location, const Vec2f& value) override;
			void ShaderSendVec3f(uint32_t location, const Vec3f& value) override;
			void ShaderSendVec4f(uint32_t location, const Vec4f& value) override;
			void ShaderSendVec2i(uint32_t location, const Vec2i& value) override;
			void ShaderSendVec3i(uint32_t location, const Vec3i& value) override;
			void ShaderSendVec4i(uint32_t location, const Vec4i& value) override;
			void ShaderSendMat4(uint32_t location, const Mat4& value, bool transpose = false) override;

			// === Buffers === //
			void CreateVertexArray(uint32_t& vao) override;
			void BindVertexArray(uint32_t vao) override;
			void UnbindVertexArray() override;

			void CreateAndFillVertexBuffer(uint32_t& vbo, const std::vector<Vec3f>& positions, const std::vector<Vec2f>& textureUVs, const std::vector<Vec3f>& normals);
			void CreateVertexBuffer(uint32_t& vbo, const void* data, size_t dataSize) override;
			void BindVertexBuffer(uint32_t vbo) override;
			void UnbindVertexBuffer() override;

			void CreateIndexBuffer(uint32_t& ebo, const void* data, size_t dataSize) override;
			void BindIndexBuffer(uint32_t ebo) override;
			void VertexAttribPointer(uint32_t index, int size, int stride, const void* pointer) override;

			void DrawElement(size_t count) override;
			void DrawArrays(size_t start, size_t count) override;

			// === Render Buffers === //
			void CreateRenderBuffer(Render::Framebuffer* framebuffer) override;
			void BindRenderBuffer(Render::Framebuffer* framebuffer) override;
			void UnbindRenderBuffer(Render::Framebuffer* framebuffer) override;
			void DeleteRenderBuffer(Render::Framebuffer* framebuffer) override;
			void ResizeRenderBuffer(Render::Framebuffer* framebuffer, const Vec2i& size) override;

			void ActiveDepth(bool active = true) override;
			void SetDepthRange(float _near, float _far) override;

			Vec4f ReadPixelColor(const Vec2f& mousePos) override;

			// Debug
			void DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color = Vec4f(1), float lineWidth = 1.f) override;
		private:

		};
	}
}
#include "Wrapper/OpenGLRenderer.inl" 

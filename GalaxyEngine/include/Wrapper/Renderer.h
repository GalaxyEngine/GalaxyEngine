#pragma once
#include "GalaxyAPI.h"
#include <vector>
#include <string>
#include <galaxymath/Maths.h>

#include "Utils/Type.h"

namespace GALAXY
{
	namespace Resource {
		class Cubemap;
		struct Uniform;
		enum class TextureFormat;
		enum class TextureWrapping;
		enum class TextureFiltering;
		class Texture;
		class Shader;
		class VertexShader;
		class GeometryShader;
		class FragmentShader;
	}
	namespace Render
	{
		class Framebuffer;

		using VertexBuffer = uint32_t;
		using VertexArray = uint32_t;
		using IndexBuffer = uint32_t;

		enum class RenderType
		{
			Default,
			Picking,
			Outline
		};
	}
	namespace Component
	{
		class Transform;
	}
	namespace Wrapper
	{
		struct Image;

		enum class RenderAPI
		{
			OPENGL,
			VULKAN,
			DIRECTX
		};
		

		class GALAXY_API Renderer
		{
		public:
			Renderer() {}
			Renderer& operator=(const Renderer& other) = default;
			Renderer(const Renderer&) = default;
			Renderer(Renderer&&) noexcept = default;
			virtual ~Renderer() {}

			static void CreateInstance(RenderAPI renderAPI);
			static Renderer* GetInstance() { return m_instance.get(); }

			virtual void Initialize() {}
			virtual void EnableDebugOutput() {}
			inline bool IsInitalized() const { return p_initalized; }

			inline void SetRenderingType(const Render::RenderType value) { p_renderType = value; }
			inline Render::RenderType GetRenderType() const { return p_renderType; }

			virtual void Viewport(const Vec2i& pos, const Vec2i& size) {}
			virtual void ClearColorAndBuffer(const Vec4f& color) {}

			// === Texture === //
			virtual void CreateTexture(Resource::Texture* texture) {}
			virtual void SetTextureWrapping(Resource::Texture* texture, Resource::TextureWrapping wrapping) {}
			virtual void SetTextureFiltering(Resource::Texture* texture, Resource::TextureFiltering filtering) {}
			virtual void DestroyTexture(Resource::Texture* texture) {}

			virtual void BindTexture(Resource::Texture* texture, uint32_t id = 0) {}
			virtual void UnbindTexture() {}

			// === Cube map === //
			virtual void CreateCubemap(Resource::Cubemap* cubemap) {}
			virtual void SetCubemapFace(uint32_t face, const Wrapper::Image &image) {}
			virtual void SetCubemapParameters() {}
			virtual void DestroyCubemap(Resource::Cubemap* cubemap) {}

			virtual void BindCubemap(Resource::Cubemap* cubemap, uint32_t id = 0) {}
			virtual void UnbindCubemap() {}

			virtual uint32_t TextureFormatToAPI(Resource::TextureFormat format) { return -1; }
			virtual uint32_t TextureWrappingToAPI(Resource::TextureWrapping filtering) { return -1; }
			virtual uint32_t TextureFilteringToAPI(Resource::TextureFiltering filtering) { return -1; }

			// === Shader === //
			inline virtual bool CompileVertexShader(Resource::VertexShader* vertex) { return false; }
			inline virtual bool CompileFragmentShader(Resource::FragmentShader* fragment) { return false; }
			inline virtual bool LinkShaders(Resource::Shader* shader) { return false; }

			virtual void UseShader(Resource::Shader* shader) {}

			virtual int GetShaderLocation(uint32_t id, const std::string& locationName) { return -1; }
			virtual UMap<std::string, Resource::Uniform> GetShaderUniforms(Resource::Shader* shader);

			virtual void ShaderSendInt(uint32_t location, int value) {}
			virtual void ShaderSendFloat(uint32_t location, float value) {}
			virtual void ShaderSendDouble(uint32_t location, double value) {}
			virtual void ShaderSendVec2f(uint32_t location, const Vec2f& value) {}
			virtual void ShaderSendVec3f(uint32_t location, const Vec3f& value) {}
			virtual void ShaderSendVec4f(uint32_t location, const Vec4f& value) {}
			virtual void ShaderSendVec2i(uint32_t location, const Vec2i& value) {}
			virtual void ShaderSendVec3i(uint32_t location, const Vec3i& value) {}
			virtual void ShaderSendVec4i(uint32_t location, const Vec4i& value) {}
			virtual void ShaderSendMat4(uint32_t location, const Mat4& value, bool transpose = false) {}

			// === Buffers === //
			virtual void CreateVertexArray(uint32_t& vao) {}
			virtual void BindVertexArray(uint32_t vao) {}
			virtual void UnbindVertexArray() {}

			virtual void CreateAndFillVertexBuffer(Render::VertexBuffer& vbo, const std::vector<Vec3f>& positions, const std::vector<Vec2f>& textureUVs, const std::vector<Vec3f>& normals) {}
			virtual void CreateVertexBuffer(Render::VertexBuffer& vbo, const void* data, size_t dataSize) {}
			virtual void BindVertexBuffer(Render::VertexBuffer vbo) {}
			virtual void UnbindVertexBuffer() {}

			virtual void CreateIndexBuffer(Render::IndexBuffer& ebo, const void* data, size_t dataSize) {}
			virtual void BindIndexBuffer(Render::IndexBuffer ebo) {}
			virtual void VertexAttribPointer(uint32_t index, int size, int stride, const void* pointer) {}

			virtual void DrawElement(size_t count) {}
			virtual void DrawArrays(size_t start, size_t count) {}

			virtual void CreateDynamicVertexBuffer(Render::VertexArray& vao, Render::VertexBuffer& vbo, size_t dataSize, size_t numVertices) {}

			// === Render Buffers === //
			virtual void CreateRenderBuffer(Render::Framebuffer* framebuffer) {}
			virtual void BindRenderBuffer(Render::Framebuffer* framebuffer) {}
			virtual void UnbindRenderBuffer(Render::Framebuffer* framebuffer) {}
			virtual void DeleteRenderBuffer(Render::Framebuffer* framebuffer) {}
			virtual void ResizeRenderBuffer(Render::Framebuffer* framebuffer, const Vec2i& size) {}

			virtual void SetViewport(const Vec2i& size) {}

			virtual void EnableDepth(bool active = true) {}
			virtual void SetDepthRange(float _near, float _far) {}

			virtual Vec4f ReadPixelColor(const Vec2f& mousePos) { return Vec4f(0, 0, 0, 0); }
			virtual void ReadPixels(const Vec2i& size, unsigned char*& data) { }
			virtual void ReadPixels(Resource::Texture* texture, unsigned char*& data) { }

			// Debug
			virtual void DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color = Vec4f(1), float lineWidth = 1.f) {}
			virtual void DrawWireCube(const Vec3f& pos, const Vec3f& size, const Vec4f& color = Vec4f(1), float lineWidth = 1.f);
			virtual void DrawWireCube(const Vec3f& pos, const Vec3f& size, const Quat& rotation, const Vec4f& color = Vec4f(1), float lineWidth = 1.f);
			virtual void DrawWireCube(Component::Transform* transform, const Vec4f& color = Vec4f(1), float lineWidth = 1.f);
			virtual void DrawWireCircle(const Vec3f& pos, const Vec3f& normal, float radius, int numSegments = 32, Vec4f color = Vec4f(1), float lineWidth = 1.f);
			virtual void DrawWireCone(const Vec3f& pos, const Quat& rotation, float topRadius, float angle, float height = 25.f, const Vec4f& color = Vec4f(1), float lineWidth = 1.f);

			virtual int GetErrorCode() {return 0;}
		protected:
			bool p_initalized = false;
			Render::RenderType p_renderType = Render::RenderType::Default;
		private:
			static std::unique_ptr<Renderer> m_instance;
		};

	}
}

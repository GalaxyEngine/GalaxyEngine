#pragma once
#include "GalaxyAPI.h"
#include <vector>
#include <string>
namespace GALAXY 
{
	namespace Resource { 
		enum class TextureFormat;
		enum class TextureFiltering;
		class Texture;
		class Shader;
		class VertexShader;
		class GeometryShader;
		class FragmentShader;
	}
	namespace Wrapper 
	{
		enum class RenderAPI
		{
			OPENGL,
			VULKAN,
			DIRECTX
		};


		class Renderer
		{
		public:
			Renderer() {}
			Renderer& operator=(const Renderer& other) = default;
			Renderer(const Renderer&) = default;
			Renderer(Renderer&&) noexcept = default;
			virtual ~Renderer() {}

			static void CreateInstance(RenderAPI renderAPI);
			static Renderer* GetInstance() { return m_instance.get(); }

			virtual void Initalize() {}
			virtual void EnableDebugOutput() {}
			bool IsInitalized() { return p_initalized; }

			virtual void Viewport(const Vec2i& pos, const Vec2i& size) {}
			virtual void ClearColorAndBuffer(const Vec4f& color) {}

			// === Texture === //
			virtual void CreateTexture(Resource::Texture* texture) {}
			virtual void DestroyTexture(Resource::Texture* texture) {}

			virtual uint32_t TextureFormatToAPI(Resource::TextureFormat format) { return -1; }
			virtual uint32_t TextureFilteringToAPI(Resource::TextureFiltering filtering) { return -1; }

			// === Shader === //
			virtual bool CompileVertexShader(Resource::VertexShader* vertex) { return false; }
			virtual bool CompileFragmentShader(Resource::FragmentShader* fragment) { return false; }
			virtual bool LinkShaders(Resource::Shader* shader) { return false; }

			virtual void UseShader(Resource::Shader* shader) {}

			virtual int GetShaderLocation(uint32_t id, const std::string& locationName) { return -1; }

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

			virtual void CreateAndFillVertexBuffer(uint32_t& vbo, const std::vector<Vec3f>& positions, const std::vector<Vec2f>& textureUVs, const std::vector<Vec3f>& normals) {}
			virtual void CreateVertexBuffer(uint32_t& vbo, const void* data, size_t dataSize) {}
			virtual void BindVertexBuffer(uint32_t vbo) {}
			virtual void UnbindVertexBuffer() {}

			virtual void CreateIndexBuffer(uint32_t& ebo, const void* data, size_t dataSize) {}
			virtual void BindIndexBuffer(uint32_t ebo) {}
			virtual void VertexAttribPointer(uint32_t index, int size, int stride, const void* pointer) {}

			virtual void DrawElement(size_t count) {}
			virtual void DrawArrays(size_t start, size_t count) {}

			// Debug
			virtual void DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color = Vec4f(1), float lineWidth = 5.f) {}
		protected:
			bool p_initalized = false;
		private:
			static std::unique_ptr<Renderer> m_instance;
		};

		class OpenGLRenderer : public Renderer
		{
		public:
			OpenGLRenderer();
			OpenGLRenderer& operator=(const OpenGLRenderer& other) = default;
			OpenGLRenderer(const OpenGLRenderer&) = default;
			OpenGLRenderer(OpenGLRenderer&&) noexcept = default;
			virtual ~OpenGLRenderer();

			void Initalize() override;
			void EnableDebugOutput() override;

			void Viewport(const Vec2i& pos, const Vec2i& size) override;
			void ClearColorAndBuffer(const Vec4f& color) override;

			// === Texture === //
			void CreateTexture(Resource::Texture* texture) override;
			void DestroyTexture(Resource::Texture* texture) override;

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

			// Debug
			void DrawLine(Vec3f pos1, Vec3f pos2, Vec4f color = Vec4f(1), float lineWidth = 5.f) override;
		private:

		};
	}
}
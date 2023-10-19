#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"

#include <unordered_map>
#include <deque>


namespace GALAXY {
	namespace Wrapper { class Renderer; class OpenGLRenderer; }
	namespace Resource {
		class VertexShader;
		class GeometryShader;
		class FragmentShader;
		class Shader : public IResource
		{
		public:
			Shader(const std::filesystem::path& fullPath) : IResource(fullPath) {}

			virtual void Load() override;
			void Send() override;

			void SetVertex(Shared<VertexShader> vertexShader, Weak<Shader> weak_this);
			void SetFragment(Shared<FragmentShader> fragmentShader, Weak<Shader> weak_this);
			void SetGeometry(Shared<GeometryShader> geometryShader, Weak<Shader> weak_this);

			void Use();
			int GetLocation(const std::string& locationName);

			void SendInt(const std::string& locationName, int value);
			void SendFloat(const std::string& locationName, float value);
			void SendDouble(const std::string& locationName, double value);
			void SendVec2f(const std::string& locationName, const Vec2f& value);
			void SendVec3f(const std::string& locationName, const Vec3f& value);
			void SendVec4f(const std::string& locationName, const Vec4f& value);
			void SendVec2i(const std::string& locationName, const Vec2i& value);
			void SendVec3i(const std::string& locationName, const Vec3i& value);
			void SendVec4i(const std::string& locationName, const Vec4i& value);
			void SendMat4(const std::string& locationName, const Mat4& value);

			// Get the enum with the class
			static ResourceType GetResourceType() { return ResourceType::Shader; }

			static Weak<Shader> Create(const std::filesystem::path& vertPath, const std::filesystem::path& fragPath);

			Weak<Shader> GetPickingVariant() const { return m_pickingVariant; }
		protected:
			std::unordered_map<std::string, int> p_locations = {};

			Wrapper::Renderer* p_renderer = nullptr;

			uint32_t p_id = -1;

			std::tuple<std::weak_ptr<VertexShader>, std::weak_ptr<GeometryShader>, std::weak_ptr<FragmentShader>> p_subShaders = {};
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			Weak<Shader> m_pickingVariant;
		};

		class BaseShader : public IResource
		{
		public:
			BaseShader(const std::filesystem::path& fullPath) : IResource(fullPath) {}

			virtual void Load() override;
			void AddShader(std::weak_ptr<Shader> shader);
		protected:
			std::vector<std::weak_ptr<Shader>> p_shader = {};
			std::string p_content = "";
			uint32_t m_id = -1;
		private:
		};

		class VertexShader : public BaseShader
		{
		public:
			VertexShader(const std::filesystem::path& fullPath) : BaseShader(fullPath) {}

			void Send() override;

			// Get the enum with the class
			static ResourceType GetResourceType() { return ResourceType::VertexShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};

		class GeometryShader : public BaseShader
		{
		public:
			GeometryShader(const std::filesystem::path& fullPath) : BaseShader(fullPath) {}

			// Get the enum with the class
			static ResourceType GetResourceType() { return ResourceType::GeometryShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};

		class FragmentShader : public BaseShader
		{
		public:
			FragmentShader(const std::filesystem::path& fullPath) : BaseShader(fullPath) {}

			void Send() override;

			// Get the enum with the class
			static ResourceType GetResourceType() { return ResourceType::FragmentShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};
	}
}

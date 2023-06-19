#pragma once
#include "GalaxyAPI.h"
#include "IResource.h"


namespace GALAXY {
	namespace Wrapper { class Renderer; class OpenGLRenderer; }
	namespace Resource {
		class VertexShader;
		class GeometryShader;
		class FragmentShader;
		class Shader : public IResource
		{
		public:
			Shader(const std::string& fullPath) : IResource(fullPath) {}

			void Load() override;
			void Send() override;

			void Use();
			int GetLocation(const std::string& locationName);

			// Get the enum with the class
			ResourceType GetResourceType() const { return ResourceType::Shader; }
		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;

			std::tuple<std::weak_ptr<VertexShader>, std::weak_ptr<GeometryShader>, std::weak_ptr<FragmentShader>> m_subShaders = {};
			std::unordered_map<std::string, int> m_locations;
			uint32_t m_id = -1;
		};

		class BaseShader : public IResource
		{
		public:
			BaseShader(const std::string& fullPath) : IResource(fullPath) {}

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
			VertexShader(const std::string& fullPath) : BaseShader(fullPath) {}

			void Send() override;

			// Get the enum with the class
			ResourceType GetResourceType() const { return ResourceType::VertexShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};

		class GeometryShader : public BaseShader
		{
		public:
			GeometryShader(const std::string& fullPath) : BaseShader(fullPath) {}

			// Get the enum with the class
			ResourceType GetResourceType() const { return ResourceType::GeometryShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};

		class FragmentShader : public BaseShader
		{
		public:
			FragmentShader(const std::string& fullPath) : BaseShader(fullPath) {}

			void Send() override;

			// Get the enum with the class
			ResourceType GetResourceType() const { return ResourceType::FragmentShader; }
		protected:

		private:
			friend Wrapper::Renderer;
			friend Wrapper::OpenGLRenderer;
		};
	}
}

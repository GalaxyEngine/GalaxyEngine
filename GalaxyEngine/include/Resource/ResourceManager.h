#pragma once
#include "GalaxyAPI.h"
#include "Resource/IResource.h"
#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Core/ThreadManager.h"

#include <unordered_map>
#include <filesystem>


namespace GALAXY {
	namespace Core
	{
		class Application;
	}
	namespace Resource {
		class ResourceManager
		{
		public:
			~ResourceManager();

			static Resource::ResourceManager* GetInstance();

			static void Release();

			// Add or Replace the Resource to the resource Manager
			inline void AddResource(IResource* resource);

			inline void AddResource(const std::shared_ptr<IResource>& resource);

			inline bool Contains(const std::filesystem::path& fullPath);

			// Remove the Resource to the resource Manager
			inline void RemoveResource(IResource* resource);

			inline void RemoveResource(const std::shared_ptr<IResource>& resource);

			// Get and load the resources if not loaded yet, 
			// import the resource if not inside the resource Manager
			// TODO: this with Engine Resources
			template <typename T>
			inline std::weak_ptr<T> GetOrLoad(const std::filesystem::path& fullPath);

			// Get The Resource, return null if the type is wrong
			template <typename T>
			[[nodiscard]] inline std::weak_ptr<T> GetResource(const std::filesystem::path& fullPath);

			template <typename T>
			[[nodiscard]] inline std::vector<std::weak_ptr<T>> GetAllResources();

			std::weak_ptr<Resource::Shader> GetDefaultShader();
			std::weak_ptr<Resource::Shader> GetUnlitShader();

			void ImportAllFilesInFolder(const std::filesystem::path& folder);
			void ImportResource(const std::filesystem::path& resourcePath);

			void ProcessDataFile(const std::filesystem::path& dataPath);

			template <typename T>
			[[nodiscard]] inline std::weak_ptr<T> ResourcePopup(const char* popupName);

			std::filesystem::path GetAssetPath() const { return m_assetPath; }
			std::filesystem::path GetProjectPath() const { return m_projectPath; }
		private:
			friend Core::Application;
			static std::unique_ptr<Resource::ResourceManager> m_instance;

			std::unordered_map<std::filesystem::path, std::shared_ptr<IResource>> m_resources;

			std::filesystem::path m_assetPath;
			std::filesystem::path m_projectPath;

		};
	}
}
#include "Resource/ResourceManager.inl" 

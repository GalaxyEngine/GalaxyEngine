#pragma once
#include "GalaxyAPI.h"
#include "Resource/IResource.h"
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

			inline void AddResource(const std::shared_ptr<IResource>& resource);

			template<typename T>
			inline Weak<T> AddResource(const std::filesystem::path& relativePath);

			inline bool Contains(const std::filesystem::path& fullPath);

			// Remove the Resource to the resource Manager
			inline void RemoveResource(IResource* resource);

			inline void RemoveResource(const std::shared_ptr<IResource>& resource);

			inline void RemoveResource(const std::filesystem::path& relativePath);

			// Get and load the resources if not loaded yet, 
			// import the resource if not inside the resource Manager
			template <typename T>
			static inline Weak<T> GetOrLoad(const std::filesystem::path& fullPath);

			template <typename T>
			static inline Weak<T> ReloadResource(const std::filesystem::path& fullPath);

			// Get The Resource, return null if the type is wrong
			template <typename T>
			[[nodiscard]] inline Weak<T> GetResource(const std::filesystem::path& fullPath);

			template <typename T>
			[[nodiscard]] inline std::vector<Weak<T>> GetAllResources();

			inline Weak<class Material> GetDefaultMaterial();
			inline Weak<Shader> GetDefaultShader();
			inline Weak<Shader> GetUnlitShader();

			void ImportAllFilesInFolder(const std::filesystem::path& folder);
			void ImportResource(const std::filesystem::path& resourcePath);

			void ProcessDataFile(const std::filesystem::path& dataPath);

			template <typename T>
			[[nodiscard]] inline Weak<T> ResourcePopup(const char* popupName, const std::vector<Resource::ResourceType>& typeFilter = {});

			std::filesystem::path GetAssetPath() const { return m_assetPath; }
			std::filesystem::path GetProjectPath() const { return m_projectPath; }
		private:
			friend Core::Application;
			static std::unique_ptr<Resource::ResourceManager> m_instance;

			std::unordered_map<std::filesystem::path, std::shared_ptr<IResource>> m_resources;

			Weak<class Material> m_defaultMaterial;
			Weak<class Shader> m_defaultShader;

			std::filesystem::path m_assetPath;
			std::filesystem::path m_projectPath;
			std::string m_projectName;

		};
	}
}
#include "Resource/ResourceManager.inl" 

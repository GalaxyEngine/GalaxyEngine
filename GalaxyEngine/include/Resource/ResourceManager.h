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

			static inline Resource::ResourceManager* GetInstance();

			static void Release();

			inline void AddResource(const Shared<IResource>& resource);

			template<typename T>
			inline Weak<T> AddResource(const Path& fullPath);

			inline bool Contains(const Path& fullPath) const;

			// Remove the Resource to the resource Manager
			inline void RemoveResource(IResource* resource);

			inline void RemoveResource(const Shared<IResource>& resource);

			inline void RemoveResource(const Path& relativePath);

			// Get and load the resources if not loaded yet, 
			// import the resource if not inside the resource Manager
			template <typename T>
			static inline Weak<T> GetOrLoad(const Path& fullPath);

			template <typename T>
			static inline Shared<T> TemporaryLoad(const Path& fullPath);

			template <typename T>
			static inline Weak<T> ReloadResource(const Path& fullPath);

			// Get The Resource, return null if the type is wrong
			template <typename T>
			[[nodiscard]] inline Weak<T> GetResource(const Path& fullPath);

			// Get The Resource, return null if the type is wrong
			template <typename T>
			[[nodiscard]] inline Shared<T> GetTemporaryResource(const Path& fullPath);

			template <typename T>
			[[nodiscard]] inline std::vector<Weak<T>> GetAllResources();

			inline Weak<class Material> GetDefaultMaterial();
			inline Weak<Shader> GetDefaultShader();
			inline Weak<Shader> GetUnlitShader();
			inline Weak<Shader> GetLitShader();

			void ImportAllFilesInFolder(const Path& folder);
			void ImportResource(const Path& resourcePath);
			static bool CheckForDataFile(const Path& resourcePath);

			void ProcessDataFile(const Path& dataPath);

			template <typename T>
			[[nodiscard]] inline Weak<T> ResourcePopup(const char* popupName, const std::vector<Resource::ResourceType>& typeFilter = {});

			Path GetAssetPath() const { return m_assetPath; }
			Path GetProjectPath() const { return m_projectPath; }
		private:
			friend Core::Application;
			static Unique<Resource::ResourceManager> m_instance;

			UMap<Path, Shared<IResource>> m_resources;
			UMap<Path, Weak<IResource>> m_temporaryResources;

			Weak<class Material> m_defaultMaterial;
			Weak<class Shader> m_unlitShader;
			Weak<class Shader> m_litShader;
			Weak<class Shader> m_defaultShader;

			Path m_assetPath;
			Path m_projectPath;
			String m_projectName;
		};

	}
}
#include "Resource/ResourceManager.inl" 

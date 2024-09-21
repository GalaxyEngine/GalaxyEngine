#pragma once
#include "GalaxyAPI.h"
#include <string>
#include <filesystem>
#include <atomic>
#include <memory>
#include "Core/UUID.h"

#include "Utils/FileInfo.h"

namespace CppSer {
	class Serializer;
	class Parser;
}

namespace GALAXY::Resource {
	enum class ResourceType
	{
		None = 0,
		Texture,
		Shader,
		PostProcessShader,
		VertexShader,
		GeometryShader,
		FragmentShader,
		Model,
		Mesh,
		Material,
		Materials,
		Data,
		Script,
		Scene,
		Sound,
		Prefab,
		Cubemap
	};

	inline const char* SerializeResourceTypeValue(ResourceType type)
	{
		switch (type)
		{
		case Resource::ResourceType::None:
			return "None";
		case Resource::ResourceType::Texture:
			return "Texture";
		case Resource::ResourceType::Shader:
			return "Shader";
		case Resource::ResourceType::PostProcessShader:
			return "PostProcessShader";
		case Resource::ResourceType::VertexShader:
			return "VertexShader";
		case Resource::ResourceType::GeometryShader:
			return "GeometryShader";
		case Resource::ResourceType::FragmentShader:
			return "FragmentShader";
		case Resource::ResourceType::Model:
			return "Model";
		case Resource::ResourceType::Mesh:
			return "Mesh";
		case Resource::ResourceType::Material:
			return "Material";
		case Resource::ResourceType::Materials:
			return "Materials";
		case Resource::ResourceType::Data:
			return "Data";
		case Resource::ResourceType::Script:
			return "Script";
		case Resource::ResourceType::Scene:
			return "Scene";
		case Resource::ResourceType::Sound:
			return "Sound";
		case Resource::ResourceType::Prefab:
			return "Prefab";
		case Resource::ResourceType::Cubemap:
			return "Cubemap";
		default:
			return "Unknown";
		}
	}

	enum class ResourceStatus : int
	{
		None = 0,
		DisplayOnInspector = 1,
		CreateDataFile = 2,
	};

	inline constexpr ResourceStatus
		operator|(const ResourceStatus a, const ResourceStatus b) {
		return static_cast<ResourceStatus>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline constexpr ResourceStatus
		operator&(const ResourceStatus a, const ResourceStatus b) {
		return static_cast<ResourceStatus>(static_cast<int>(a) & static_cast<int>(b));
	}

	inline constexpr ResourceStatus
		operator~(const ResourceStatus a) {
		return static_cast<ResourceStatus>(~static_cast<int>(a));
	}

	class GALAXY_API IResource : public std::enable_shared_from_this<IResource>
	{
	public:
		explicit IResource(const Path& fullPath);
		IResource& operator=(const IResource& other);
		IResource(const IResource&) = default;
		IResource(IResource&&) noexcept = default;
		virtual ~IResource();

		virtual void Load() {}
		virtual void Send() {}
		virtual void ShowInInspector() {}
		// Called when the resource is removed from the resourceManager
		virtual void Unload() {}
		//Called when the resource is added to the resourceManager
		virtual void OnAdd() {}

		virtual const char* GetResourceName() const = 0;
		virtual Path GetThumbnailPath() const {return "";}
		static ResourceType GetResourceType() { return ResourceType::None; }

		inline bool ShouldBeLoaded() const { return p_shouldBeLoaded.load(); }
		inline bool IsLoaded() const { return p_loaded.load(); }
		inline bool HasBeenSent() const { return p_hasBeenSent.load(); }

		void SendRequest() const;

		void CreateDataFile() const;

		void ParseDataFile();

		inline void SetDisplayOnInspector(const bool val)
		{
			p_status = val ? (p_status | ResourceStatus::DisplayOnInspector) : (p_status & ~ResourceStatus::DisplayOnInspector);
		}
		inline void SetCreateDataFile(const bool val)
		{
			p_status = val ? (p_status | ResourceStatus::CreateDataFile) : (p_status & ~ResourceStatus::CreateDataFile);
		}

		//Editor Only!
		void SetUUID(const Core::UUID& uuid);
		// ! this will only rename the internal resource full path, relative path and name, not the file on disk
		void Rename(const Path& newFullPath);

		inline std::string GetName() const { return p_fileInfo.GetFileName(); }
		inline Utils::FileInfo& GetFileInfo() { return p_fileInfo; }
		inline Utils::FileInfo GetFileInfo() const { return p_fileInfo; }
		inline Core::UUID GetUUID() const { return p_uuid; }
		Path GetDataFilePath() const;

		static void SerializeResource(CppSer::Serializer& serializer, const char* name, Weak<IResource> resource);

		void StartLoading() const;
		void FinishLoading() const;
	protected:
		virtual void Serialize(CppSer::Serializer& serializer) const;
		virtual void Deserialize(CppSer::Parser& parser);

	private:
		bool ShouldDisplayOnInspector() const { return (p_status & ResourceStatus::DisplayOnInspector) != ResourceStatus::None; }
		bool ShouldCreateDataFile() const { return (p_status & ResourceStatus::CreateDataFile) != ResourceStatus::None; }
	protected:
		friend class ResourceManager;

		Utils::FileInfo p_fileInfo;

		Core::UUID p_uuid;

		ResourceStatus p_status = ResourceStatus::DisplayOnInspector | ResourceStatus::CreateDataFile;

		std::atomic_bool p_shouldBeLoaded = false;
		std::atomic_bool p_loaded = false;
		std::atomic_bool p_hasBeenSent = false;
	};

}
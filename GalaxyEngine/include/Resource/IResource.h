#pragma once
#include "GalaxyAPI.h"
#include <string>
#include <filesystem>
#include <atomic>
#include <memory>
#include <vector>

#include "Utils/FileInfo.h"

namespace GALAXY::Resource {
	enum class ResourceType
	{
		None,
		Texture,
		Shader,
		VertexShader,
		GeometryShader,
		FragmentShader,
		Model,
		Mesh,
		Material,
		Data,
		Script,
	};

	class IResource
	{
	public:
		IResource(const std::filesystem::path& fullPath);
		IResource& operator=(const IResource& other) = default;
		IResource(const IResource&) = default;
		IResource(IResource&&) noexcept = default;
		virtual ~IResource() {}

		virtual void Load() {}
		virtual void Send() {}
		virtual void ShowInInspector() {}

		bool ShouldBeLoaded() const { return p_shouldBeLoaded.load(); }
		bool IsLoaded() const { return p_loaded.load(); }
		bool HasBeenSent() const { return p_hasBeenSent.load(); }

		void SendRequest();

		std::string GetName() { return p_fileInfo.GetFileName().string(); }

		Utils::FileInfo& GetFileInfo() { return p_fileInfo; }
	protected:
		friend class ResourceManager;

		Utils::FileInfo p_fileInfo;

		std::atomic_bool p_shouldBeLoaded = false;
		std::atomic_bool p_loaded = false;
		std::atomic_bool p_hasBeenSent = false;

	};
}

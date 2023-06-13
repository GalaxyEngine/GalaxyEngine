#pragma once
#include "GalaxyAPI.h"
#include <string>
#include <atomic>
#include <memory>
#include <vector>
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
	};

	class IResource
	{
	public:
		IResource(const std::string& fullPath);
		IResource& operator=(const IResource& other) = default;
		IResource(const IResource&) = default;
		IResource(IResource&&) noexcept = default;
		virtual ~IResource() {}

		static ResourceType GetTypeFromExtension(const std::string_view& ext);
		static std::string ExtractNameFromPath(std::string path, bool extension = true);
		static std::string ExtractRelativePathFromPath(const std::string& path);
		static std::string ExtractExtensionFromPath(const std::string& path);

		virtual void Load() {}
		virtual void Send() {}

		bool ShouldBeLoaded() const { return p_shouldBeLoaded.load(); }
		bool IsLoaded() const { return p_loaded.load(); }
		bool HasBeenSent() const { return p_hasBeenSent.load(); }

		void SendRequest();
	protected:
		friend class ResourceManager;

		ResourceType p_type;
		std::string p_fullPath;
		std::string p_relativepath;
		std::string p_name;

		std::atomic_bool p_shouldBeLoaded = false;
		std::atomic_bool p_loaded = false;
		std::atomic_bool p_hasBeenSent = false;

	};
}

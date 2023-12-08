#pragma once
#include "GalaxyAPI.h"
#include <string>
#include <filesystem>
#include <atomic>
#include <memory>
#include <vector>

#include "Utils/FileInfo.h"
#include "Utils/Parser.h"

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
	};

	enum class ParseResult
	{
		Sucess,
		Failed,
		NeedToLoad
	};

	class IResource
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
		virtual void Unload() {}

		inline bool ShouldBeLoaded() const { return p_shouldBeLoaded.load(); }
		inline bool IsLoaded() const { return p_loaded.load(); }
		inline bool HasBeenSent() const { return p_hasBeenSent.load(); }

		void SendRequest() const;

		void CreateDataFile();
		virtual void Serialize(Utils::Serializer& serializer);

		ParseResult ParseDataFile();

		void ShouldBeDisplayOnInspector(const bool val) { p_displayOnInspector = val; }

		inline std::string GetName() const { return p_fileInfo.GetFileName(); }
		inline Utils::FileInfo& GetFileInfo() { return p_fileInfo; }
	protected:

	virtual ParseResult Deserialize(Utils::Parser& parser);
	protected:
		friend class ResourceManager;

		Utils::FileInfo p_fileInfo;

		std::atomic_bool p_shouldBeLoaded = false;
		std::atomic_bool p_loaded = false;
		std::atomic_bool p_hasBeenSent = false;

		bool p_displayOnInspector = true;

	};
}

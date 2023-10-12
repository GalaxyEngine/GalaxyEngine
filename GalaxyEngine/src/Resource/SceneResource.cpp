#include "pch.h"

#include "Resource/SceneResource.h"
#include "Resource/ResourceManager.h"

#include "Utils/Parser.h"

#include "Core/GameObject.h"
#include "Core/SceneHolder.h"
#include "Core/Scene.h"

namespace GALAXY 
{
	void Resource::SceneResource::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;

		m_root = std::make_shared<Core::GameObject>();
		Utils::Parser parser(GetFileInfo().GetFullPath());
		m_root->Deserialize(parser);

		p_loaded = true;
	}

	void Resource::SceneResource::Save()
	{
		Utils::Serializer serializer(GetFileInfo().GetFullPath());

		if (!m_root)
			return;
		m_root->Serialize(serializer);
		serializer.CloseFile();
	}

	Weak<Resource::SceneResource> Resource::SceneResource::Create(const std::filesystem::path& path)
	{
		SceneResource scene(path);
		scene.Save();
		return Resource::ResourceManager::GetInstance()->GetOrLoad<SceneResource>(path);
	}

	void Resource::SceneResource::Reload()
	{
		m_root.reset();
		m_root = std::make_shared<Core::GameObject>();
		Utils::Parser parser(GetFileInfo().GetFullPath());
		m_root->Deserialize(parser);
	}

}

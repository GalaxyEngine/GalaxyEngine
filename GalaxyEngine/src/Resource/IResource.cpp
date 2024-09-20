#include "pch.h"

#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"

#include "Core/Application.h"
#include "Editor/UI/EditorUIManager.h"

#include "Utils/OS.h"

namespace GALAXY {
	Resource::IResource::IResource(const Path& fullPath) : p_fileInfo(fullPath)
	{
		ASSERT(!fullPath.empty());
		p_shouldBeLoaded = false;
		p_loaded = false;
		p_hasBeenSent = false;
	}

	Resource::IResource& Resource::IResource::operator=(const Resource::IResource& other)
	{
		p_shouldBeLoaded.store(false);
		p_loaded.store(false);
		p_hasBeenSent.store(false);
		return *this;
	}

	Resource::IResource::~IResource()
	{
		//PrintWarning("Expired %s", p_fileInfo.GetFullPath().string().c_str());
	}

	void Resource::IResource::CreateDataFile() const
	{
		if (!ShouldCreateDataFile())
			return;

		auto dataPath = GetDataFilePath();
		{
			CppSer::Serializer serializer(dataPath);

			serializer << CppSer::Pair::BeginMap << "Data";
			Serialize(serializer);
			serializer << CppSer::Pair::EndMap << "Data";
		}

		Utils::OS::ShowFile(dataPath, false);
	}

	void Resource::IResource::SetUUID(const Core::UUID& uuid)
	{
		p_uuid = uuid;
		CreateDataFile();
	}

	void Resource::IResource::SerializeResource(CppSer::Serializer& serializer, const char* name, Weak<IResource> resource)
	{
		if (resource.lock())
			serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << resource.lock()->GetUUID();
		else
			serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << UUID_NULL;
	}

	void Resource::IResource::StartLoading() const
	{
#if WITH_EDITOR
		Core::ThreadManager::Lock();
		ASSERT(p_shouldBeLoaded);
		auto editorUiManager = Editor::UI::EditorUIManager::GetInstance();
		ASSERT(editorUiManager);
		editorUiManager->AddResourceLoading(this->p_uuid);
		Core::ThreadManager::Unlock();
#endif
	}

	void Resource::IResource::FinishLoading() const
	{
#if WITH_EDITOR
		Core::ThreadManager::Lock();
		ASSERT(p_loaded);
		Editor::UI::EditorUIManager::GetInstance()->RemoveResourceLoading(this->p_uuid);
		Core::ThreadManager::Unlock();
#endif
	}

	void Resource::IResource::Serialize(CppSer::Serializer& serializer) const
	{
		serializer << CppSer::Pair::Key << "UUID" << CppSer::Pair::Value << p_uuid;
	}

	void Resource::IResource::ParseDataFile()
	{
		const auto dataPath = Path(p_fileInfo.GetFullPath().generic_string() + ".gdata");
		CppSer::Parser parser(dataPath);

		if (!parser.IsFileOpen())
		{
			//PrintWarning("Failed to open %s", dataPath.generic_string().c_str());
			return;
		}

		return Deserialize(parser);
	}

	void Resource::IResource::Deserialize(CppSer::Parser& parser)
	{
		p_uuid = parser["UUID"].As<uint64_t>();
	}

	void Resource::IResource::SendRequest() const
	{
		Core::ThreadManager::ForceLock();
		Core::Application::GetInstance().AddResourceToSend(p_fileInfo.GetFullPath());
		Core::ThreadManager::Unlock();
	}

	void Resource::IResource::Rename(const Path& newFullPath)
	{
		Resource::ResourceManager::RenameSingle(p_fileInfo.GetFullPath(), newFullPath);
	}

	Path Resource::IResource::GetDataFilePath() const
	{
		return Path(p_fileInfo.GetFullPath().string() + ".gdata");
	}

}
#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Core/Application.h"

namespace GALAXY {
	Resource::IResource::IResource(const Path& fullPath)
	{
		ASSERT(!fullPath.empty());
		p_shouldBeLoaded = false;
		p_loaded = false;
		p_hasBeenSent = false;
		p_fileInfo = Utils::FileInfo(fullPath);
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

		auto dataPath = Path(p_fileInfo.GetFullPath().generic_string() + ".gdata");
		CppSer::Serializer serializer(dataPath);

		serializer << CppSer::Pair::BeginMap << "Data";
		Serialize(serializer);
		serializer << CppSer::Pair::EndMap << "Data";
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
			serializer << CppSer::Pair::Key << name << CppSer::Pair::Value << INDEX_NONE;
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
		Core::ThreadManager::Lock();
		Core::Application::GetInstance().AddResourceToSend(p_fileInfo.GetFullPath());
		Core::ThreadManager::Unlock();
	}


}
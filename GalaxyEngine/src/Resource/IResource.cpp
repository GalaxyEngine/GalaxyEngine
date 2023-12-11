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
		if (!GetCreateDataFile())
			return;

		auto dataPath = Path(p_fileInfo.GetFullPath().generic_string() + ".gdata");
		Serializer serializer(dataPath);

		serializer << Pair::BEGIN_MAP << "Data";
		Serialize(serializer);
		serializer << Pair::END_MAP << "Data";
	}

	void Resource::IResource::SetUUID(const Core::UUID& uuid)
	{
		p_uuid = uuid;
		CreateDataFile();
	}

	void Resource::IResource::SerializeResource(Utils::Serializer& serializer, const char* name, Weak<IResource> resource)
	{
		if (resource.lock())
			serializer << Pair::KEY << name << Pair::VALUE << resource.lock()->GetUUID();
		else
			serializer << Pair::KEY << name << Pair::VALUE << INDEX_NONE;
	}

	void Resource::IResource::Serialize(Utils::Serializer& serializer) const
	{
		serializer << Pair::KEY << "UUID" << Pair::VALUE << p_uuid;
	}

	void Resource::IResource::ParseDataFile()
	{
		const auto dataPath = Path(p_fileInfo.GetFullPath().generic_string() + ".gdata");
		Parser parser(dataPath);

		if (!parser.IsFileOpen())
		{
			//PrintWarning("Failed to open %s", dataPath.generic_string().c_str());
			return;
		}

		return Deserialize(parser);
	}

	void Resource::IResource::Deserialize(Utils::Parser& parser)
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
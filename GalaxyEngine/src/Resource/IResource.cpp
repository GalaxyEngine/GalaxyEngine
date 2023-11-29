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
		PrintWarning("Expired %s", p_fileInfo.GetFullPath().string().c_str());
	}

	void Resource::IResource::SendRequest()
	{
		Core::ThreadManager::Lock();
		Core::Application::GetInstance().AddResourceToSend(p_fileInfo.GetFullPath());
		Core::ThreadManager::Unlock();
	}
}
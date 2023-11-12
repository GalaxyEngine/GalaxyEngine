#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Core/Application.h"

namespace GALAXY {
	Resource::IResource::IResource(const Path& fullPath)
	{
		ASSERT(!fullPath.empty());
		p_fileInfo = Utils::FileInfo(fullPath);
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
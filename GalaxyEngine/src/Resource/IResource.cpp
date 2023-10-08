#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Core/Application.h"


Resource::IResource::IResource(const std::filesystem::path& fullPath)
{
	ASSERT(!fullPath.empty());
	p_fileInfo = Utils::FileInfo(fullPath);
}

Resource::IResource::~IResource()
{
	PrintWarning("Expired %s", p_fileInfo.GetFullPath().string().c_str());
}

void GALAXY::Resource::IResource::SendRequest()
{
	Core::ThreadManager::GetInstance()->Lock();
	Core::Application::GetInstance().AddResourceToSend(p_fileInfo.GetFullPath());
	Core::ThreadManager::GetInstance()->Unlock();
}

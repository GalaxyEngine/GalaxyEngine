#include "pch.h"
#include "Resource/IResource.h"
#include "Resource/ResourceManager.h"
#include "Core/Application.h"


Resource::IResource::IResource(const std::filesystem::path& fullPath)
{
	p_fileInfo = Utils::FileInfo(fullPath);
}

void GALAXY::Resource::IResource::SendRequest()
{
	Core::ThreadManager::GetInstance()->Lock();
	Core::Application::GetInstance().AddResourceToSend(p_fileInfo.GetFullPath());
	Core::ThreadManager::GetInstance()->Unlock();
}

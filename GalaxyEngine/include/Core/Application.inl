#pragma once
#include "Core/Application.h"
namespace GALAXY 
{
	inline void Core::Application::AddResourceToSend(const std::filesystem::path& fullPath)
	{
		if (!std::count(m_resourceToSend.begin(), m_resourceToSend.end(), fullPath))
			m_resourceToSend.push_back(fullPath);
	}
}

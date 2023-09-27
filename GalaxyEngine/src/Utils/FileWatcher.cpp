#include "pch.h"
#include "Utils/FileWatcher.h"

#include "Core/ThreadManager.h"
namespace GALAXY 
{

	void Utils::FileWatcher::StartWatching()
	{
		m_shouldStop = false;
		if (m_multithread)
			Core::ThreadManager::GetInstance()->AddTask(&FileWatcher::Update, this);
	}

}

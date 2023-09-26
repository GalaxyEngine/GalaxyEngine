#include "pch.h"
#include "Utils/FileWatcher.h"

#include "Core/ThreadManager.h"
namespace GALAXY 
{

	void Utils::FileWatcher::StartWatching()
	{
		Core::ThreadManager::GetInstance()->AddTask(&FileWatcher::Update, this);
	}

}

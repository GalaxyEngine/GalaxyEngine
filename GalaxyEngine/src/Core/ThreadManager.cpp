#include "pch.h"
#include "Core/ThreadManager.h"

std::unique_ptr<Core::ThreadManager> Core::ThreadManager::m_instance;

Core::ThreadManager::~ThreadManager()
{
}

void Core::ThreadManager::Initialize()
{
	m_threadList.resize(std::thread::hardware_concurrency());
	for (uint32_t i = 0; i < m_threadList.size(); i++) {
		m_threadList.at(i) = std::thread(&ThreadManager::ThreadLoop, this);
	}
}

void Core::ThreadManager::ThreadLoop()
{
	while (!m_terminate)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!m_tasks.empty())
		{
			Lock();
			if (!m_tasks.empty()) {
				auto task = m_tasks.front();
				m_tasks.pop();
				Unlock();
				if (task != nullptr)
					task();
				break;
			}
			Unlock();
		}
	}
	if (m_terminate)
	{
		return;
	}
	ThreadLoop();
}

void Core::ThreadManager::Lock()
{
	while (!m_instance->m_mutex.try_lock()) {}
}

void Core::ThreadManager::Unlock()
{
#pragma warning(push)
#pragma warning(disable: 26110)
	m_instance->m_mutex.unlock();
#pragma warning(pop)  
}

Core::ThreadManager* Core::ThreadManager::GetInstance()
{
	if (m_instance == nullptr) {
		m_instance = std::make_unique<ThreadManager>();
	}
	return m_instance.get();
}

void Core::ThreadManager::Destroy()
{
	Terminate();
	// clear threadList
	while (m_threadList.size() > 0)
	{
		for (size_t i = 0; i < m_threadList.size(); i++) {
			if (m_threadList[i].joinable())
			{
				m_threadList[i].join();
				m_threadList.erase(m_threadList.begin() + i--);
			}
		}
	}
}

#pragma once
#include "GalaxyAPI.h"

#include <thread>
#include <queue>
#include <functional>
#include <mutex>

namespace GALAXY::Core {
	class ThreadManager
	{
	public:
		~ThreadManager();

		void Initialize();
		void Destroy();

		void ThreadLoop();

		void Terminate() { m_terminate = true; }

		template <typename F, typename... A> void AddTask(F&& task, A&&... args)
		{
			std::function<void()> task_function = [task = std::forward<F>(task), args = std::make_tuple(std::forward<A>(args)...)]() mutable {
				std::apply(task, args);
			};
			if (task_function) {
				std::lock_guard<std::mutex> lock(m_mutex);
				m_tasks.push(task_function);
			}
		}

		void Lock();
		void Unlock();

		static ThreadManager* GetInstance();

	private:
		static std::unique_ptr<ThreadManager> m_instance;

		std::queue<std::function<void()>> m_tasks = {};
		std::vector<std::thread> m_threadList = {};
		std::mutex m_mutex;
		bool m_terminate = false;

	};
}

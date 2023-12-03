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
		~ThreadManager() = default;

		void Initialize();
		void Destroy();

		void ThreadLoop();

		inline void Terminate() { m_terminate = true; }

		template <typename F, typename... A> inline void AddTask(F&& task, A&&... args)
		{
			std::function<void()> task_function = [task = std::forward<F>(task), args = std::make_tuple(std::forward<A>(args)...)]() mutable {
				std::apply(task, args);
				};
			if (task_function) {
				std::lock_guard lock(m_mutex);
				m_tasks.push(task_function);
			}
		}

		static void Lock();
		static void Unlock();

		static ThreadManager* GetInstance();

	private:
		static std::unique_ptr<ThreadManager> m_instance;

		std::queue<std::function<void()>> m_tasks = {};
		std::vector<std::thread> m_threadList = {};
		std::mutex m_mutex;
		bool m_terminate = false;

	};
}

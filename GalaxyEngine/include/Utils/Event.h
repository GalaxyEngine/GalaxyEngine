#pragma once
#include "GalaxyAPI.h"
#include <functional>
namespace GALAXY 
{
	namespace Utils
	{
		template<typename... Args>
		class Event {
		public:
			Event() = default;
			Event& operator=(const Event& other) = default;
			Event(const Event&) = default;
			Event(Event&&) noexcept = default;
			virtual ~Event() = default; 

			using Callback = std::function<void(Args...)>;

			virtual void Bind(Callback callback)
			{
				m_callbacks.push_back(callback);
			}

			virtual void Invoke(Args... args)
			{
				for (auto& callback : m_callbacks)
				{
					callback(args...);
				}
			}
		private:
			std::vector<Callback> m_callbacks;
		};
	}
}

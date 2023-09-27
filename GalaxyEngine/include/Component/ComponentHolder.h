#pragma once
#include "GalaxyAPI.h"
#include <vector>
#include <memory>

namespace GALAXY::Component
{
	class BaseComponent;
	class ComponentHolder
	{
	public:
		~ComponentHolder() {}

		static void Initialize();

		template<typename T>
		static inline void RegisterComponent();

		template<typename T>
		static inline void RegisterComponent(Shared<T> component);

		template<typename T>
		static inline void UnregisterComponent(Shared<T> component);

		static std::vector<std::shared_ptr<BaseComponent>>& GetList() { return m_componentList; }

	private:
		static std::vector<std::shared_ptr<BaseComponent>> m_componentList;
	};

	template<typename T>
	void Component::ComponentHolder::UnregisterComponent(Shared<T> component)
	{
		if (auto comp = std::find(m_componentList.begin(), m_componentList.end(), component); comp != m_componentList.end())
			m_componentList.erase(comp);
	}

}
#include "Component/ComponentHolder.inl" 

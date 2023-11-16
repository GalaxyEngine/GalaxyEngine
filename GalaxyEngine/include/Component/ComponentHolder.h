#pragma once
#include "GalaxyAPI.h"
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
		static inline void RegisterComponent(T* component);

		template<typename T>
		static inline void UnregisterComponent(T* component);

		static List<Shared<BaseComponent>>& GetList() { return m_componentList; }



	private:
		static List<Shared<BaseComponent>> m_componentList;
	};

}
#include "Component/ComponentHolder.inl" 

#pragma once
#include "GalaxyAPI.h"
#include <unordered_map>
#include "ScriptEngine.h"

namespace GALAXY 
{
	namespace Component { class ScriptComponent; }
	namespace Scripting
	{
		using ScriptConstructor = void* (*)();
		using GetterMethod = void* (*)(void*);
		using SetterMethod = void (*)(void*, void*);

		class ScriptInstance
		{
		public:
			ScriptInstance() {}
			ScriptInstance& operator=(const ScriptInstance& other) = default;
			ScriptInstance(const ScriptInstance&) = default;
			ScriptInstance(ScriptInstance&&) noexcept = default;
			virtual ~ScriptInstance() {}

			std::unordered_map<std::string, Scripting::VariableType> GetAllVariables() const { return m_variables; }
		private:
			friend class ScriptEngine;
			friend Component::ScriptComponent;

			ScriptConstructor m_constructor = nullptr;
			std::unordered_map<std::string, GetterMethod> m_gettersMethods;
			std::unordered_map<std::string, SetterMethod> m_settersMethods;
			std::unordered_map<std::string, VariableType> m_variables;
		}; 
	}
}
#include "Scripting/ScriptInstance.inl" 

#pragma once
#include "GalaxyAPI.h"
#include <unordered_map>
#include "ScriptEngine.h"

namespace GALAXY 
{
	namespace Component { class ScriptComponent; }
	namespace Scripting
	{
		class ScriptInstance
		{
		public:
			ScriptInstance() {}
			ScriptInstance& operator=(const ScriptInstance& other) = default;
			ScriptInstance(const ScriptInstance&) = default;
			ScriptInstance(ScriptInstance&&) noexcept = default;
			virtual ~ScriptInstance() {}

			inline UMap<std::string, VariableData> GetAllVariables() const { return m_variables; }
		private:
			friend class ScriptEngine;
			friend Component::ScriptComponent;

			ScriptConstructor m_constructor = nullptr;
			UMap<std::string, GetterMethod> m_gettersMethods;
			UMap<std::string, SetterMethod> m_settersMethods;
			UMap<std::string, VariableData> m_variables;
		}; 
	}
}
#include "Scripting/ScriptInstance.inl" 

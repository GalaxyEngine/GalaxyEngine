#pragma once
#include "GalaxyAPI.h"
#include <unordered_map>
namespace GALAXY 
{
	namespace Scripting
	{
		using ScriptConstructor = void* (*)();
		using GetterMethod = void* (*)(void*);
		using SetterMethod = void (*)(void*, void*);

		enum class VariableType
		{
			Bool,
			Int,
			Float,
			Double,
		};

		class ScriptInstance
		{
		public:
			ScriptInstance() {}
			ScriptInstance& operator=(const ScriptInstance& other) = default;
			ScriptInstance(const ScriptInstance&) = default;
			ScriptInstance(ScriptInstance&&) noexcept = default;
			virtual ~ScriptInstance() {}

		private:
			ScriptConstructor m_constructor;
			std::unordered_map<std::string, GetterMethod> m_gettersMethods;
			std::unordered_map<std::string, SetterMethod> m_settersMethods;
			std::unordered_map<std::string, VariableType> m_variables;
		};
	}
}
#include "Scripting/ScriptInstance.inl" 

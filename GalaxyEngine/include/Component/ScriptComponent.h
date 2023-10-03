#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

#include <unordered_map>
#include <any>

namespace GALAXY
{
	namespace Scripting { enum class VariableType; }
	namespace Component
	{

		class GALAXY_API ScriptComponent : public IComponent<ScriptComponent>
		{
		public:
			ScriptComponent() {}
			ScriptComponent& operator=(const ScriptComponent& other) = default;
			ScriptComponent(const ScriptComponent&) = default;
			ScriptComponent(ScriptComponent&&) noexcept = default;
			virtual ~ScriptComponent() {}

			virtual const char* GetComponentName() const override
			{
				return "Script Component";
			}

			virtual void ShowInInspector() override;

			template<typename T>
			T* GetVariable(const std::string& variableName)
			{
				return reinterpret_cast<T*>(GetVariableVoid(variableName));
			}

			std::any GetVariable(const std::string& variableName);
			void SetVariable(const std::string& variableName, std::any value);

			template<typename T>
			void SetVariable(const std::string& variableName, T value)
			{
				T* newValue = &value;
				SetVariableVoid(variableName, newValue);
			}

			std::unordered_map<std::string, Scripting::VariableType> GetAllVariables() const;

		private:

			void* GetVariableVoid(const std::string& variableName);

			void SetVariableVoid(const std::string& variableName, void* value);

		};

		class ReloadScript
		{
		public:
			ReloadScript(Shared<ScriptComponent> component) : m_component(component) {}

			void BeforeReloadScript();
			void AfterReloadScript();
			Shared<GALAXY::Component::ScriptComponent> GetComponent() const { return m_component; }
		private:
			Shared<ScriptComponent> m_component;

			std::string m_scriptName;

			std::unordered_map<std::string, std::any> m_tempVariables;
		};
	}
}
#include "Component/ScriptComponent.inl" 

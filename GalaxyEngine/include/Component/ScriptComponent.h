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

			virtual const char* GetComponentName() override;

			virtual std::shared_ptr<BaseComponent> Clone() override;

			void OnCreate() override;

			void OnStart() override;

			void OnUpdate() override;

			void OnDraw() override;

			void OnDestroy() override;

			void ShowInInspector() override;

			void SetGameObject(Weak<Core::GameObject> object) override;

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

			void SetScriptComponent(std::shared_ptr<Component::BaseComponent> val);

			void BeforeReloadScript();
			void AfterReloadScript();
		private:

			void* GetVariableVoid(const std::string& variableName);

			void SetVariableVoid(const std::string& variableName, void* value);

			std::shared_ptr<BaseComponent> m_component;

			std::string m_scriptName;

			std::unordered_map<std::string, std::any> m_tempVariables;

		};
	}
}
#include "Component/ScriptComponent.inl" 

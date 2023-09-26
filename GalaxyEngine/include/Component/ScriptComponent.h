#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"
namespace GALAXY
{
	namespace Component
	{
		class ScriptComponent : public IComponent<ScriptComponent>
		{
		public:
			ScriptComponent() {}
			ScriptComponent& operator=(const ScriptComponent& other) = default;
			ScriptComponent(const ScriptComponent&) = default;
			ScriptComponent(ScriptComponent&&) noexcept = default;
			virtual ~ScriptComponent() {}

			virtual std::string GetComponentName() const override { return "ScriptComponent"; }

			void OnCreate() override;

			void OnStart() override;

			void OnUpdate() override;

			void OnDraw() override;

			void OnDestroy() override;

			void ShowInInspector() override;

			template<typename T>
			T GetVariable(const std::string& variableName)
			{
				return reinterpret_cast<T*>(GetVariableVoid(variableName));
			}

			template<typename T>
			void SetVariable(const std::string& variableName, T value)
			{
				T* newValue = &value;
				SetVariableVoid(variableName, newValue);
			}

		private:
			void* GetVariableVoid(const std::string& variableName);

			void SetVariableVoid(const std::string& variableName, void* value);

			std::shared_ptr<BaseComponent> m_component;

		};
	}
}
#include "Component/ScriptComponent.inl" 

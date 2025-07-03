#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

#include "Core/UUID.h"

#include <unordered_map>
#include <any>
#include <optional>

#include "Scripting/ScriptEngine.h"

namespace GALAXY
{
	namespace Scripting { class ScriptEngine; }
	namespace Core {class GameObject;}
	namespace Component
	{

		class GALAXY_API ScriptComponent : public IComponent<ScriptComponent>
		{
		public:
			ScriptComponent() {}
			ScriptComponent& operator=(const ScriptComponent& other) = default;
			ScriptComponent(const ScriptComponent&) = default;
			ScriptComponent(ScriptComponent&&) noexcept = default;
			~ScriptComponent() override {}

			inline virtual const char* GetComponentName() const override { return "ScriptComponent"; }
			inline virtual std::set<const char*> GetComponentNames() const {
				std::set<const char*> list = BaseComponent::GetComponentNames();	
				list.insert(ScriptComponent::GetComponentName());
				return list;
			}

#ifdef WITH_EDITOR
			EDITOR_ONLY virtual void ShowInInspector() override;

			EDITOR_ONLY virtual uint32_t GetIcon() override;
#endif

			void OnCreate() override;
			void AfterLoad() override;

			template<typename T>
			inline T* GetVariable(const std::string& variableName)
			{
				return Scripting::ScriptEngine::GetInstance()->GetScriptVariable<T>(this, GetComponentName(), variableName);
			}

			template<typename T>
			inline void SetVariable(const std::string& variableName, T value)
			{
				Scripting::ScriptEngine::GetInstance()->SetScriptVariable<T>(this, GetComponentName(), variableName, value);
			}

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;
			
		private:
			void InitializeVariablesInfo();
			void SetupVariables();

		private:
			friend Scripting::ScriptEngine;
			std::unordered_map<std::string, std::shared_ptr<Scripting::VariableInfo>> m_variablesInfo;
			std::unordered_map<std::string, void*> m_variablesPtr;
		};
	}
}
#include "Component/ScriptComponent.inl" 

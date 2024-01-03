#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"
#include "Scripting/ScriptEngine.h"

#include "Core/UUID.h"

#include <unordered_map>
#include <any>
#include <optional>

namespace GALAXY
{
	namespace Scripting {}
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

			inline virtual const char* GetComponentName() const override { return "Script Component"; }

			virtual void ShowInInspector() override;

			template<typename T>
			inline T* GetVariable(const std::string& variableName)
			{
				auto value = static_cast<T*>(GetVariableVoid(variableName));
				return value;
			}

			std::any GetVariable(const std::string& variableName);
			void SetVariable(const std::string& variableName, std::any value);

			template<typename T>
			inline void SetVariable(const std::string& variableName, T value)
			{
				T* newValue = &value;
				SetVariableVoid(variableName, newValue);
			}

			UMap<std::string, Scripting::VariableData> GetAllVariables() const;

			void Serialize(Utils::Serializer& serializer) override;
			void Deserialize(Utils::Parser& parser) override;

			void AfterLoad() override;

		private:
			template<typename T> inline void DisplayAndManageVariable(const std::pair<std::string, Scripting::VariableData>& variable);

			template<typename T> void DisplayVariableT(const std::pair<std::string, Scripting::VariableData>& variable, T* value);

			void DisplayVariableField(const std::pair<std::string, Scripting::VariableData>& variable);

			void* GetVariableVoid(const std::string& variableName);

			void SetVariableVoid(const std::string& variableName, void* value);

			// De-serialization variables
			std::unordered_map<std::string, uint64_t> p_tempGameObjectIDs;
			std::unordered_map<std::string, ComponentID> p_tempComponentIDs;
		};

		struct ComponentInfo
		{
			Core::UUID gameObjectID = -1;
			uint32_t componentID = -1;
			std::string componentName = "";
			std::optional<size_t> indexOnList;
			std::string variableName = "";
		};

		class ReloadScript
		{
		public:
			explicit ReloadScript(const Shared<ScriptComponent>& component) : m_component(component) {}

			void BeforeReloadScript();
			void AfterReloadScript();
			void SyncComponentsValues() const;
			Shared<ScriptComponent> GetComponent() const { return m_component; }

			std::string GetScriptName() const { return m_scriptName; }
		private:
			static std::any ConvertComponentToInfo(const std::any& value);

			static std::any ConvertGameObjectToID(const std::any& value);

			static bool ConvertInfoToComponent(std::any& value);

			static std::any ConvertIDToGameObject(const std::any& value);
		private:
			Shared<ScriptComponent> m_component;

			std::string m_scriptName;

			UMap<std::string, ComponentInfo> m_missingComponentRefs;

			UMap<std::string, std::pair<std::any, Scripting::VariableData>> m_tempVariables;
		};
	}
}
#include "Component/ScriptComponent.inl" 

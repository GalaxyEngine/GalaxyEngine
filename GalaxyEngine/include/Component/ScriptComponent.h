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
		private:
			std::shared_ptr<BaseComponent> m_component;

			void OnCreate() override;

			void OnStart() override;

			void OnUpdate() override;

			void OnDraw() override;

			void OnDestroy() override;

			void ShowInInspector() override;

		};
	}
}
#include "Component/ScriptComponent.inl" 

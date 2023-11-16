#pragma once
#include "GalaxyAPI.h"
#include "Light.h"

namespace GALAXY 
{
	namespace Component
	{
		class DirectionalLight : public Light
		{
		public:
			DirectionalLight() {}
			DirectionalLight& operator=(const DirectionalLight& other) = default;
			DirectionalLight(const DirectionalLight&) = default;
			DirectionalLight(DirectionalLight&&) noexcept = default;
			virtual ~DirectionalLight() {}

			inline const char* GetComponentName() const override { return "Directional Light"; }

			inline virtual Shared<Component::BaseComponent> Clone() override {
					return std::make_shared<DirectionalLight>(*dynamic_cast<DirectionalLight*>(this));
			}

			void SendLightValues(Resource::Shader* shader) override;

			void ResetLightValues(Resource::Shader* shader) override;

			void ShowInInspector() override;

			void OnEditorDraw() override;

			

			inline Vec3f GetDirection() const { return m_direction; }
			inline void SetDirection(Vec3f val) { m_direction = val; }

		private:

			Vec3f m_direction = Vec3f::Down();
		};
	}
}
#include "Component/DirectionalLight.inl" 

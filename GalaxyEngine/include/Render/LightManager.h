#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Component
	{
		class Light;
	}
	namespace Render
	{
		class LightManager
		{
		public:
			LightManager() {}

			static void AddLight(Weak<Component::Light> light);
			static void RemoveLight(Weak<Component::Light> light);
			inline List<Weak<Component::Light>> GetLights() { return m_lights; }

			static void AddShader(Weak<Resource::Shader> shader);
			static void RemoveShader(Weak<Resource::Shader> shader);

			void SendLightData();
			static void ResetLightData(Component::Light* light);

			static LightManager* GetInstance();
		private:
			static Unique<LightManager> m_instance;

			List<Weak<Resource::Shader>> m_shaders;

			List<Weak<Component::Light>> m_lights;

		};
	}
}

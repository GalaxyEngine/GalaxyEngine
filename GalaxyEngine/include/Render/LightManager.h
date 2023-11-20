#pragma once
#include "GalaxyAPI.h"
#include "Utils/Define.h"
#include <array>

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

			static bool AddLight(Weak<Component::Light> light);
			static void RemoveLight(Weak<Component::Light> light);

			static void AddShader(Weak<Resource::Shader> shader);
			static void RemoveShader(Weak<Resource::Shader> shader);

			void SendLightData();
			static void ResetLightData(Component::Light* light);

			static LightManager* GetInstance();
		private:
			static Unique<LightManager> m_instance;

			List<Weak<Resource::Shader>> m_shaders;

			std::array<Weak<Component::Light>, MAX_LIGHT_NUMBER * 3> m_lights;

		};
	}
}

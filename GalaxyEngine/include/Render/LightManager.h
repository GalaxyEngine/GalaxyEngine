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

			static bool AddLight(const Weak<Component::Light>& light);
			static void RemoveLight(const Weak<Component::Light>& light);

			static void AddShader(const Weak<Resource::Shader>& shader);
			static void RemoveShader(const Weak<Resource::Shader>& shader);

			void SendLightData() const;
			void SendLightData(Resource::Shader* shader, const Vec3f& cameraPos) const;
			static void ResetLightData(Component::Light* light);
		private:

			static List<Weak<Resource::Shader>> m_shaders;

			std::array<Weak<Component::Light>, MAX_LIGHT_NUMBER * 3> m_lights;

		};
	}
}

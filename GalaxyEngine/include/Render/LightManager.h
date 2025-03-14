#pragma once
#include "GalaxyAPI.h"
#include "Utils/Define.h"
#include <array>

#include "Utils/Type.h"

namespace GALAXY
{
	namespace Resource
	{
		class Shader;
	}

	namespace Component
	{
		class Light;
	}
	namespace Render
	{
		class Camera;

		class LightManager
		{
		public:
			LightManager() {}

			static bool AddLight(const Weak<Component::Light>& light);
			static void RemoveLight(const Weak<Component::Light>& light);

			static void AddShader(const Weak<Resource::Shader>& shader);
			static void RemoveShader(const Weak<Resource::Shader>& shader);

			void SendLightData() const;
			void SendLightData(Resource::Shader* shader, const Shared<Camera>& camera) const;
			static void ResetLightData(Component::Light* light);

			void RenderShadowMaps() const;

			void SetDirty() const;

			Weak<Component::Light> GetLight(size_t index) const {return m_lights[index];}
		private:

			static List<Weak<Resource::Shader>> m_shaders;

			std::array<Weak<Component::Light>, MAX_LIGHT_NUMBER * 3> m_lights;

		};
	}
}

#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Wrapper
	{
		class Renderer;
	}
	namespace Core
	{
		class GameObject;
	}
	namespace Editor
	{
		class Gizmo
		{
		public:
			Gizmo();
			~Gizmo();

			void Update();
			void Draw();

			inline void SetGameObject(Weak<Core::GameObject> object) { m_object = object; }

		private:
			Wrapper::Renderer* m_renderer = nullptr;

			Weak<Core::GameObject> m_object = {};
		};
	}
}

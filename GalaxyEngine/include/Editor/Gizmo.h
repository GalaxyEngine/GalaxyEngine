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
	namespace Component
	{
		class Transform;
	}
	namespace Physic
	{
		// TODO : Move this in another file
		struct Ray
		{
			Vec3f origin;
			Vec3f direction;
			float scale;
		};
	}

	namespace Editor
	{
		enum class GizmoType
		{
			Translation,
			Rotation,
			Scale,
			None
		};
		enum class GizmoAxis
		{
			X,
			Y,
			Z,
			None,
		};
		enum class GizmoMode
		{
			World,
			Local
		};
		class Gizmo
		{
		public:
			Gizmo();
			~Gizmo();

			void Initialize();
			void Update();
			void Draw();

			void SetGameObject(Weak<Core::GameObject> object);
		private:
			void HandleAxis(const Physic::Ray &mouseRay);
			float CalculateRayDistance(const Physic::Ray &mouseRay, int index);

		private:
			Wrapper::Renderer *m_renderer = nullptr;

			Weak<Core::GameObject> m_object = {};
			Component::Transform *m_transform = nullptr;

			GizmoType m_type = GizmoType::Translation;
			GizmoAxis m_axis = GizmoAxis::None;
			GizmoMode m_mode = GizmoMode::World;

			float m_gizmoLength = 100.f;

			Vec3f m_gizmoCenter;
			Vec3f m_gizmoScale;

			Physic::Ray m_translateRays[3];

			Vec3f m_startPosition;
			Vec3f m_currentPosition;
		};
	}
}

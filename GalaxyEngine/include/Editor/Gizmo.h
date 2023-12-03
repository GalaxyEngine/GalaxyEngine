#pragma once
#include "GalaxyAPI.h"
#include "Core/Input.h"

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


	struct Plane
	{
		Vec3f normal;
		float distance;

		Plane(Vec3f _normal, float _distance) : normal(_normal), distance(_distance) {}
		Plane(Vec3f _normal, Vec3f _point) : normal(_normal)
		{
			distance = -_normal.Dot(_point);
		}
	};

	struct Circle
	{
		Vec3f orientation;
		Vec3f center;
		float radius;
	};

	namespace Editor
	{
		//TODO : Move this in another file
		enum class Space
		{
			Local,
			World,
		};

		inline const char* SerializeSpaceEnum()
		{
			return "Local\0World\0";
		}

		inline const char* SerializeSpaceValue(Space space)
		{
			switch (space)
			{
			case Space::Local:	return "Local";
			case Space::World:	return "World";
			default:			return "Unknown";
			}
		}

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

		class Gizmo
		{
		public:
			Gizmo();
			~Gizmo();

			void Initialize();
			void Update();
			void Draw();

			inline bool IsGizmoClicked() { return m_gizmoClicked; }

			inline Space GetGizmoMode() { return m_mode; }
			inline void SetGizmoMode(Space mode) { m_mode = mode; }

			void SetGameObject(Weak<Core::GameObject> object);

		private:

			void HandleAxis(Physic::Ray& mouseRay);

			void HandleRotation(Physic::Ray& mouseRay, const Vec3f& position);

			void HandleAction();

		private:
			Wrapper::Renderer* m_renderer = nullptr;

			Weak<Core::GameObject> m_object = {};
			Component::Transform* m_transform = nullptr;

			GizmoType m_type = GizmoType::Translation;
			GizmoAxis m_axis = GizmoAxis::None;
			Space	  m_mode = Space::World;

			float m_gizmoLength = 0.f;

			Vec3f m_gizmoCenter;
			Quat m_gizmoRotation;
			Vec3f m_gizmoScale;

			Physic::Ray m_translateRays[3];
			Circle m_rotateCircle[3];

			Vec3f m_startPosition;
			Vec3f m_currentPosition;

			bool m_gizmoClicked = false;
		};
	}
}

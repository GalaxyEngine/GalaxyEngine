#pragma once
#include "GalaxyAPI.h"
namespace GALAXY
{
	namespace Physic {
		struct Plane
		{
			Vec3f normal = Vec3f::Up();
			float distance = 0.f;

			Plane() = default;
			Plane(const Vec3f& _point, const Vec3f& _normal)
			{
				normal = _normal.GetNormalize();
				distance = normal.Dot(_point);
			}
			float GetDistanceToPlane(const Vec3f& _point) const { return normal.Dot(_point) - distance; }

			void DebugDraw() const;
		};
	}
}

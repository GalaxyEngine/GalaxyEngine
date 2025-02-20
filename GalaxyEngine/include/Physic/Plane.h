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
			Plane(const Vec3f& normal, float distance) : normal(normal), distance(distance) {}
			Plane(const Vec3f& _point, const Vec3f& _normal)
			{
				normal = _normal.GetNormalize();
				distance = normal.Dot(_point);
			}
			float GetDistanceToPlane(const Vec3f& _point) const { return normal.Dot(_point) - distance; }
			float GetDistanceFromPlane(const Vec3f& _point) const { return _point.Dot(normal) + distance; }
			Vec3f ProjectPointOntoPlane(const Vec3f& point) const;

			static Plane PlaneFromTri(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2);

			void DebugDraw() const;
		};
	}
}

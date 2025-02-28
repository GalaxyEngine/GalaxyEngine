#pragma once
#include "GalaxyAPI.h"

namespace GALAXY 
{
	namespace Component
	{
		class Collider;
	}
	namespace Physic
	{
		struct GALAXY_API Ray
		{
			Vec3f origin;
			Vec3f direction;
			float scale;
		};

		struct GALAXY_API RaycastHit
		{
			bool hit = false;
			Vec3f point;
			Vec3f normal;
			float distance;

			Shared<Component::Collider> collider = {};
		};

		bool Raycast(const Ray& ray, float maxDistance, RaycastHit& hit);
		bool Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance, RaycastHit& hit);
	}
}

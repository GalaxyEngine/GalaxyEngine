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
			Vec3f point;
			Vec3f normal;
			float distance;

			Shared<Component::Collider> collider = {};
		};
	}
}

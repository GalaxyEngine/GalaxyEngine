#pragma once
#include "GalaxyAPI.h"
#include "Physic/Plane.h"
namespace GALAXY
{
	namespace Render {
		class Camera;
	}
	namespace Physic {
		struct Frustum
		{
			Frustum();

			Plane planes[6];

			void Create(Render::Camera* camera);

			void DebugDraw() const;
		};
	}
}

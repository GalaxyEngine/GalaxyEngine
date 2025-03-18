#pragma once
#include "GalaxyAPI.h"
#include "Physic/Plane.h"
#include <array>
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

			std::array<Vec3f, 8> GetCorners() const;

			void DebugDraw() const;
		};
	}
}

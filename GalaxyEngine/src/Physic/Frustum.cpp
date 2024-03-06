#include "pch.h"
#include "Physic/Frustum.h"

#include "Render/Camera.h"

namespace GALAXY 
{

	Physic::Frustum::Frustum()
	{

	}

	void Physic::Frustum::Create(Render::Camera* camera)
	{
		const auto _far = camera->GetFar();
		const auto _near = camera->GetNear();

		const float fovY = -camera->GetFOV() * DegToRad;
		const float halfVSide = _far * tanf(fovY * .5f);
		const float halfHSide = halfVSide * camera->GetAspectRatio();
		const auto camPos = camera->GetTransform()->GetWorldPosition();
		const auto camFront = camera->GetTransform()->GetForward();
		const auto camUp = camera->GetTransform()->GetUp();
		const auto camRight = camera->GetTransform()->GetRight();
		const Vec3f frontMultFar = camFront * _far;

		planes[0] = Physic::Plane(camPos + camFront * _near, camFront);
		planes[1] = Physic::Plane(camPos + frontMultFar, -camFront);
		planes[2] = Physic::Plane(camPos, (frontMultFar - camRight * halfHSide).Cross(camUp));
		planes[3] = Physic::Plane(camPos, camUp.Cross(frontMultFar + camRight * halfHSide));
		planes[4] = Physic::Plane(camPos, camRight.Cross(frontMultFar - camUp * halfVSide));
		planes[5] = Physic::Plane(camPos, (frontMultFar + camUp * halfVSide).Cross(camRight));
	}

	void Physic::Frustum::DebugDraw() const
	{
		for (int i = 0; i < 6; i++)
		{
			planes[i].DebugDraw();
		}
	}

}

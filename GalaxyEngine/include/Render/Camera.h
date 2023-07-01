#pragma once
#include "GalaxyAPI.h"
#include <Maths/Maths.h>
#include "Component/Transform.h"
namespace GALAXY
{
	namespace Render
	{
		class Camera
		{
		public:
			Camera();
			Camera& operator=(const Camera& other) = default;
			Camera(const Camera&) = default;
			Camera(Camera&&) noexcept = default;
			virtual ~Camera();

			void Update();

			virtual Component::Transform* GetTransform();

			Mat4 GetViewMatrix();
			Mat4 GetProjectionMatrix();
			Mat4 GetViewProjectionMatrix();
		private:
			void StartLooking();
			void StopLooking();
		protected:
			float p_fov = 70.f;
			float p_far = 0.1f;
			float p_near = 1000.f;
			float p_aspectRatio = 4.f / 3.f;

			Vec4f p_clearColor = Vec4f(0.45f, 0.55f, 0.60f, 1.00f);

		private:
			float m_movementSpeed = 10.f;
			float m_fastMovementSpeed = 100.f;
			float m_freeLookSensitivity = 3.f;
			float m_zoomSensitivity = 1.f;
			float m_fastZoomSensitivity = 5.f;

			bool m_looking = false;

			std::unique_ptr<Component::Transform> m_transform;

		};
	}
}

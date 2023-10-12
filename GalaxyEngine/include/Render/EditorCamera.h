#pragma once
#include "GalaxyAPI.h"
#include "Camera.h"

namespace GALAXY 
{
	namespace Render
	{
		class EditorCamera : public Camera
		{
		public:
			EditorCamera();
			EditorCamera& operator=(const EditorCamera& other) = default;
			EditorCamera(const EditorCamera&) = default;
			EditorCamera(EditorCamera&&) noexcept = default;
			virtual ~EditorCamera();

			void Update();

			Component::Transform* GetTransform() override;

			class Framebuffer* GetOutlineFramebuffer() const { return m_outlineFramebuffer; }
		private:

			void StartLooking();
			void StopLooking();

			class Framebuffer* m_outlineFramebuffer = nullptr;

		private:
			float m_movementSpeed = 10.f;
			float m_fastMovementSpeed = 100.f;
			float m_freeLookSensitivity = 100.f;
			float m_zoomSensitivity = 1.f;
			float m_fastZoomSensitivity = 5.f;

			bool m_looking = false;

			std::unique_ptr<Component::Transform> m_transform;

		};
	}
}

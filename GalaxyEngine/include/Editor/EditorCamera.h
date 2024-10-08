#pragma once
#include "GalaxyAPI.h"
#include "Render/Camera.h"

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
			~EditorCamera() override;

			void Update();

			void DisplayCameraSettings() override;

			inline Component::Transform* GetTransform() const override {
				return m_transform.get();
			}

			inline class Framebuffer* GetOutlineFramebuffer() const { return m_outlineFramebuffer; }

			inline bool IsLooking() const { return m_looking; }
		private:

			void StartLooking();
			void StopLooking();

			class Framebuffer* m_outlineFramebuffer = nullptr;

		private:
			float m_movementSpeed = 10.f;
			float m_multiplierSpeed = 1.f;
			float m_fastMovementSpeed = 100.f;
			float m_freeLookSensitivity = 0.2f;
			float m_zoomSensitivity = 1.f;
			float m_fastZoomSensitivity = 5.f;

			bool m_looking = false;

			std::unique_ptr<Component::Transform> m_transform;

		};
	}
}

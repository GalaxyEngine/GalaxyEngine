#pragma once
#include "GalaxyAPI.h"
#include <Maths/Maths.h>
#include "Component/Transform.h"
namespace GALAXY
{
	namespace Resource
	{
		class Texture;
	}
	namespace Physic
	{
		struct Ray;
	}
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

			static Shared<class EditorCamera> GetEditorCamera();
			static Weak<Camera> GetCurrentCamera();

			void Begin();

			void End();

			void DisplayCameraSettings();

			inline virtual Component::Transform* GetTransform() const = 0;

			inline void SetSize(const Vec2i& framebufferSize);

			virtual bool IsVisible();

			inline Mat4 GetViewMatrix() const;
			inline Mat4 GetProjectionMatrix() const;
			inline Mat4 GetViewProjectionMatrix() const;
			inline Vec4f GetClearColor() const { return p_clearColor; }
			Physic::Ray ScreenPointToRay(const Vec3f& point);

			virtual Vec2i GetScreenResolution() const;
			inline Vec2f ToViewport(const Vec2f& pos) const;
			inline Vec3f UnProject(const Vec3f& point) const;

			Weak<Resource::Texture> GetRenderTexture();

			inline float GetFar() const { return p_far; }
			inline void SetFar(float val) { p_far = val; }

			inline float GetNear() const { return p_near; }
			inline void SetNear(float val) { p_near = val; }

			inline float GetFOV() const { return p_fov; }
			inline void SetFOV(float val) { p_fov = val; }

		protected:
			float p_fov = 70.f;
			float p_far = 1000.f;
			float p_near = 0.03f;
			float p_aspectRatio = 4.f / 3.f;
			Vec2i p_framebufferSize;

			Vec4f p_clearColor = Vec4f(0.45f, 0.55f, 0.60f, 1.00f);

			class Framebuffer* m_framebuffer = nullptr;

		private:

		};
	}
}
#include "Render/Camera.inl" 

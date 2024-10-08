#pragma once
#include "GalaxyAPI.h"
#include "Component/Transform.h"
#include "Physic/Frustum.h"
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
	namespace Component
	{
		class CameraComponent;
	}
	namespace Render
	{
		class Framebuffer;
		class Camera
		{
		public:
			Camera();
			Camera& operator=(const Camera& other) = default;
			Camera(const Camera&) = default;
			Camera(Camera&&) noexcept = default;
			virtual ~Camera() = default;

#ifdef WITH_EDITOR
			static Shared<class EditorCamera> GetEditorCamera();
#endif
			static Shared<Component::CameraComponent> GetMainCamera();

			static Shared<Camera> GetCurrentCamera();

			void Begin() const;

			void End() const;

			virtual void DisplayCameraSettings();

			inline virtual Component::Transform* GetTransform() const = 0;

			inline void SetSize(const Vec2i& framebufferSize);

			virtual bool IsVisible() const;

			inline Mat4 GetViewMatrix() const;
			inline Mat4 GetProjectionMatrix() const;
			inline Mat4 GetViewProjectionMatrix() const;
			inline Vec4f GetClearColor() const { return p_clearColor; }
			Physic::Ray ScreenPointToRay(const Vec3f& point);

			virtual Vec2i GetScreenResolution() const;
			inline Vec2f ToViewport(const Vec2f& pos) const;
			inline Vec3f UnProject(const Vec3f& point) const;

			Weak<Resource::Texture> GetRenderTexture() const;

			float GetAspectRatio() const { return p_aspectRatio; }

			inline float GetFar() const { return p_far; }
			inline void SetFar(float val) { p_far = val; }

			inline float GetNear() const { return p_near; }
			inline void SetNear(float val) { p_near = val; }

			inline float GetFOV() const { return p_fov; }
			inline void SetFOV(float val) { p_fov = val; }

			Shared<Framebuffer> GetFramebuffer() const { return p_framebuffer; }
			void SetClearColor(const Vec4f& clearColor) { p_clearColor = clearColor; }

			void CreateFrustum(); 

			Physic::Frustum& GetFrustum() { return p_frustum; }
		protected:
			float p_fov = 70.f;
			float p_far = 1000.f;
			float p_near = 0.03f;
			float p_aspectRatio = 4.f / 3.f;
			Vec2i p_framebufferSize;

			Vec4f p_clearColor = Vec4f(0.45f, 0.55f, 0.60f, 1.00f);

			Shared<Framebuffer> p_framebuffer = nullptr;

			Physic::Frustum p_frustum;
		};
	}
}
#include "Render/Camera.inl" 

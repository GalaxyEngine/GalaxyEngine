#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"
#include "Editor/EditorIcon.h"
#include "Render/Camera.h"

namespace GALAXY
{
	namespace Resource
	{
		class Scene;
	}
	namespace Component
	{
		class CameraComponent : public IComponent<CameraComponent>, public Render::Camera
		{
		public:
			CameraComponent() = default;
			CameraComponent& operator=(const CameraComponent& other) = default;
			CameraComponent(const CameraComponent&) = default;
			CameraComponent(CameraComponent&&) noexcept = default;
			~CameraComponent() override = default;

			const char* GetComponentName() const override { return "CameraComponent"; }

			void OnCreate() override;
			void OnDraw() override;
			void OnEditorDraw() override;
			void ShowInInspector() override;

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& deserializer) override;
			void AfterLoad() override;

			bool IsVisible() const override;

			inline Component::Transform* GetTransform() const override {
				if (p_gameObject == nullptr)
					return nullptr;
				return p_gameObject->GetTransform();
			}
			Vec2i GetScreenResolution() const override;

			void SetMainCamera();

		private:
			friend Resource::Scene;

			bool m_isMainCamera = false;
			Editor::EditorIcon m_editorIcon;
		};
	}
}

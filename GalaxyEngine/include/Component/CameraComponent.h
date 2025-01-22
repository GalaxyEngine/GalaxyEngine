#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"
#include "Core/GameObject.h"
#include "Render/Camera.h"

#ifdef WITH_EDITOR
#include "Editor/EditorIcon.h"
#endif

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
			void OnDestroy() override;
			void OnDraw() override;
			void OnEditorDraw() override;
			EDITOR_ONLY void ShowInInspector() override;

			void Serialize(CppSer::Serializer& serializer) override;
			void Deserialize(CppSer::Parser& parser) override;
			void AfterLoad() override;

			bool IsVisible() const override;

			Transform* GetTransform() const override {
				if (p_gameObject == nullptr)
					return nullptr;
				return p_gameObject->GetTransform();
			}
			Vec2i GetScreenResolution() const override;

			void SetMainCamera();
			bool IsMainCamera() const { return m_isMainCamera; }
		private:
			friend Resource::Scene;

			bool m_isMainCamera = false;
#ifdef WITH_EDITOR
			Editor::EditorIcon m_editorIcon;

#endif
			uint64_t m_postprocessID = -1;
		};
	}
}

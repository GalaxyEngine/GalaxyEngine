#include "pch.h"
#include "Component/CameraComponent.h"

#include "Resource/Scene.h"
#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"

#include "Editor/UI/EditorUIManager.h"
#include "Render/Framebuffer.h"

namespace GALAXY
{

	void Component::CameraComponent::OnCreate()
	{
		Weak<BaseComponent> self = GetGameObject()->GetComponentWithIndex(p_id);
		const auto scene = GetGameObject()->GetScene();
		if (!scene)
			return;
		scene->AddCamera(std::static_pointer_cast<CameraComponent>(self.lock()));
		m_editorIcon.SetIconTexture(Resource::ResourceManager::GetOrLoad<Resource::Texture>(CAMERA_ICON_PATH));
	}

	void Component::CameraComponent::OnDraw()
	{

	}

	void Component::CameraComponent::OnEditorDraw()
	{
		const Core::GameObject* game_object = GetGameObject();

		m_editorIcon.SetPosition(GetTransform()->GetModelMatrix().GetTranslation());
		m_editorIcon.Render(game_object->GetSceneGraphID());

		if (game_object->IsSelected())
		{
			auto instance = Wrapper::Renderer::GetInstance();
			Vec2f screenResolution = GetScreenResolution(); // Assuming this doesn't change often
			std::vector<Vec3f> points(8);

			// Calculate all points
			points[0] = UnProject(Vec3f{ 0, 0, p_near });
			points[1] = UnProject(Vec3f{ screenResolution.x, 0, p_near });
			points[2] = UnProject(Vec3f{ 0, screenResolution.y, p_near });
			points[3] = UnProject(Vec3f{ screenResolution, p_near });
			points[4] = UnProject(Vec3f{ 0, 0, p_far });
			points[5] = UnProject(Vec3f{ screenResolution.x, 0, p_far });
			points[6] = UnProject(Vec3f{ 0, screenResolution.y, p_far });
			points[7] = UnProject(Vec3f{ screenResolution, p_far });

			// Draw near plane
			instance->DrawLine(points[0], points[1]);
			instance->DrawLine(points[1], points[3]);
			instance->DrawLine(points[3], points[2]);
			instance->DrawLine(points[2], points[0]);

			// Draw far plane
			instance->DrawLine(points[4], points[5]);
			instance->DrawLine(points[5], points[7]);
			instance->DrawLine(points[7], points[6]);
			instance->DrawLine(points[6], points[4]);

			// Draw lines between near and far planes
			for (int i = 0; i < 4; ++i)
			{
				instance->DrawLine(points[i], points[i + 4]);
			}
		}
	}

	void Component::CameraComponent::ShowInInspector()
	{
		DisplayCameraSettings();
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0));
		if (ImGui::Begin("##CameraPreview", 0, ImGuiWindowFlags_NoTitleBar))
		{
			auto startPos = ImGui::GetWindowContentRegionMin();
			Wrapper::GUI::TextureImage(p_framebuffer->GetRenderTexture().lock().get(), ImGui::GetWindowSize() - startPos * 2, { 0, 1 }, { 1, 0 });
		}
		//ImGui::PopStyleVar();
		ImGui::End();
	}

	void Component::CameraComponent::Serialize(CppSer::Serializer& serializer)
	{
		serializer << CppSer::Pair::Key << "Near" << CppSer::Pair::Value << p_near;
		serializer << CppSer::Pair::Key << "Far" << CppSer::Pair::Value << p_far;
		serializer << CppSer::Pair::Key << "Fov" << CppSer::Pair::Value << p_fov;
		serializer << CppSer::Pair::Key << "Clear Color" << CppSer::Pair::Value << p_clearColor;
		serializer << CppSer::Pair::Key << "Is Main Camera" << CppSer::Pair::Value << m_isMainCamera;
		Resource::IResource::SerializeResource(serializer, "PP Shader", p_framebuffer->GetPostProcessShader());
	}

	void Component::CameraComponent::Deserialize(CppSer::Parser& parser)
	{
		p_near = parser["Near"].As<float>();
		p_far = parser["Far"].As<float>();
		p_fov = parser["Fov"].As<float>();
		p_clearColor = parser["Clear Color"].As<Vec4f>();
		m_isMainCamera = parser["Is Main Camera"].As<bool>();

		uint64_t uuid = parser["PP Shader"].As<uint64_t>();
		p_framebuffer->SetPostProcessShader(Resource::ResourceManager::GetOrLoad<Resource::PostProcessShader>(uuid));
	}

	void Component::CameraComponent::AfterLoad()
	{
		SetMainCamera();
	}

	bool Component::CameraComponent::IsVisible() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetGameWindow()->IsVisible() || GetGameObject()->IsSelected();
	}

	Vec2i Component::CameraComponent::GetScreenResolution() const
	{
		return Editor::UI::EditorUIManager::GetInstance()->GetGameWindow()->GetImageSize();
	}

	void Component::CameraComponent::SetMainCamera()
	{
		Weak<BaseComponent> self = GetGameObject()->GetComponentWithIndex(p_id);
		GetGameObject()->GetScene()->SetMainCamera(std::static_pointer_cast<CameraComponent>(self.lock()));
	}

}

#include "pch.h"

#include "Render/Camera.h"
#include "Render/Framebuffer.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"
#include "Core/Input.h"

#include "Resource/ResourceManager.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Scene.h"

#include "Physic/2D/Rect.h"
#include "Physic/Ray.h"
#include "Resource/Mesh.h"

#ifdef WITH_EDITOR
#include "Editor/EditorCamera.h"
#include "Editor/UI/EditorUIManager.h"
#include "Editor/Gizmo.h"
#endif

namespace GALAXY
{

	Render::Camera::Camera()
	{
		p_framebuffer = std::make_shared<Framebuffer>(Core::Application::GetInstance().GetWindow()->GetSize());
	}

	void Render::Camera::DisplayCameraSettings()
	{
		constexpr float contentWidth = 200.f;

		// Use the content width as the width for the controls
		ImGui::PushItemWidth(contentWidth);
		ImGui::SliderFloat("FOV", &p_fov, 25.f, 120.f);
		ImGui::DragFloatRange2("Near/Far", &p_near, &p_far, 0.1f);
		ImGui::ColorEdit4("Clear Color", &p_clearColor.x);
		Resource::ResourceManager::ResourceField<Resource::Cubemap>(p_skybox, "Cubemap");
		std::string buttonName;
		if (const Shared<Resource::PostProcessShader> shader = p_framebuffer->GetPostProcessShader().lock())
			buttonName = shader->GetFileInfo().GetFileNameNoExtension();
		else
			buttonName = "Set Post Process Shader";
		if (ImGui::Button(buttonName.c_str(), Vec2f(contentWidth, 0)))
		{
			ImGui::OpenPopup("PostProcessPopup");
		}
		ImGui::PopItemWidth();
		Weak<Resource::PostProcessShader> ppShader;
		if (Resource::ResourceManager::GetInstance()->ResourcePopup("PostProcessPopup", ppShader))
		{
			p_framebuffer->SetPostProcessShader(ppShader);
		}
	}

	Physic::Ray Render::Camera::ScreenPointToRay(const Vec3f& point)
	{
		const Vec3f position = GetTransform()->GetWorldPosition();
		const Vec3f unprojectPoint = UnProject({ point.x, point.y, point.z });
		const Vec3f direction = (unprojectPoint - position).GetNormalize();
		return Physic::Ray(position, direction, point.z);
	}

	Physic::Ray Render::Camera::ScreenPointToRay(const Vec2f& point)
	{
		return ScreenPointToRay(Vec3f(point, p_far));
	}

	Vec2i Render::Camera::GetScreenResolution() const
	{
#ifdef WITH_EDITOR
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->GetImageSize();
#else
		return Core::Application::GetInstance().GetWindow()->GetSize();
#endif
	}

	Weak<Resource::Texture> Render::Camera::GetRenderTexture() const
	{
		return p_framebuffer->GetRenderTexture();
	}

	void Render::Camera::CreateFrustum()
	{
		p_frustum.Create(this);
	}

	void Render::Camera::RenderSkybox() const
	{
		if (!p_skybox.lock() || !p_skybox.lock()->HasBeenSent() || !p_skybox.lock()->IsLoaded())
			return;
		p_skybox.lock()->RenderCubemap();
		/*
		for (const auto& shader : Resource::ResourceManager::GetAllResources<Resource::Shader>())
		{
			Wrapper::Renderer::GetInstance()->BindCubemap(p_skybox.lock().get());
			shader.lock()->SendInt("skybox", 0);
		}
		*/

		/*
		Wrapper::Renderer::GetInstance()->ActiveDepth(false);
		static auto cube = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(CUBE_PATH);
		if (auto mesh = cube.lock())
		{
			if (!mesh->HasBeenSent() || !mesh->IsLoaded())
				return;
			Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
			renderer->BindVertexArray(mesh->GetVertexArrayIndex());
			auto skyboxShader = Resource::ResourceManager::GetOrLoad<Resource::Shader>(SKYBOX_PATH).lock();
			if (!skyboxShader || !skyboxShader->HasBeenSent() || !skyboxShader->IsLoaded())
				return;
			const Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
			skyboxShader->SendMat4("Model", Mat4(1));
			skyboxShader->SendMat4("MVP", this->GetViewProjectionMatrix());
			
			Wrapper::Renderer::GetInstance()->BindCubemap(p_skybox.lock().get());
			skyboxShader->SendInt("skybox", 0);
			
			renderer->DrawArrays(0, 36);
			
			renderer->UnbindVertexArray();
		}
		
		// p_skybox.lock()->Draw();
		Wrapper::Renderer::GetInstance()->ActiveDepth(true);
		*/
	}

#ifdef WITH_EDITOR
	Shared<Render::EditorCamera> Render::Camera::GetEditorCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetEditorCamera();
	}
#endif

	Shared<Component::CameraComponent> Render::Camera::GetMainCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetMainCamera();
	}

	Shared<Render::Camera> Render::Camera::GetCurrentCamera()
	{
		return Core::SceneHolder::GetCurrentScene()->GetCurrentCamera();
	}

	bool Render::Camera::IsVisible() const
	{
#ifdef WITH_EDITOR
		return Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow()->IsVisible();
#else
		return true;
#endif
	}

	void Render::Camera::Begin() const
	{
		p_framebuffer->Begin(p_framebufferSize);
		Wrapper::Renderer::GetInstance()->ClearColorAndBuffer(p_clearColor);
		RenderSkybox();
	}

	void Render::Camera::End() const
	{
		p_framebuffer->End(p_framebufferSize, p_clearColor);
	}

}
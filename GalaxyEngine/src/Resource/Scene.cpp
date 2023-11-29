#include "pch.h"

#include "Resource/Scene.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"
#include "Render/Grid.h"
#include "Render/Framebuffer.h"
#include "Render/LightManager.h"

#include "Editor/Gizmo.h"
#include "Editor/ActionManager.h"

#include "Component/MeshComponent.h"

#include "Utils/Parser.h"

#include "Wrapper/Window.h"

#include "Core/Input.h"

using namespace Resource;
namespace GALAXY
{
	Scene::Scene(const Path& path) : IResource(path)
	{
		m_root = std::make_shared<Core::GameObject>(GetFileInfo().GetFileNameNoExtension());
		m_root->m_scene = this;
	}

	Scene::~Scene()
	{
	}
	
	void Scene::Initialize()
	{
		m_editorCamera = std::make_unique<Render::EditorCamera>();

		m_gizmo = std::make_shared<Editor::Gizmo>();

		m_grid = std::make_shared<Render::Grid>();
		m_grid->Initialize();
		p_hasBeenSent = true;

		m_actionManager = std::make_shared<Editor::ActionManager>();
	}

	void Scene::Update()
	{
		static Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		static Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		static Editor::UI::Inspector* inspector = Editor::UI::EditorUIManager::GetInstance()->GetInspector();
		static Editor::UI::SceneWindow* sceneWindow = Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow();
		static Render::LightManager* lightManager = Render::LightManager::GetInstance();
		static Editor::UI::EditorUIManager* editorUIManager = Editor::UI::EditorUIManager::GetInstance();
		
		editorUIManager->DrawUI();
		if (!HasBeenSent()) // if it reload the current scene from the Main Bar menu
			return;

		m_root->UpdateSelfAndChild();

		m_actionManager->Update();

		if (m_editorCamera->IsVisible()) {
			SetCurrentCamera(m_editorCamera);

			// Outline 
			{
				auto outlineFrameBuffer = m_editorCamera->GetOutlineFramebuffer();
				outlineFrameBuffer->Begin();

				renderer->ClearColorAndBuffer(Vec4f(0));

				renderer->SetRenderingType(Render::RenderType::Outline);
				for (auto& selected : inspector->GetSelected())
				{
					selected.lock()->DrawSelfAndChild();
				}
				renderer->SetRenderingType(Render::RenderType::Default);
				outlineFrameBuffer->End();
			}

			// Bind Default Framebuffer
			m_currentCamera.lock()->Begin();
			renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());
			m_currentCamera.lock()->SetSize(Core::Application::GetInstance().GetWindow()->GetSize());

			m_editorCamera->Update();

			m_gizmo->Update();

			static Vec3f cameraPosition = Vec3f::Zero();
			static Vec3f clickPosition = Vec3f::Zero();
			if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1) && sceneWindow->IsHovered())
			{
				renderer->ClearColorAndBuffer(Vec4f(1));

				renderer->SetRenderingType(Render::RenderType::Picking);
				m_root->DrawSelfAndChild();
				renderer->SetRenderingType(Render::RenderType::Default);

				// Calculate Mouse Position
				Vec2i mainWindowSize = window->GetSize();
				Vec2f imageSize = sceneWindow->GetImageSize();
				Vec2f mousePosition = sceneWindow->GetMousePosition();
				mousePosition = mousePosition * Vec2f(mainWindowSize.x / imageSize.x, mainWindowSize.y / imageSize.y);
				mousePosition.y = mainWindowSize.y - mousePosition.y;

				Vec4f color = renderer->ReadPixelColor(mousePosition);

				uint64_t pickedID = static_cast<uint64_t>(color.x + color.y * 256 + color.z * 256 * 256);

				if (m_gizmo->IsGizmoClicked())
				{ }
				else if (Shared<Core::GameObject> gameObject = GetWithSceneGraphID(pickedID).lock())
					inspector->SetSelected(gameObject);
				else
					inspector->ClearSelected();

				renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());

				auto ray = m_editorCamera->ScreenPointToRay(sceneWindow->GetMousePosition());
				cameraPosition = ray.origin;
				clickPosition = ray.origin + ray.direction * ray.scale;
			}

			lightManager->SendLightData();

			renderer->DrawLine(cameraPosition, clickPosition, Vec4f(0, 1, 0, 1), 4.f);

			if (*Core::Application::GetInstance().GetDrawGridPtr())
				m_grid->Draw();

			m_root->DrawSelfAndChild();
			m_gizmo->Draw();
			
			m_currentCamera.lock()->End();
		}
	}

	void Scene::SetCurrentCamera(Weak<Render::Camera> camera)
	{
		m_currentCamera = camera;
		m_VP = m_currentCamera.lock()->GetViewProjectionMatrix();
		m_cameraUp = m_currentCamera.lock()->GetTransform()->GetUp();
		m_cameraRight = m_currentCamera.lock()->GetTransform()->GetRight();
	}

#pragma region Resource Methods
	void Resource::Scene::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;

		Utils::Parser parser(GetFileInfo().GetFullPath());
		m_root->m_scene = this;
		m_root->Deserialize(parser);

		p_loaded = true;
		m_root->AfterLoad();
		SendRequest();
	}

	void Scene::Unload()
	{
		m_root->Destroy();
	}

	void Scene::Send()
	{
		Initialize();
	}

	void Scene::Save(const Path& fullPath)
	{
		Utils::Serializer serializer(fullPath.empty() ? p_fileInfo.GetFullPath() : fullPath);

		if (!m_root)
			return;
		m_root->Serialize(serializer);
		serializer.CloseFile();
	}

	Weak<Scene> Scene::Create(const Path& path)
	{
		Scene scene(path);
		scene.Save();
		return Resource::ResourceManager::GetInstance()->GetOrLoad<Scene>(path);
	}
#pragma endregion
}
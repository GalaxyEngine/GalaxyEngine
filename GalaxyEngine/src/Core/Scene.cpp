#include "pch.h"

#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "EditorUI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/SceneResource.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"
#include "Render/Grid.h"
#include "Render/Framebuffer.h"

#include "Component/MeshComponent.h"

#include "Wrapper/Window.h"

#include "Core/Input.h"

using namespace Core;
namespace GALAXY
{
	Scene::Scene()
	{
		m_root = std::make_shared<GameObject>("Scene");
		m_editorCamera = std::make_unique<Render::EditorCamera>();

		m_grid = std::make_shared<Render::Grid>();
		m_grid->Initialize();
	}

	Scene::~Scene()
	{
	}

	void Scene::Update()
	{
		static Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		static Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
		static EditorUI::Inspector* inspector = EditorUI::EditorUIManager::GetInstance()->GetInspector();
		static EditorUI::SceneWindow* sceneWindow = EditorUI::EditorUIManager::GetInstance()->GetSceneWindow();

		EditorUI::EditorUIManager::GetInstance()->DrawUI();

		m_root->UpdateSelfAndChild();

		if (m_editorCamera->IsVisible()) {
			SetCurrentCamera(m_editorCamera);

			// Outline 
			{
				auto outlineFrameBuffer = m_editorCamera->GetOutlineFramebuffer();
				outlineFrameBuffer->Begin();

				renderer->ClearColorAndBuffer(Vec4f(0));

				renderer->SetRenderingType(Render::RenderType::OUTLINE);
				for (auto& selected : inspector->GetSelected())
				{
					selected.lock()->DrawSelfAndChild();
				}
				renderer->SetRenderingType(Render::RenderType::DEFAULT);
				outlineFrameBuffer->End();
			}

			// Bind Default Framebuffer
			m_currentCamera.lock()->Begin();
			renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());
			m_currentCamera.lock()->SetSize(Core::Application::GetInstance().GetWindow()->GetSize());

			m_editorCamera->Update();

			if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1) && sceneWindow->IsHovered())
			{
				renderer->ClearColorAndBuffer(Vec4f(1));

				renderer->SetRenderingType(Render::RenderType::PICKING);
				m_root->DrawSelfAndChild();
				renderer->SetRenderingType(Render::RenderType::DEFAULT);

				// Calculate Mouse Position
				Vec2i mainWindowSize = window->GetSize();
				Vec2f imageSize = sceneWindow->GetImageSize();
				Vec2f mousePosition = sceneWindow->GetMousePosition();
				mousePosition = mousePosition * Vec2f(mainWindowSize.x / imageSize.x, mainWindowSize.y / imageSize.y);
				mousePosition.y = mainWindowSize.y - mousePosition.y;

				Vec4f color = renderer->ReadPixelColor(mousePosition);

				uint64_t pickedID = static_cast<uint64_t>(color.x + color.y * 256 + color.z * 256 * 256);

				if (auto gameObject = GetWithIndex(pickedID).lock())
					inspector->SetSelected(gameObject);
				else
					inspector->ClearSelected();

				renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());
			}

			if (*Core::Application::GetInstance().GetDrawGrid())
				m_grid->Draw();

			m_root->DrawSelfAndChild();

			
			renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Right() * 5.f, Vec4f(1, 0, 0, 1), 5.f);
			renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Up() * 5.f, Vec4f(0, 1, 0, 1), 5.f);
			renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Forward() * 5.f, Vec4f(0, 0, 1, 1), 5.f);
			
			m_currentCamera.lock()->End();
		}

		SwitchSceneUpdate();
	}

	std::weak_ptr<GameObject> Scene::GetRootGameObject() const
	{
		return m_root;
	}

	uint64_t Scene::GetFreeIndex()
	{
		uint64_t index = 0;
		while (m_objectList.find(index) != m_objectList.end()) {
			index++;
		}
		return index;
	}

	void Scene::RemoveObject(GameObject* object)
	{
		auto shared = std::find_if(m_objectList.begin(), m_objectList.end(), [&](const std::pair<uint64_t, std::shared_ptr<GameObject>>& element) {
			return element.second.get() == object; });
		shared->second->RemoveFromParent();
		if (shared != m_objectList.end())
		{
			m_objectList.erase(shared);
		}
	}

	std::weak_ptr<GameObject> Scene::GetWithIndex(uint64_t index)
	{
		if (m_objectList.count(index))
		{
			return m_objectList.at(index);
		}
		return std::weak_ptr<GameObject>();
	}

	void Scene::SetCurrentCamera(std::weak_ptr<Render::Camera> camera)
	{
		m_currentCamera = camera;
		m_VP = m_currentCamera.lock()->GetViewProjectionMatrix();
	}

	void Scene::Save()
	{
		if (!m_resource.lock())
			return;
		m_resource.lock()->Save();
	}

	void Scene::AddObject(std::shared_ptr<GameObject> gameObject)
	{
		if (!m_objectList.count(gameObject->m_id))
		{
			m_objectList[gameObject->m_id] = gameObject;
		}
		else
		{
			gameObject->m_id = GetFreeIndex();
			m_root->AddChild(gameObject);
		}
	}

	void Scene::SwitchSceneUpdate()
	{
		if (!m_resource.lock() || !m_resource.lock()->IsLoaded())
			return;
		m_objectList.clear();
		m_root.reset();
		m_root = m_resource.lock()->m_root;

		for (auto& child : m_root->GetAllChildren())
		{
			AddObject(child.lock());
		}
		m_resource.reset();
	}

	void Scene::SwitchScene(Weak<Resource::SceneResource> sceneResource)
	{
		if (!sceneResource.lock())
			return;
		m_resource = sceneResource;
		if (m_resource.lock()->IsLoaded())
			m_resource.lock()->Reload();
	}

	void Scene::SaveScene(Weak<Resource::SceneResource> sceneResource)
	{
		sceneResource.lock().get()->m_root = m_root;
	}

}
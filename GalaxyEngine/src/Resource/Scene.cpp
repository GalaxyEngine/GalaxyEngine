#include "pch.h"

#include "Resource/Scene.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "EditorUI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Render/EditorCamera.h"
#include "Render/Grid.h"
#include "Render/Framebuffer.h"

#include "Editor/Gizmo.h"

#include "Component/MeshComponent.h"

#include "Utils/Parser.h"

#include "Wrapper/Window.h"

#include "Core/Input.h"

using namespace Resource;
namespace GALAXY
{
	Scene::Scene(const std::filesystem::path& path) : IResource(path)
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

			m_gizmo->Update();

			static Vec3f cameraPosition = Vec3f::Zero();
			static Vec3f clickPosition = Vec3f::Zero();
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
				else if (!m_gizmo->IsGizmoClicked())
					inspector->ClearSelected();

				renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());

				auto ray = m_editorCamera->ScreenPointToRay(sceneWindow->GetMousePosition());
				cameraPosition = ray.origin;
				clickPosition = ray.origin + ray.direction * ray.scale;
			}
			renderer->DrawLine(cameraPosition, clickPosition, Vec4f(0, 1, 0, 1), 4.f);

			if (*Core::Application::GetInstance().GetDrawGrid())
				m_grid->Draw();

			m_root->DrawSelfAndChild();
			m_gizmo->Draw();
			
			m_currentCamera.lock()->End();
		}
	}

	std::weak_ptr <Core::GameObject> Scene::GetRootGameObject() const
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

	void Scene::RemoveObject(Core::GameObject* object)
	{
		auto shared = std::find_if(m_objectList.begin(), m_objectList.end(), [&](const std::pair<uint64_t, std::shared_ptr<Core::GameObject>>& element) {
			return element.second.get() == object; });
		if (shared != m_objectList.end()) {
			shared->second->RemoveFromParent();
			if (shared != m_objectList.end())
			{
				m_objectList.erase(shared);
			}
		}
	}

	std::weak_ptr<Core::GameObject> Scene::GetWithIndex(uint64_t index)
	{
		if (m_objectList.count(index))
		{
			return m_objectList.at(index);
		}
		return std::weak_ptr<Core::GameObject>();
	}

	void Scene::SetCurrentCamera(std::weak_ptr<Render::Camera> camera)
	{
		m_currentCamera = camera;
		m_VP = m_currentCamera.lock()->GetViewProjectionMatrix();
	}

	void Scene::AddObject(std::shared_ptr<Core::GameObject> gameObject)
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

#pragma region Resource Methods
	void Resource::Scene::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;

		Utils::Parser parser(GetFileInfo().GetFullPath());
		m_root->Deserialize(parser);

		p_loaded = true;
		m_root->AfterLoad();
		SendRequest();
	}

	void Scene::Send()
	{
		Initialize();
	}

	void Scene::Save(const std::filesystem::path& fullPath)
	{
		Utils::Serializer serializer(fullPath.empty() ? p_fileInfo.GetFullPath() : fullPath);

		if (!m_root)
			return;
		m_root->Serialize(serializer);
		serializer.CloseFile();
	}

	Weak<Scene> Scene::Create(const std::filesystem::path& path)
	{
		Scene scene(path);
		scene.Save();
		return Resource::ResourceManager::GetInstance()->GetOrLoad<Scene>(path);
	}
#pragma endregion
}
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

#include "Component/MeshComponent.h"

#include "Utils/Parser.h"

#include "Wrapper/Window.h"

#include "Core/Input.h"

using namespace Resource;
namespace GALAXY
{
	Scene::Scene(const std::filesystem::path& path) : IResource(path)
	{
		m_root = std::make_shared<Core::GameObject>(path.filename());
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
				mousePosition.Print();

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
		shared->second->RemoveFromParent();
		if (shared != m_objectList.end())
		{
			m_objectList.erase(shared);
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

		m_root = std::make_shared<Core::GameObject>();
		m_root->m_scene = this;

		Utils::Parser parser(GetFileInfo().GetFullPath());
		m_root->Deserialize(parser);

		p_loaded = true;

		m_root->AfterLoad();
	}

	void Scene::Save()
	{
		Utils::Serializer serializer(GetFileInfo().GetFullPath());

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
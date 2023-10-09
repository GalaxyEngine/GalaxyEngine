#include "pch.h"

#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Core/Application.h"

#include "EditorUI/EditorUIManager.h"

#include "Resource/ResourceManager.h"

#include "Render/Camera.h"
#include "Render/Grid.h"

#include "Component/MeshComponent.h"

#include "Wrapper/Window.h"

#include "Core/Input.h"

using namespace Core;
Scene::Scene()
{
	m_root = std::make_shared<GameObject>("Scene");
	m_editorCamera = std::make_unique<Render::Camera>();

	m_grid = std::make_shared<Render::Grid>();
	m_grid->Initialize();
}

Scene::~Scene()
{
}

void Scene::Update()
{
	const auto renderer = Wrapper::Renderer::GetInstance();

	EditorUI::EditorUIManager::GetInstance()->DrawUI();

	m_root->UpdateSelfAndChild();

	if (m_editorCamera->IsVisible()) {
		SetCurrentCamera(m_editorCamera);

		m_currentCamera.lock()->Begin();
		renderer->ClearColorAndBuffer(m_currentCamera.lock()->GetClearColor());
		m_currentCamera.lock()->SetSize(Core::Application::GetInstance().GetWindow()->GetSize());

		m_editorCamera->Update();
		if (*Core::Application::GetInstance().GetDrawGrid())
			m_grid->Draw();

		if (Input::IsMouseButtonDown(MouseButton::BUTTON_1))
		{
			renderer->RenderingPicking(true);
			m_root->DrawSelfAndChild();
			renderer->RenderingPicking(false);

			auto color = renderer->ReadPixelColor(Input::GetMousePosition());
			std::cout << "Mouse Position ";
			Input::GetMousePosition().Print();
			EditorUI::EditorUIManager::GetInstance()->GetSceneWindow()->GetMousePosition().Print();
			//color.Print();
		}

		//m_root->DrawSelfAndChild();

		renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Right() * 5.f, Vec4f(1, 0, 0, 1));
		renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Up() * 5.f, Vec4f(0, 1, 0, 1));
		renderer->DrawLine(Vec3f::Up(), Vec3f::Up() + Vec3f::Forward() * 5.f, Vec4f(0, 0, 1, 1));
		m_currentCamera.lock()->End();
	}
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

void Scene::SetCurrentCamera(const std::weak_ptr<Render::Camera>& camera)
{
	m_currentCamera = camera;
	m_VP = m_currentCamera.lock()->GetViewProjectionMatrix();
}

#include "pch.h"

#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"

#include "Render/Camera.h"
#include "Render/Grid.h"
#include "Render/Framebuffer.h"
#include "Render/LightManager.h"

#ifdef WITH_EDITOR
#include "Editor/UI/EditorUIManager.h"
#include "Editor/EditorCamera.h"
#include "Editor/Gizmo.h"
#include "Editor/ActionManager.h"
#endif

#include "Component/CameraComponent.h"

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

	void Scene::Initialize()
	{
		p_hasBeenSent = true;
#ifdef WITH_EDITOR
		m_gizmo = std::make_shared<Editor::Gizmo>();

		m_grid = std::make_shared<Render::Grid>();
		m_grid->Initialize();

		m_actionManager = std::make_shared<Editor::ActionManager>();
#endif
		m_lightManager = std::make_shared<Render::LightManager>();
	}

	bool Scene::WasModified() const
	{
		// Compare current scene to last saved
		auto file = std::fstream(p_fileInfo.GetFullPath(), std::ios::in);
		if (!file.is_open()) {
			file.close();
			return true;
		}
		const std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		CppSer::Serializer serializer(p_fileInfo.GetFullPath());
		serializer.SetShouldSaveOnDestroy(false);
		m_root->Serialize(serializer);

		const std::string newContent = serializer.GetContent();

		return content != newContent;
	}

	void Scene::Update()
	{
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		Wrapper::Window* window = Core::Application::GetInstance().GetWindow();
#ifdef WITH_EDITOR
		Editor::UI::Inspector* inspector = Editor::UI::EditorUIManager::GetInstance()->GetInspector();
		Editor::UI::SceneWindow* sceneWindow = Editor::UI::EditorUIManager::GetInstance()->GetSceneWindow();
		Editor::UI::EditorUIManager* editorUIManager = Editor::UI::EditorUIManager::GetInstance();

		editorUIManager->DrawUI();
#endif

		const Vec2i windowSize = Core::Application::GetInstance().GetWindow()->GetSize();

		if (!HasBeenSent()) // if it reload the current scene from the Main Bar menu
			return;

		m_root->UpdateSelfAndChild();

#ifdef WITH_EDITOR
		m_actionManager->Update();

		if (m_editorCamera->IsVisible()) {
			static bool shouldClearOutline = false;
			SetCurrentCamera(m_editorCamera);
			std::shared_ptr<Render::Camera> currentCamera = m_currentCamera.lock();

			const auto& selectedGameObjects = inspector->GetSelectedGameObjects();
			// Outline 
			if (selectedGameObjects.size() > 0 || shouldClearOutline)
			{
				shouldClearOutline = true;
				Render::Framebuffer* outlineFrameBuffer = m_editorCamera->GetOutlineFramebuffer();
				outlineFrameBuffer->Begin(windowSize);

				constexpr Vec4f clearColor = Vec4f(0);
				renderer->ClearColorAndBuffer(clearColor);

				renderer->SetRenderingType(Render::RenderType::Outline);
				for (auto& selected : selectedGameObjects)
				{
					selected.lock()->DrawSelfAndChild(DrawMode::Editor);
				}
				renderer->SetRenderingType(Render::RenderType::Default);
				outlineFrameBuffer->End(windowSize, clearColor);
				if (selectedGameObjects.empty())
				{
					shouldClearOutline = false;
				}
			}

			// Bind Default Framebuffer
			currentCamera->Begin();
			renderer->ClearColorAndBuffer(currentCamera->GetClearColor());
			currentCamera->SetSize(Core::Application::GetInstance().GetWindow()->GetSize());

			m_editorCamera->Update();

			m_gizmo->Update();

			static Vec3f cameraPosition = Vec3f::Zero();
			static Vec3f clickPosition = Vec3f::Zero();
			if (Input::IsMouseButtonPressed(MouseButton::BUTTON_1) && sceneWindow->IsHovered())
			{
				renderer->ClearColorAndBuffer(Vec4f(1));

				renderer->SetRenderingType(Render::RenderType::Picking);
				m_root->DrawSelfAndChild(DrawMode::Editor);
				renderer->SetRenderingType(Render::RenderType::Default);

				// Calculate Mouse Position
				const Vec2f mainWindowSize = window->GetSize();
				const Vec2f imageSize = sceneWindow->GetImageSize();
				Vec2f mousePosition = sceneWindow->GetMousePosition();
				mousePosition = mousePosition * Vec2f(mainWindowSize.x / imageSize.x, mainWindowSize.y / imageSize.y);
				mousePosition.y = mainWindowSize.y - mousePosition.y;

				const Vec4f color = renderer->ReadPixelColor(mousePosition);

				const auto pickedID = static_cast<uint64_t>(color.x + color.y * 256 + color.z * 256 * 256);

				if (m_gizmo->IsGizmoClicked())
				{
				}
				else if (const Shared<Core::GameObject> gameObject = GetWithSceneGraphID(pickedID).lock())
					inspector->SetSelected(gameObject);
				else
					inspector->ClearSelected();

				renderer->ClearColorAndBuffer(currentCamera->GetClearColor());

				const Physic::Ray ray = m_editorCamera->ScreenPointToRay(sceneWindow->GetMousePosition());
				cameraPosition = ray.origin;
				clickPosition = ray.origin + ray.direction * ray.scale;
			}

			m_lightManager->SendLightData();

			//renderer->DrawLine(cameraPosition, clickPosition, Vec4f(0, 1, 0, 1), 4.f);

			if (*Core::Application::GetInstance().GetDrawGridPtr())
				m_grid->Draw();

			m_root->DrawSelfAndChild(DrawMode::Editor);
			m_gizmo->Draw();

			currentCamera->End();
		}
#endif

		size_t index = 0;
		for (auto& camera : m_cameras)
		{
			if (!camera.lock()) {
				// Remove from vector
				m_cameras.erase(m_cameras.begin() + index);
				continue;
			}
			index++;
			SetCurrentCamera(camera);
			std::shared_ptr<Render::Camera> currentCamera = m_currentCamera.lock();
			if (!currentCamera || !currentCamera->IsVisible())
				return;

			// Bind Default Framebuffer
			currentCamera->Begin();
			renderer->ClearColorAndBuffer(currentCamera->GetClearColor());
			currentCamera->SetSize(Core::Application::GetInstance().GetWindow()->GetSize());

			m_lightManager->SendLightData();

			m_root->DrawSelfAndChild(DrawMode::Game);

			currentCamera->End();
		}
	}

	void Scene::SetCurrentCamera(const Weak<Render::Camera>& camera)
	{
		m_currentCamera = camera;
		m_currentCamera.lock()->CreateFrustum();
		m_VP = m_currentCamera.lock()->GetViewProjectionMatrix();
		m_cameraUp = m_currentCamera.lock()->GetTransform()->GetUp();
		m_cameraRight = m_currentCamera.lock()->GetTransform()->GetRight();
	}

#pragma region Resource Methods

	Scene::~Scene()
	{
		Unload();
	}

	void Resource::Scene::Load()
	{
		if (p_shouldBeLoaded)
			return;
		p_shouldBeLoaded = true;
		{
			const Path& path = GetFileInfo().GetFullPath();
			PROFILE_SCOPE_LOG("Scene::Load(%s)", path.generic_string().c_str());
			CppSer::Parser parser(path);
			m_root->m_scene = this;
			m_root->Deserialize(parser);
		}

		p_loaded = true;
		SendRequest();
	}

	void Scene::SetMainCamera(const Weak<Component::CameraComponent>& camera)
	{
		if (m_mainCamera.lock())
			m_mainCamera.lock()->m_isMainCamera = false;
		camera.lock()->m_isMainCamera = true;
		m_mainCamera = camera;
	}

	void Scene::AddCamera(const Weak<Component::CameraComponent>& camera)
	{
		if (m_cameras.empty())
		{
			SetMainCamera(camera);
		}
		m_cameras.push_back(camera);
	}

	void Scene::RemoveCamera(const Component::CameraComponent* camera)
	{
		for (auto it = m_cameras.begin(); it != m_cameras.end(); it++)
		{
			if (it->lock().get() == camera)
			{
				m_cameras.erase(it);
				break;
			}
		}
		if (camera->IsMainCamera() && !m_cameras.empty())
		{
			SetMainCamera(m_cameras[0]);
		}
	}

	void Scene::Unload()
	{
		if (m_root)
		{
			m_root->Destroy();
			m_root.reset();
		}
	}

	void Scene::Send()
	{
		m_root->AfterLoad();
		Initialize();
	}

	void Scene::Save(const Path& fullPath) const
	{
		CppSer::Serializer serializer(fullPath.empty() ? p_fileInfo.GetFullPath() : fullPath);

		if (!m_root)
			return;
		m_root->Serialize(serializer);
		serializer.CloseFile();
	}

	Weak<Scene> Scene::Create(const Path& path)
	{
		const Scene scene(path);
		scene.Save();
		return ResourceManager::GetInstance()->GetOrLoad<Scene>(path);
	}
#pragma endregion
}
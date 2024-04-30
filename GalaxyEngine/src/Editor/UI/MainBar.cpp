#include "pch.h"
#include "Editor/UI/MainBar.h"

#include "Component/BoxCollider.h"
#include "Editor/UI/EditorUIManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Scene.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"

#include "Core/Application.h"
#include "Core/SceneHolder.h"

#include "Component/CameraComponent.h"
#include "Component/DirectionalLight.h"
#include "Component/MeshComponent.h"
#include "Component/PointLight.h"
#include "Component/SpotLight.h"

#include "Utils/OS.h"

namespace GALAXY
{
	void Editor::UI::MainBar::Draw()
	{
		const std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		const EditorUIManager* editorInstance = EditorUIManager::GetInstance();
		EditorSettings& editorSettings = Core::Application::GetInstance().GetEditorSettings();
		Core::ProjectSettings& projectSettings = Core::Application::GetInstance().GetProjectSettings();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Scene"))
				{
					if (const std::string path = Utils::OS::OpenDialog(filters); !path.empty())
					{
						if (std::filesystem::path(path).extension() != ".galaxy")
							return;
						OpenScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene As"))
				{
					if (const std::string path = Utils::OS::SaveDialog(filters); !path.empty())
					{
						SaveScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene"))
				{
					std::string path = Core::SceneHolder::GetCurrentScene()->GetFileInfo().GetFullPath().generic_string();
					if (std::filesystem::exists(path)) {
						SaveScene(path);
					}
					else
					{
						if (path = Utils::OS::SaveDialog(filters); !path.empty())
						{
							SaveScene(path);
						}
					}
				}
				if (ImGui::MenuItem("Exit"))
				{
					Core::Application::GetInstance().Exit();
				}
				ImGui::EndMenu();
			}
			bool openEditorSettings = false;
			bool openProjectSettings = false;
			if (ImGui::BeginMenu("Edit"))
			{
				openEditorSettings = ImGui::MenuItem("Editor Settings");
				openProjectSettings = ImGui::MenuItem("Project Settings");
				ImGui::EndMenu();
			}
			if (openEditorSettings)
			{
				ImGui::OpenPopup("Editor Settings");
			}
			editorSettings.Display();
			if (openProjectSettings)
			{
				ImGui::OpenPopup("Project Settings");
			}
			projectSettings.Display();

			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Hierarchy", nullptr, &editorInstance->GetHierarchy()->p_open);
				ImGui::MenuItem("Inspector", nullptr, &editorInstance->GetInspector()->p_open);
				ImGui::MenuItem("SceneWindow", nullptr, &editorInstance->GetSceneWindow()->p_open);
				ImGui::MenuItem("GameWindow", nullptr, &editorInstance->GetGameWindow()->p_open);
				ImGui::MenuItem("File Explorer", nullptr, &editorInstance->GetFileExplorer()->p_open);
				ImGui::MenuItem("Console", nullptr, &editorInstance->GetConsole()->p_open);
				ImGui::MenuItem("Resources", nullptr, &editorInstance->GetResourceWindow()->p_open);
				ImGui::MenuItem("Debug", nullptr, &editorInstance->GetDebugWindow()->p_open);
				ImGui::EndMenu();
			}
			bool openCreateWithModel = false;
			if (ImGui::BeginMenu("GameObject"))
			{
				DisplayCreateGameObject(openCreateWithModel);
				ImGui::EndMenu();
			}
			UpdateModelPopup(openCreateWithModel);
			
			auto cursorPosX = ImGui::GetWindowContentRegionMax().x * 0.48f;
			ImGui::SetCursorPosX(cursorPosX);
			if (ImGui::MenuItem(Core::Application::IsPlayMode() ? "[  ]" : "|>"))
			{
				auto& appInstance = Core::Application::GetInstance();
				appInstance.SetApplicationMode(Core::Application::IsPlayMode() ? ApplicationMode::Editor : ApplicationMode::Play);
			}
			bool isPauseMode = Core::Application::IsPauseMode();
			if (isPauseMode)
				ImGui::PushStyleColor(ImGuiCol_Text, Vec4f(0.4f, 0.59f, 0.73f, 1.0f));
			if (ImGui::MenuItem("||"))
			{
				if (Core::Application::IsPlayMode() || Core::Application::IsPauseMode())
					Core::Application::GetInstance().SetApplicationMode(isPauseMode ? ApplicationMode::Play : ApplicationMode::Pause);
			}
			if (isPauseMode)
				ImGui::PopStyleColor();
			
			ImGui::EndMainMenuBar();
		}
	}

	void Editor::UI::MainBar::OpenScene(const std::string& path)
	{
		if (Core::SceneHolder::GetCurrentScene() == Resource::ResourceManager::GetResource<Resource::Scene>(path).lock().get())
			return;
		const auto sceneResource = Resource::ResourceManager::ReloadResource<Resource::Scene>(path);

		Core::SceneHolder::GetInstance()->SwitchScene(sceneResource);
	}

	void Editor::UI::MainBar::SaveScene(std::string path)
	{
		if (path.find(".galaxy") == std::string::npos)
			path = path + ".galaxy";

		const Resource::Scene* scene = Core::SceneHolder::GetCurrentScene();
		scene->Save(path);
	}

	void Editor::UI::MainBar::AddModelToScene() const
	{
		if (!m_waitingModel.lock())
			return;
		const auto object = m_waitingModel.lock()->ToGameObject();
		Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
		currentScene->AddObject(object);
		currentScene->GetRootGameObject().lock()->AddChild(object);
	}

	void Editor::UI::MainBar::DisplayCreateGameObject(bool& openModelPopup)
	{
		if (ImGui::MenuItem("With Model"))
		{
			openModelPopup = true;
		}
		if (ImGui::MenuItem("Camera"))
		{
			const auto currentScene = Core::SceneHolder::GetCurrentScene();
			const auto cameraObject = currentScene->CreateObject();
			cameraObject.lock()->AddComponent<Component::CameraComponent>();
			cameraObject.lock()->SetName("Camera");
			currentScene->GetRootGameObject().lock()->AddChild(cameraObject.lock());
		}
		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				const auto currentScene = Core::SceneHolder::GetCurrentScene();
				const auto object = currentScene->CreateObject().lock();
				auto cubeMesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(CUBE_PATH);
				auto meshComp = object->AddComponent<Component::MeshComponent>().lock();
				meshComp->SetMesh(cubeMesh);
				meshComp->AddMaterial(Resource::ResourceManager::GetDefaultMaterial());
				object->AddComponent<Component::BoxCollider>();
				object->SetName("Cube");
				currentScene->GetRootGameObject().lock()->AddChild(object);
			}
			if (ImGui::MenuItem("Sphere"))
			{
				const auto currentScene = Core::SceneHolder::GetCurrentScene();
				const auto object = currentScene->CreateObject().lock();
				auto sphereMesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(SPHERE_PATH);
				auto meshComp = object->AddComponent<Component::MeshComponent>().lock();
				meshComp->SetMesh(sphereMesh);
				meshComp->AddMaterial(Resource::ResourceManager::GetDefaultMaterial());
				object->SetName("Sphere");
				currentScene->GetRootGameObject().lock()->AddChild(object);
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItem("Directional"))
			{
				const auto currentScene = Core::SceneHolder::GetCurrentScene();
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::DirectionalLight>();
				object.lock()->SetName("Directional Light");
				currentScene->GetRootGameObject().lock()->AddChild(object.lock());
			}
			if (ImGui::MenuItem("Point"))
			{
				const auto currentScene = Core::SceneHolder::GetCurrentScene();
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::PointLight>();
				object.lock()->SetName("Point Light");
				currentScene->GetRootGameObject().lock()->AddChild(object.lock());
			}
			if (ImGui::MenuItem("Spot"))
			{
				const auto currentScene = Core::SceneHolder::GetCurrentScene();
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::SpotLight>();
				object.lock()->SetName("Spot Light");
				currentScene->GetRootGameObject().lock()->AddChild(object.lock());
			}
			ImGui::EndMenu();
		}
	}

	bool Editor::UI::MainBar::UpdateModelPopup(bool openModelPopup, Core::GameObject* parent)
	{
		if (openModelPopup)
		{
			ImGui::OpenPopup("Create With Model");
		}
		Weak<Resource::Model> model;
		if (Resource::ResourceManager::GetInstance()->ResourcePopup("Create With Model", model))
		{
			if (const Shared<Resource::Model> modelShared = model.lock())
			{
				auto bind = [modelShared, parent]
				{
					if (!modelShared)
						return;
					const auto object = modelShared->ToGameObject();
					
					Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
					auto parentObject = parent ? parent : currentScene->GetRootGameObject().lock().get();
					
					parentObject->GetScene()->AddObject(object);
					parentObject->AddChild(object);
				};
				if (!modelShared->IsLoaded()) {
					
					modelShared->OnLoad.Bind(bind);
				}
				else
				{
					bind();
				}
				return  true;
			}
		}
		return false;
	}
}


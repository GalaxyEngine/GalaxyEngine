#include "pch.h"
#include "Editor/UI/MainBar.h"
#include "Editor/UI/EditorUIManager.h"
#include "Editor/EditorSettings.h"
#include "Editor/PackageManager.h"

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
#include "Component/BoxCollider.h"
#include "Component/SphereCollider.h"

#include "Utils/OS.h"

namespace GALAXY
{
	WaitingModelMap Editor::UI::MainBar::m_waitingModels;
	void Editor::UI::MainBar::Draw()
	{
		PackageManager& packageManager = EditorSettings::GetInstance().GetPackageManager();
		const std::vector filters = { Utils::OS::Filter("Galaxy", "galaxy") };
		const EditorUIManager* editorInstance = EditorUIManager::GetInstance();
		Core::Application& application = Core::Application::GetInstance();
		EditorSettings& editorSettings = application.GetEditorSettings();
		Core::ProjectSettings& projectSettings = application.GetProjectSettings();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene"))
				{
					Core::SceneHolder::NewScene();
				}
				if (ImGui::MenuItem("Open Scene"))
				{
					if (const std::string path = Utils::OS::OpenDialog(filters, Resource::ResourceManager::GetAssetPath()); !path.empty())
					{
						if (std::filesystem::path(path).extension() != ".galaxy")
							return;
						Core::SceneHolder::OpenScene(path);
					}
				}
				if (ImGui::MenuItem("Save Scene As"))
				{
					if (const std::string path = Utils::OS::SaveDialog(filters, Resource::ResourceManager::GetAssetPath()); !path.empty())
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
						if (path = Utils::OS::SaveDialog(filters, Resource::ResourceManager::GetAssetPath()); !path.empty())
						{
							SaveScene(path);
						}
					}
				}
				if (ImGui::MenuItem("Exit"))
				{
					application.Exit();
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

			bool openPackageManager = false;
			if (ImGui::BeginMenu("Package"))
			{
				if (ImGui::MenuItem("Package Settings"))
				{
					openPackageManager = true;
				}
				ImGui::EndMenu();
			}
			if (openPackageManager)
			{
				packageManager.OpenSettings();
			}
			packageManager.DrawSettings();

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Documentation"); // TODO
				ImGui::MenuItem("About"); // TODO
				ImGui::EndMenu();
			}
			
			
			auto cursorPosX = ImGui::GetWindowContentRegionMax().x * 0.48f;
			ImGui::SetCursorPosX(cursorPosX);
			bool displaySquare = Core::Application::IsPlayMode() || Core::Application::IsPauseMode();
			if (ImGui::MenuItem(displaySquare ? "[  ]" : "|>"))
			{
				application.SetApplicationMode(displaySquare ? ApplicationMode::Editor : ApplicationMode::Play);
			}
			bool isPauseMode = Core::Application::IsPauseMode() || application.ShouldPauseFirstFrame();
			if (isPauseMode)
				ImGui::PushStyleColor(ImGuiCol_Text, Vec4f(0.4f, 0.59f, 0.73f, 1.0f));
			if (ImGui::MenuItem("||"))
			{
				if (Core::Application::IsPlayMode() || Core::Application::IsPauseMode())
					application.SetApplicationMode(isPauseMode ? ApplicationMode::Play : ApplicationMode::Pause);
				else if (Core::Application::IsEditorMode())
					application.SetShouldPauseFirstFrame(true);
			}
			if (isPauseMode)
				ImGui::PopStyleColor();
			if (Core::Application::IsPauseMode())
			{
				ImGui::PopStyleColor();
				if (ImGui::MenuItem(">>"))
				{
					application.SetApplicationMode(ApplicationMode::Play);
					application.MoveOneFrame();
				}
			}
			
			ImGui::EndMainMenuBar();
		}
	}

	void Editor::UI::MainBar::SaveScene(std::string path)
	{
		if (path.find(".galaxy") == std::string::npos)
			path = path + ".galaxy";

		const Resource::Scene* scene = Core::SceneHolder::GetCurrentScene();
		scene->Save(path);
	}

	void Editor::UI::MainBar::DisplayCreateGameObject(bool& openModelPopup, Core::GameObject* parent)
	{
		const auto currentScene = Core::SceneHolder::GetCurrentScene();
		if (!parent)
			parent = currentScene->GetRootGameObject().lock().get(); 
		if (ImGui::MenuItem("Empty"))
		{
			auto emptyObject = currentScene->CreateObject();
			parent->AddChild(emptyObject.lock());
		}
		if (ImGui::MenuItem("With Model"))
		{
			openModelPopup = true;
		}
		if (ImGui::MenuItem("Camera"))
		{
			const auto cameraObject = currentScene->CreateObject();
			cameraObject.lock()->AddComponent<Component::CameraComponent>();
			cameraObject.lock()->SetName("Camera");
			parent->AddChild(cameraObject.lock());
		}
		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				const auto object = currentScene->CreateObject().lock();
				auto cubeMesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(CUBE_PATH);
				auto meshComp = object->AddComponent<Component::MeshComponent>().lock();
				meshComp->SetMesh(cubeMesh);
				meshComp->AddMaterial(Resource::ResourceManager::GetDefaultMaterial());
				object->AddComponent<Component::BoxCollider>();
				object->SetName("Cube");
				parent->AddChild(object);
			}
			if (ImGui::MenuItem("Sphere"))
			{
				const auto object = currentScene->CreateObject().lock();
				auto sphereMesh = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(SPHERE_PATH);
				auto meshComp = object->AddComponent<Component::MeshComponent>().lock();
				meshComp->SetMesh(sphereMesh);
				meshComp->AddMaterial(Resource::ResourceManager::GetDefaultMaterial());
				object->AddComponent<Component::SphereCollider>();
				object->SetName("Sphere");
				parent->AddChild(object);
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItem("Directional"))
			{
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::DirectionalLight>();
				object.lock()->SetName("Directional Light");
				parent->AddChild(object.lock());
			}
			if (ImGui::MenuItem("Point"))
			{
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::PointLight>();
				object.lock()->SetName("Point Light");
				parent->AddChild(object.lock());
			}
			if (ImGui::MenuItem("Spot"))
			{
				const auto object = currentScene->CreateObject();
				object.lock()->AddComponent<Component::SpotLight>();
				object.lock()->SetName("Spot Light");
				parent->AddChild(object.lock());
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
		if (Resource::ResourceManager::ResourcePopup("Create With Model", model))
		{
			if (const Shared<Resource::Model> modelShared = model.lock())
			{
				// do not add the variable inside the [] for smart ptr, it will cause a memory leak
				int index = static_cast<int>(m_waitingModels.size());
				m_waitingModels[index] = std::make_pair(model.lock(), parent);
				std::function bind = [index]()
				{
					Shared<Resource::Model> modelShared = m_waitingModels[index].first;
					Core::GameObject* parent = m_waitingModels[index].second;
					if (!modelShared)
					{
						m_waitingModels.erase(index);
						return;
					}
					const Shared<Core::GameObject> object = modelShared->ToGameObject();
					
					Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
					Core::GameObject* parentObject = parent ? parent : currentScene->GetRootGameObject().lock().get();
					
					parentObject->GetScene()->AddObject(object);
					parentObject->AddChild(object);

					m_waitingModels.erase(index);
				};
				if (!modelShared->IsLoaded())
				{					
					modelShared->EOnLoad.Bind(bind);
				}
				else
				{
					bind();
				}
			}
			return true; 
		}
		return false;
	}
}


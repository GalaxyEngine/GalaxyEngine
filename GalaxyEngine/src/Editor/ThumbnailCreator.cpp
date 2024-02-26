#include "pch.h"
#include "Editor/ThumbnailCreator.h"

#include "Core/GameObject.h"
#include "Core/Application.h"

#include "Component/MeshComponent.h"
#include "Component/CameraComponent.h"
#include "Component/DirectionalLight.h"
#include "Core/SceneHolder.h"
#include "Editor/UI/EditorUIManager.h"
#include "Render/Framebuffer.h"

#include "Render/LightManager.h"

#include "Resource/ResourceManager.h"
#include "Resource/Mesh.h"
#include "Wrapper/ImageLoader.h"

namespace GALAXY
{

	void Editor::ThumbnailCreator::Initialize()
	{
		constexpr Vec4f clearColor(0);

		Path projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();
		auto path = projectPath / THUMBNAIL_PATH;
		std::filesystem::create_directories(path);

		m_scene = std::make_shared<Resource::Scene>("Temp");
		m_scene->Initialize();

		m_cameraObject = std::make_shared<Core::GameObject>();
		m_scene->AddObject(m_cameraObject);
		m_camera = m_cameraObject->AddComponent<Component::CameraComponent>().lock();
		m_camera->SetClearColor(clearColor);

		m_sphereMaterialObject = std::make_shared<Core::GameObject>();
		m_scene->AddObject(m_sphereMaterialObject);

		auto lightObject = std::make_shared<Core::GameObject>();
		m_scene->AddObject(lightObject);
		lightObject->AddComponent<Component::DirectionalLight>();

		auto sphere = Resource::ResourceManager::GetOrLoad<Resource::Mesh>(SPHERE_PATH);

		if (!sphere.lock())
		{
			PrintError("Failed to load sphere mesh for Thumbnail Creator");
			return;
		}

		auto meshComponent = m_sphereMaterialObject->AddComponent<Component::MeshComponent>();

		meshComponent.lock()->SetMesh(sphere);

		m_initialized = true;
	}

	void Editor::ThumbnailCreator::AddToQueue(const Weak<Resource::Material>& material)
	{
		m_thumbnailQueue.push_back(material.lock()->GetFileInfo().GetFullPath());
	}

	void Editor::ThumbnailCreator::CreateMaterialThumbnail(const Weak<Resource::Material>& material)
	{
		auto materialShared = material.lock();
		ASSERT(materialShared != nullptr);

		if (!m_initialized)
		{
			AddToQueue(material);
			return;
		}
		constexpr Vec3f cameraPosition(0, 0, 2);
		static Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		const Shared<Component::MeshComponent> meshComponent = m_sphereMaterialObject->GetWeakComponent<Component::MeshComponent>().lock();

		assert(meshComponent != nullptr);
		const bool canBeCreated = meshComponent->GetMesh().lock()->HasBeenSent() && materialShared->IsLoaded() && materialShared->GetShader()->HasBeenSent();

		if (!canBeCreated)
		{
			AddToQueue(material);
			return;
		}

		Resource::Scene* currentScene = Core::SceneHolder::GetCurrentScene();
		const auto currentCamera = currentScene->GetCurrentCamera();
		constexpr Vec2i frameBufferSize(1024);

		meshComponent->ClearMaterials();
		meshComponent->AddMaterial(material);

		m_cameraObject->GetTransform()->SetLocalPosition(cameraPosition);

		m_camera->SetSize(frameBufferSize);
		currentScene->SetCurrentCamera(m_camera);
		renderer->SetViewport(frameBufferSize);
		m_camera->Begin();

		renderer->ClearColorAndBuffer(m_camera->GetClearColor());

		m_scene->GetLightManager()->SendLightData();

		m_sphereMaterialObject->DrawSelfAndChild(DrawMode::Game);

		m_camera->End();

		// Reset previous data
		currentScene->SetCurrentCamera(currentCamera);
		renderer->SetViewport(Core::Application::GetInstance().GetWindow()->GetSize());

		auto texture = m_camera->GetRenderTexture();

		// Save thumbnail to file 
		const Path thumbnailPath = GetThumbnailPath(materialShared->GetUUID());

		Wrapper::Image imageData;
		imageData.size = frameBufferSize;
		imageData.data = new uint8_t[frameBufferSize.x * frameBufferSize.y * 4];
		Wrapper::Renderer::GetInstance()->BindRenderBuffer(m_camera->GetFramebuffer().get());
		renderer->ReadPixels(imageData.size, imageData.data);

		PrintLog("Save thumbnail to %s", thumbnailPath.generic_string().c_str());
		Wrapper::ImageLoader::SaveImage(thumbnailPath.generic_string().c_str(), imageData);
		delete[] imageData.data;

		// Load render texture
		const auto renderTexture = Resource::ResourceManager::ReloadResource<Resource::Texture>(thumbnailPath).lock();

		UI::EditorUIManager::GetInstance()->GetFileExplorer()->ReloadContent();
	}

	void Editor::ThumbnailCreator::Update()
	{
		if (!m_thumbnailQueue.empty())
		{
			const auto last = m_thumbnailQueue.front();
			m_thumbnailQueue.pop_front();

			const auto resource = Resource::ResourceManager::GetResource<Resource::IResource>(last);
			if (!resource.lock())
				return;
			const Resource::ResourceType resourceType = resource.lock()->GetFileInfo().GetResourceType();
			switch (resourceType)
			{
			case Resource::ResourceType::Material:
			{
				CreateMaterialThumbnail(std::dynamic_pointer_cast<Resource::Material>(resource.lock()));
				break;
			}
			default:
				break;
			}
		}
	}

	std::filesystem::path Editor::ThumbnailCreator::GetThumbnailPath(const Core::UUID& uuid)
	{
		return Resource::ResourceManager::GetInstance()->GetProjectPath() / THUMBNAIL_PATH / (std::to_string(uuid) + ".tmb");
	}

	bool Editor::ThumbnailCreator::IsThumbnailUpToDate(Resource::IResource* resource)
	{
		if (!std::filesystem::exists(GetThumbnailPath(resource->GetUUID())))
			return false;

		const auto thumbnailPath = GetThumbnailPath(resource->GetUUID());
		const auto thumbnailTime = std::filesystem::last_write_time(thumbnailPath);
		const auto resourceTime = std::filesystem::last_write_time(resource->GetFileInfo().GetFullPath());

		return thumbnailTime >= resourceTime;
	}
}

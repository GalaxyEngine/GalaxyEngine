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
		if (!Resource::ResourceManager::DoesProjectExists())
			return;

		constexpr Vec4f clearColor(0);

		Path projectPath = Resource::ResourceManager::GetInstance()->GetProjectPath();
		auto path = projectPath / THUMBNAIL_PATH;
		std::filesystem::create_directories(path);

		m_scene = std::make_shared<Resource::Scene>("Temp");
		m_scene->Initialize();

		m_cameraObject = std::make_shared<Core::GameObject>("Camera");
		m_scene->AddObject(m_cameraObject);
		m_camera = m_cameraObject->AddComponent<Component::CameraComponent>().lock();
		m_camera->SetClearColor(clearColor);

		m_sphereMaterialObject = std::make_shared<Core::GameObject>("Sphere");
		m_scene->AddObject(m_sphereMaterialObject);

		auto lightObject = std::make_shared<Core::GameObject>("Directional");
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

	void Editor::ThumbnailCreator::Release()
	{
		m_sphereMaterialObject.reset();
		m_cameraObject.reset();
		m_camera.reset();
		m_scene.reset();

		m_initialized = false;
	}

	void Editor::ThumbnailCreator::AddToQueue(const Weak<Resource::IResource>& material)
	{
		std::lock_guard lock(Core::ThreadManager::GetMutex());
		std::filesystem::path fullPath = material.lock()->GetFileInfo().GetFullPath();
		for (auto& path : m_thumbnailQueue)
		{
			if (path == fullPath)
				return;
		}

		m_thumbnailQueue.push_back(fullPath);
	}

	void Editor::ThumbnailCreator::CreateModelThumbnail(const Weak<Resource::Model>& model)
	{
		if (!m_initialized)
			return;
		auto modelShared = model.lock();
		ASSERT(modelShared != nullptr);

		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();
		bool canBeCreated = modelShared->HasBeenSent();
		if (canBeCreated) {
			for (auto& material : modelShared->GetMaterials())
			{
				Shared<Resource::Material> materialLock = material.lock();
				if (!materialLock->IsLoaded() || !materialLock->GetShader() || !materialLock->GetShader()->HasBeenSent())
				{
					canBeCreated = false;
					break;
				}
			}
		}

		if (!canBeCreated)
		{
			AddToQueue(model);
			return;
		}

		auto modelObject = modelShared->ToGameObject();
		modelObject->SetScene(m_scene.get());

		float max = FLT_MIN;
		for (int i = 0; i < 3; i++)
		{
			max = std::max(max, modelShared->GetBoundingBox().max[i]);
		}

		Vec3f cameraPosition = Vec3f(-max, max * 1.f, max * 2.5f);
		auto lookAt = Quat::LookRotation((cameraPosition - modelShared->GetBoundingBox().GetCenter()).GetNormalize(), Vec3f(0, -1, 0));
		const Quat cameraRotation = lookAt;

		m_cameraObject->GetTransform()->SetLocalRotation(cameraRotation);
		m_cameraObject->GetTransform()->SetLocalPosition(cameraPosition);
		m_cameraObject->UpdateSelfAndChild();

		m_camera->SetSize(m_thumbnailSize);
		m_scene->SetCurrentCamera(m_camera);
		renderer->SetViewport(m_thumbnailSize);
		m_camera->Begin();

		renderer->ClearColorAndBuffer(m_camera->GetClearColor());

		m_scene->GetLightManager()->SendLightData(Resource::ResourceManager::GetDefaultShader().lock().get(), cameraPosition);

		modelObject->DrawSelfAndChild(DrawMode::Game);

		m_camera->End();

		// Reset previous data
		renderer->SetViewport(Core::Application::GetInstance().GetWindow()->GetSize());

		auto texture = m_camera->GetRenderTexture();

		// Save thumbnail to file 
		const Path thumbnailPath = GetThumbnailPath(modelShared->GetUUID());

		SaveThumbnail(thumbnailPath, m_thumbnailSize);
	}

	void Editor::ThumbnailCreator::CreateMaterialThumbnail(const Weak<Resource::Material>& material)
	{
		if (!m_initialized)
			return;
		auto materialShared = material.lock();
		ASSERT(materialShared != nullptr);
		constexpr Vec3f cameraPosition(0, 0, 2);
		const Quat cameraAngleAxis = Quat::AngleAxis(180, Vec3f(0, 0, 1));
		Wrapper::Renderer* renderer = Wrapper::Renderer::GetInstance();

		const Shared<Component::MeshComponent> meshComponent = m_sphereMaterialObject->GetWeakComponent<Component::MeshComponent>().lock();

		assert(meshComponent != nullptr);
		const bool canBeCreated = meshComponent->GetMesh().lock()->HasBeenSent() && materialShared->IsLoaded() && materialShared->GetShader() && materialShared->GetShader()->HasBeenSent();

		if (!canBeCreated)
		{
			AddToQueue(material);
			return;
		}

		meshComponent->ClearMaterials();
		meshComponent->AddMaterial(material);

		m_cameraObject->GetTransform()->SetLocalPosition(cameraPosition);
		m_cameraObject->GetTransform()->SetLocalRotation(cameraAngleAxis);
		m_cameraObject->UpdateSelfAndChild();

		m_camera->SetSize(m_thumbnailSize);
		m_scene->SetCurrentCamera(m_camera);
		renderer->SetViewport(m_thumbnailSize);
		m_camera->Begin();

		renderer->ClearColorAndBuffer(m_camera->GetClearColor());

		m_scene->GetLightManager()->SendLightData(materialShared->GetShader().get(), cameraPosition);

		m_sphereMaterialObject->DrawSelfAndChild(DrawMode::Game);

		m_camera->End();

		// Reset previous data
		renderer->SetViewport(Core::Application::GetInstance().GetWindow()->GetSize());

		auto texture = m_camera->GetRenderTexture();

		// Save thumbnail to file 
		const Path thumbnailPath = GetThumbnailPath(materialShared->GetUUID());

		SaveThumbnail(thumbnailPath, m_thumbnailSize);
	}

	void Editor::ThumbnailCreator::Update()
	{
		if (!m_thumbnailQueue.empty())
		{
			if (!m_initialized && Resource::ResourceManager::DoesProjectExists())
			{
				Initialize();
			}
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
			case Resource::ResourceType::Model:
			{
				CreateModelThumbnail(std::dynamic_pointer_cast<Resource::Model>(resource.lock()));
				break;
			}
			default:
				break;
			}

			if (m_thumbnailQueue.empty())
			{
				Release();
			}
		}
	}

	std::filesystem::path Editor::ThumbnailCreator::GetThumbnailPath(const Core::UUID& uuid)
	{
		return Resource::ResourceManager::GetInstance()->GetProjectPath() / THUMBNAIL_PATH / (std::to_string(uuid) + ".tmb");
	}

	bool Editor::ThumbnailCreator::IsThumbnailUpToDate(Resource::IResource* resource)
	{
		const std::filesystem::path thumbnailPath = GetThumbnailPath(resource->GetUUID());
		if (!std::filesystem::exists(thumbnailPath))
			return false;

		const auto thumbnailTime = std::filesystem::last_write_time(thumbnailPath);
		const auto resourceTime = std::filesystem::last_write_time(resource->GetFileInfo().GetFullPath());

		return thumbnailTime >= resourceTime;
	}

	void SaveThumb(Wrapper::Image& imageData, const Path& thumbnailPath)
	{
		Wrapper::ImageLoader::SaveImage(thumbnailPath.generic_string().c_str(), imageData);
		delete[] imageData.data;

		// Load render texture
		Resource::ResourceManager::ReloadResource<Resource::Texture>(thumbnailPath);

	}

	void Editor::ThumbnailCreator::SaveThumbnail(const Path& thumbnailPath, const Vec2i& frameBufferSize)
	{
		auto renderer = Wrapper::Renderer::GetInstance();

		Wrapper::Image imageData;
		imageData.size = frameBufferSize;
		imageData.data = new uint8_t[frameBufferSize.x * frameBufferSize.y * 4];

		renderer->BindRenderBuffer(m_camera->GetFramebuffer().get());
		renderer->ReadPixels(imageData.size, imageData.data);

		PrintLog("Save thumbnail to %s", thumbnailPath.generic_string().c_str());
		Core::ThreadManager::GetInstance()->AddTask(&SaveThumb, imageData, thumbnailPath);

		if (m_thumbnailQueue.empty())
			Editor::UI::EditorUIManager::GetInstance()->GetFileExplorer()->ReloadContent();
	}

}

#pragma once
#include "Resource/ResourceManager.h"

#include "Utils/Define.h"

#include "Resource/Texture.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"
#include "Resource/Scene.h"

namespace GALAXY
{
	inline void Resource::ResourceManager::AddResource(const Shared<IResource>& resource)
	{
		if (m_resources.contains(resource->GetFileInfo().GetRelativePath())) {
			PrintWarning("Already Contain %s", resource->GetFileInfo().GetRelativePath().string().c_str());
			return;
		}
		m_resources[resource->GetFileInfo().GetRelativePath()] = resource;
	}

	template<typename T>
	inline Weak<T> Resource::ResourceManager::AddResource(const Path& fullPath)
	{
		Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_resources.contains(relativePath)) {
			PrintWarning("Already Contain %s", relativePath.string().c_str());
			return Weak<T>();
		}
		m_resources[relativePath] = std::make_shared<T>(fullPath);
		return std::dynamic_pointer_cast<T>(m_resources[relativePath]);
	}

	inline void Resource::ResourceManager::RemoveResource(IResource* resource)
	{
		if (!resource)
			return;
		if (auto it = m_resources.find(resource->GetFileInfo().GetRelativePath());  it != m_resources.end())
		{
			it->second->Unload();
			m_resources.erase(it);
		}
		else
		{
			PrintError("Resource %s not found in Resource Manager", resource->GetFileInfo().GetRelativePath().string().c_str());
			return;
		}

	}

	inline void Resource::ResourceManager::RemoveResource(const Shared<IResource>& resource)
	{
		RemoveResource(resource.get());
	}

	inline void Resource::ResourceManager::RemoveResource(const Path& relativePath)
	{
		auto it = m_resources[relativePath];
		m_resources.erase(relativePath);
		it.reset();
	}

	inline bool Resource::ResourceManager::Contains(const Path& fullPath)
	{
		return m_resources.contains(fullPath);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetOrLoad(const Path& fullPath)
	{
		if (fullPath.empty())
			return {};
		Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		auto resource = m_instance->m_resources.find(relativePath);
		if (resource == m_instance->m_resources.end())
		{
			// if resource is not imported
			// if the resource does not exist in path and is not a shader (Shaders are not always a file)
			if (!std::filesystem::exists(fullPath)
				&& T::GetResourceType() != Resource::ResourceType::Shader
				&& T::GetResourceType() != Resource::ResourceType::PostProcessShader
				&& T::GetResourceType() != Resource::ResourceType::Mesh)
				return Weak<T>{};

			m_instance->AddResource<T>(fullPath);
			resource = m_instance->m_resources.find(relativePath);
		}
		if (resource != m_instance->m_resources.end())
		{
			// Load the resource if not loaded.
			if (!resource->second->p_shouldBeLoaded)
			{
#ifdef ENABLE_MULTITHREAD
				Core::ThreadManager::GetInstance()->AddTask(&IResource::Load, resource->second.get());
#else
				resource->second->Load();
#endif // ENABLE_MULTITHREAD

				return std::dynamic_pointer_cast<T>(resource->second);
			}
			else
			{
				return std::dynamic_pointer_cast<T>(resource->second);
			}
		}

		return Weak<T>{};
	}

	template <typename T>
	inline Shared<T> Resource::ResourceManager::TemporaryLoad(const Path& fullPath)
	{
		if (fullPath.empty())
			return {};

		Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);

		if (m_instance->m_resources.contains(relativePath))
		{
			// If is inside resource list, then return the resource from this list
			return GetOrLoad<T>(fullPath).lock();
		}

		Shared<T> resourceShared;
		auto resource = m_instance->m_temporaryResources.find(fullPath);
		if (resource == m_instance->m_temporaryResources.end() || resource->second.expired())
		{
			// if resource is not imported
			resourceShared = std::make_shared<T>(fullPath);
			m_instance->m_temporaryResources[fullPath] = resourceShared;
			resource = m_instance->m_temporaryResources.find(fullPath);
		}
		if (resource != m_instance->m_temporaryResources.end())
		{
			// Load the resource if not loaded.
			if (!resource->second.lock()->p_shouldBeLoaded)
			{
#ifdef ENABLE_MULTITHREAD
				Core::ThreadManager::GetInstance()->AddTask(&IResource::Load, resource->second.lock().get());
#else
				resource->second->Load();
#endif // ENABLE_MULTITHREAD

				return std::dynamic_pointer_cast<T>(resource->second.lock());
			}
			else
			{
				return std::dynamic_pointer_cast<T>(resource->second.lock());
			}
		}
		return {};
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::ReloadResource(const Path& fullPath)
	{
		Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (!m_instance->m_resources.count(relativePath)) {
			PrintWarning("Resource %s not found in Resource Manager, Create it", relativePath.string().c_str());
			return GetOrLoad<T>(fullPath);
		}

		Shared<IResource> resource = m_instance->m_resources.at(relativePath);
		T* resourcePtr = dynamic_pointer_cast<T>(resource).get();
		*resourcePtr = T(fullPath);

		return GetOrLoad<T>(fullPath);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetResource(const Path& fullPath)
	{
		auto relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_resources.count(relativePath))
		{
			return std::dynamic_pointer_cast<T>(m_resources.at(relativePath));
		}
		return Weak<T>{};
	}

	template <typename T>
	inline Shared<T> Resource::ResourceManager::GetTemporaryResource(const Path& fullPath)
	{
		if (m_temporaryResources.count(fullPath))
		{
			auto resource = m_temporaryResources.at(fullPath);
			if (resource.expired()) 
			{
				m_temporaryResources.erase(fullPath);
				return nullptr;
			}
				
			return std::dynamic_pointer_cast<T>(resource.lock());
		}
		return nullptr;
	}

	template <typename T>
	inline std::vector<Weak<T>> Resource::ResourceManager::GetAllResources()
	{
		std::vector<Weak<T>> m_resourcesOfType;
		for (auto&& resource : m_resources)
		{
			if (resource.second->GetFileInfo().GetResourceType() == T::GetResourceType())
			{
				m_resourcesOfType.push_back(std::dynamic_pointer_cast<T>(resource.second));
			}
		}
		return m_resourcesOfType;
	}

	template <typename T>
	[[nodiscard]] inline Weak<T> Resource::ResourceManager::ResourcePopup(const char* popupName, const std::vector<Resource::ResourceType>& typeFilter /* = {}*/)
	{
		if (ImGui::BeginPopup(popupName))
		{
			static ImGuiTextFilter filter;
			filter.Draw();
			Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
			size_t i = 0;
			for (const auto& [path, resource] : m_resources)
			{
				bool typeChecked = false;
				if (typeFilter.size() > 0)
					typeChecked = std::find(typeFilter.begin(), typeFilter.end(), resource->GetFileInfo().GetResourceType()) != typeFilter.end();
				else
					typeChecked = resource->GetFileInfo().GetResourceType() == T::GetResourceType();

				if (typeChecked && filter.PassFilter(resource->GetFileInfo().GetFileNameNoExtension().c_str())
					&& resource->p_displayOnInspector)
				{
					ImGui::PushID((int)i++);
					if (ImGui::Button(resource->GetFileInfo().GetFileNameNoExtension().c_str(), buttonSize))
					{
						ImGui::CloseCurrentPopup();
						return GetOrLoad<T>(path);
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(resource->GetFileInfo().GetRelativePath().string().c_str());
					}
					ImGui::PopID();
				}
			}
			ImGui::EndPopup();
		}
		return Weak<T>();
	}

	inline Weak<Resource::Shader> Resource::ResourceManager::GetUnlitShader()
	{
		if (!m_unlitShader.lock())
			m_unlitShader = GetOrLoad<Resource::Shader>(ENGINE_RESOURCE_FOLDER_NAME"/shaders/UnlitShader/unlit.shader");
		return m_unlitShader;
	}

	inline Weak<Resource::Shader> Resource::ResourceManager::GetLitShader()
	{
		if (!m_litShader.lock())
			m_litShader = GetOrLoad<Resource::Shader>(ENGINE_RESOURCE_FOLDER_NAME"/shaders/LitShader/lit.shader");
		return m_litShader;
	}

	inline Weak<Resource::Material> Resource::ResourceManager::GetDefaultMaterial()
	{
		if (!m_defaultMaterial.lock())
			m_defaultMaterial = GetOrLoad<Resource::Material>(ENGINE_RESOURCE_FOLDER_NAME"/materials/DefaultMaterial.mat");
		return m_defaultMaterial;
	}

	inline Resource::ResourceManager* Resource::ResourceManager::GetInstance()
	{
		if (m_instance == nullptr) {
			m_instance = std::make_unique<ResourceManager>();
		}
		return m_instance.get();
	}

	inline Weak<Resource::Shader> Resource::ResourceManager::GetDefaultShader()
	{
		if (!m_defaultShader.lock())
			m_defaultShader = GetLitShader();
		return m_defaultShader;
	}
}

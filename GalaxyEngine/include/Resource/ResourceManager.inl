#pragma once
#include "Resource/ResourceManager.h"

#include "Utils/Define.h"

#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Scene.h"

namespace GALAXY
{
	inline void Resource::ResourceManager::AddResource(const Shared<IResource>& resource)
	{
		if (m_instance->m_resources.contains(resource->GetFileInfo().GetRelativePath())) {
			PrintWarning("Already Contain %s", resource->GetFileInfo().GetRelativePath().string().c_str());
			return;
		}
		m_instance->m_resources[resource->GetFileInfo().GetRelativePath()] = resource;
	}

	template<typename T>
	inline Weak<T> Resource::ResourceManager::AddResource(const Path& fullPath)
	{
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_instance->m_resources.contains(relativePath)) {
			PrintWarning("Already Contain %s", relativePath.string().c_str());
			return std::dynamic_pointer_cast<T>(m_instance->m_resources[relativePath]);
		}
		auto resource = std::make_shared<T>(fullPath);
		resource->ParseDataFile();
		m_instance->m_resources[relativePath] = resource;
		resource->OnAdd();

		return std::dynamic_pointer_cast<T>(m_instance->m_resources[relativePath]);
	}

	inline void Resource::ResourceManager::RemoveResource(IResource* resource)
	{
		if (!resource)
			return;
		if (const auto it = m_resources.find(resource->GetFileInfo().GetRelativePath());  it != m_resources.end())
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

	inline bool Resource::ResourceManager::Contains(const Path& fullPath) const
	{
		return m_resources.contains(fullPath);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetOrLoad(const Path& fullPath)
	{
		if (fullPath.empty())
			return {};
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
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

			AddResource<T>(fullPath);
			resource = m_instance->m_resources.find(relativePath);
		}
		if (resource != m_instance->m_resources.end())
		{
			// Load the resource if not loaded.
			if (!resource->second->p_shouldBeLoaded.load())
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
	Weak<T> Resource::ResourceManager::GetOrLoad(const Core::UUID& uuid)
	{
		if (uuid == -1)
			return {};

		auto resource = std::find_if(m_instance->m_resources.begin(), m_instance->m_resources.end(), [&](const std::pair<Path, Shared<IResource>>& resource)
			{
				return resource.second->GetUUID() == uuid;
			});
		if (resource == m_instance->m_resources.end())
		{
			PrintWarning("Resource with UUID %llu not found", uuid);
			// Not found
			return {};
		}

		return GetOrLoad<T>(resource->first);
	}

	template <typename T>
	inline Shared<T> Resource::ResourceManager::TemporaryLoad(const Path& fullPath)
	{
		if (fullPath.empty())
			return {};

		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);

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
				resource->second.lock()->Load();
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
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (!m_instance->m_resources.contains(relativePath)) {
			//PrintWarning("Resource %s not found in Resource Manager, Create it", relativePath.string().c_str());
			return GetOrLoad<T>(fullPath);
		}

		const Shared<IResource> resource = m_instance->m_resources.at(relativePath);
		T* resourcePtr = dynamic_pointer_cast<T>(resource).get();
		*resourcePtr = T(fullPath);

		return GetOrLoad<T>(fullPath);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetResource(const Path& fullPath)
	{
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_instance->m_resources.contains(relativePath))
		{
			return std::dynamic_pointer_cast<T>(m_instance->m_resources.at(relativePath));
		}
		return {};
	}

	template <typename T>
	Weak<T> Resource::ResourceManager::GetResource(const Core::UUID& uuid)
	{
		if (uuid == INDEX_NONE)
			return {};
		auto resource = std::find_if(m_instance->m_resources.begin(), m_instance->m_resources.end(), [&](const std::pair<Path, Shared<IResource>>& resource)
			{
				return resource.second->GetUUID() == uuid;
			});
		if (resource == m_instance->m_resources.end())
		{
			// Not found
			return {};
		}

		return GetOrLoad<T>(resource->first);
	}

	template<typename T>
	inline Shared<T> Resource::ResourceManager::TemporaryAdd(const Path& fullPath)
	{
		if (m_instance->m_temporaryResources.contains(fullPath)) {
			PrintWarning("Already Contain %s", fullPath.string().c_str());
			return std::dynamic_pointer_cast<T>(m_instance->m_temporaryResources[fullPath].lock());
		}
		auto resource = std::make_shared<T>(fullPath);
		m_instance->m_temporaryResources[fullPath] = resource;
		return resource;
	}

	template <typename T>
	inline Shared<T> Resource::ResourceManager::GetTemporaryResource(const Path& fullPath)
	{
		if (m_temporaryResources.contains(fullPath))
		{
			const Weak<IResource> resource = m_temporaryResources.at(fullPath);
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
		for (Shared<IResource>& val : m_resources | std::views::values)
		{
			if (val->GetFileInfo().GetResourceType() == T::GetResourceType())
			{
				m_resourcesOfType.push_back(std::dynamic_pointer_cast<T>(val));
			}
		}
		return m_resourcesOfType;
	}

	template <typename T>
	inline bool Resource::ResourceManager::ResourcePopup(const char* popupName, Weak<T>& outResource, const std::vector<Resource::ResourceType>& typeFilter /* = {}*/)
	{
		bool result = false;
		if (ImGui::BeginPopup(popupName))
		{
			const Vec2f buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
			ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_RED);
			if (ImGui::Button("Reset", buttonSize)) {
				outResource.reset();
				result = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.f);
			static ImGuiTextFilter filter;
			filter.Draw();
			size_t i = 0;
			const bool checkTypeInRange = typeFilter.size() > 0;
			for (const auto& [path, resource] : m_resources)
			{
				bool typeChecked;
				if (checkTypeInRange)
					typeChecked = std::ranges::find(typeFilter, resource->GetFileInfo().GetResourceType()) != typeFilter.end();
				else {
					typeChecked = dynamic_pointer_cast<T>(resource) != nullptr;
				}

				if (typeChecked && filter.PassFilter(resource->GetFileInfo().GetFileNameNoExtension().c_str())
					&& resource->ShouldDisplayOnInspector())
				{
					ImGui::PushID(static_cast<int>(i++));
					if (ImGui::Button(resource->GetFileInfo().GetFileNameNoExtension().c_str(), buttonSize))
					{
						result = true;
						outResource = GetOrLoad<T>(path);
						ImGui::CloseCurrentPopup();
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
		return result;
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

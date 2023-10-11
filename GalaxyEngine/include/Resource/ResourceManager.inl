#pragma once
#include "Resource/ResourceManager.h"

#include "Resource/Texture.h"
#include "Resource/PostProcessShader.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"

namespace GALAXY
{
	void Resource::ResourceManager::AddResource(IResource* resource)
	{
		if (m_resources.contains(resource->GetFileInfo().GetRelativePath())) {
			PrintWarning("Already Contain %s", resource->GetFileInfo().GetRelativePath().string().c_str());
			delete resource;
			return;
		}
		m_resources[resource->GetFileInfo().GetRelativePath()] = std::shared_ptr<IResource>(resource);
	}

	void Resource::ResourceManager::AddResource(std::shared_ptr<IResource> resource)
	{
		if (m_resources.contains(resource->GetFileInfo().GetRelativePath())) {
			PrintWarning("Already Contain %s", resource->GetFileInfo().GetRelativePath().string().c_str());
			return;
		}
		m_resources[resource->GetFileInfo().GetRelativePath()] = resource;
	}

	void Resource::ResourceManager::RemoveResource(IResource* resource)
	{
		/*if (auto it = m_resources.find(resource->GetFileInfo().GetRelativePath());  it != m_resources.end())
		{
			it->second->Unload();
		}
		else
		{
			PrintError("Resource %s not found in Resource Manager", resource->GetFileInfo().GetRelativePath().string().c_str());
			return;
		}*/

	}

	void Resource::ResourceManager::RemoveResource(const std::shared_ptr<IResource>& resource)
	{
		RemoveResource(resource.get());
	}

	void Resource::ResourceManager::RemoveResource(const std::filesystem::path& relativePath)
	{
		auto it = m_resources[relativePath];
		m_resources.erase(relativePath);
		it.reset();
	}

	bool Resource::ResourceManager::Contains(const std::filesystem::path& fullPath)
	{
		return m_resources.contains(fullPath);
	}

	template <typename T>
	inline std::weak_ptr<T> Resource::ResourceManager::GetOrLoad(const std::filesystem::path& fullPath)
	{
		std::filesystem::path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		auto resource = m_instance->m_resources.find(relativePath);
		if (resource == m_instance->m_resources.end())
		{
			// if resource is not imported
			
			// if the resource does not exist in path and is not a shader (Shaders are not always a file)
			if (!std::filesystem::exists(fullPath) 
				&& T::GetResourceType() != Resource::ResourceType::Shader 
				&& T::GetResourceType() != Resource::ResourceType::PostProcessShader)
				return std::weak_ptr<T>{};

			m_instance->AddResource(new T(fullPath));
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

		return std::weak_ptr<T>{};
	}

	template <typename T>
	inline std::weak_ptr<T> Resource::ResourceManager::GetResource(const std::filesystem::path& fullPath)
	{
		auto relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_resources.count(relativePath))
		{
			return std::dynamic_pointer_cast<T>(m_resources.at(relativePath));
		}
		return std::weak_ptr<T>{};
	}

	template <typename T>
	inline std::vector<std::weak_ptr<T>> Resource::ResourceManager::GetAllResources()
	{
		std::vector<std::weak_ptr<T>> m_resourcesOfType;
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
	[[nodiscard]] inline std::weak_ptr<T> Resource::ResourceManager::ResourcePopup(const char* popupName, const std::vector<Resource::ResourceType>& typeFilter /* = {}*/)
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
					
				if (typeChecked	&& filter.PassFilter(resource->GetFileInfo().GetFileNameNoExtension().c_str()) 
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
		return std::weak_ptr<T>();
	}

	std::weak_ptr<Resource::Shader> Resource::ResourceManager::GetUnlitShader()
	{
		std::string unlitPath = ENGINE_RESOURCE_FOLDER_NAME"\\shaders\\UnlitShader\\unlit.shader";
		return GetOrLoad<Resource::Shader>(unlitPath);
	}

	std::weak_ptr<Resource::Material> Resource::ResourceManager::GetDefaultMaterial()
	{
		if (!m_defaultMaterial.lock())
			m_defaultMaterial = GetOrLoad<Resource::Material>(ENGINE_RESOURCE_FOLDER_NAME"\\materials\\DefaultMaterial.mat");
		return m_defaultMaterial;
	}

	std::weak_ptr<Resource::Shader> Resource::ResourceManager::GetDefaultShader()
	{
		if (!m_defaultShader.lock())
			m_defaultShader = GetUnlitShader();
		return m_defaultShader;
	}
}

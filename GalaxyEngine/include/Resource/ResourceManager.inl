#pragma once
#include "Resource/ResourceManager.h"

#include "Utils/Define.h"

#include "Resource/Texture.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Scene.h"
#include "Resource/Prefab.h"
#include "Resource/Sound.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <Wrapper/GUI.h>

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
		if (const auto it = m_instance->m_resources.find(resource->GetFileInfo().GetRelativePath());  it != m_instance->m_resources.end())
		{
			it->second->Unload();
			m_instance->m_resources.erase(it);
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

	inline void Resource::ResourceManager::RemoveResource(const Path& fullPath)
	{
		if (!m_instance)
			return;
		auto relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (!m_instance->m_resources.contains(relativePath))
			return;

		auto it = m_instance->m_resources[relativePath];
		it->Unload();
		m_instance->m_resources.erase(relativePath);
		it.reset();
	}

	inline bool Resource::ResourceManager::Contains(const Path& fullPath) const
	{
		return m_resources.contains(fullPath);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetOrLoad(const Path& fullPath, bool async /*= true*/)
	{
#ifndef ENABLE_MULTI_THREAD
		async = false;
#endif
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
			if (!resource->second)
			{
				PrintError("Resource %s not found in Resource Manager", relativePath.string().c_str());
				return Weak<T>{};
			}
			// Load the resource if not loaded.
			if (!resource->second->p_shouldBeLoaded.load())
			{
				if (async)
				{
					Core::ThreadManager::GetInstance()->AddTask(&IResource::Load, resource->second.get());
				}
				else
				{
					resource->second->Load();
				}
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
		if (uuid == UUID_NULL)
			return {};

		auto resource = std::find_if(m_instance->m_resources.begin(), m_instance->m_resources.end(), [&](const std::pair<Path, Shared<IResource>>& _resource)
			{
				return _resource.second->GetUUID() == uuid;
			});
		if (resource == m_instance->m_resources.end())
		{
			// Not found
			PrintWarning("Resource with UUID %llu not found", uuid);
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
#ifdef ENABLE_MULTI_THREAD
				Core::ThreadManager::GetInstance()->AddTask(&IResource::Load, resource->second.lock().get());
#else
				resource->second.lock()->Load();
#endif // ENABLE_MULTI_THREAD

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
	inline Weak<T> Resource::ResourceManager::ReloadResource(const Path& fullPath, bool async /*= true*/)
	{
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (!m_instance->m_resources.contains(relativePath)) {
			//PrintWarning("Resource %s not found in Resource Manager, Create it", relativePath.string().c_str());
			return GetOrLoad<T>(fullPath, async);
		}

		const Shared<IResource> resource = m_instance->m_resources.at(relativePath);
		T* resourcePtr = static_pointer_cast<T>(resource).get();
		*resourcePtr = T(fullPath);

		return GetOrLoad<T>(fullPath, async);
	}

	template<typename T>
	inline Weak<T> Resource::ResourceManager::ReloadResource(const Core::UUID& uuid)
	{
		if (uuid == UUID_NULL)
			return {};

		auto resource = std::find_if(m_instance->m_resources.begin(), m_instance->m_resources.end(), [&](const std::pair<Path, Shared<IResource>>& _resource)
			{
				return _resource.second->GetUUID() == uuid;
			});
		if (resource == m_instance->m_resources.end())
		{
			// Not found
			PrintWarning("Resource with UUID %llu not found", uuid);
			return {};
		}

		return ReloadResource<T>(resource->first);
	}

	template <typename T>
	inline Weak<T> Resource::ResourceManager::GetResource(const Path& fullPath)
	{
		const Path relativePath = Utils::FileInfo::ToRelativePath(fullPath);
		if (m_instance->m_resources.contains(relativePath))
		{
			return std::static_pointer_cast<T>(m_instance->m_resources.at(relativePath));
		}
		return {};
	}

	template <typename T>
	Weak<T> Resource::ResourceManager::GetResource(const Core::UUID& uuid)
	{
		if (uuid == UUID_NULL)
			return {};
		auto resource = std::find_if(m_instance->m_resources.begin(), m_instance->m_resources.end(), [&](const std::pair<Path, Shared<IResource>>& _resource)
			{
				return _resource.second->GetUUID() == uuid;
			});
		if (resource == m_instance->m_resources.end())
		{
			// Not found
			return {};
		}

		return std::static_pointer_cast<T>(resource->second);
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
	inline bool Resource::ResourceManager::ResourcePopup(const char* popupName, Weak<T>& outResource)
	{
		bool result = false;
		if (ImGui::BeginPopup(popupName))
		{
			constexpr int maxButtonDisplay = 5;
			const float regionAvailX = ImGui::GetContentRegionAvail().x;
			Vec2f buttonSize = Vec2f(regionAvailX, 0);
			const Vec2f imageSize = Vec2f(64, 64) * Wrapper::GUI::GetScaleFactor();
			const Vec2f selectableSize(buttonSize.x, imageSize.y);
			if (ImGui::Button("Cancel", buttonSize)) {
				result = true;
				ImGui::CloseCurrentPopup();
			}
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
			ImGui::BeginChild("ResourceList", Vec2f(regionAvailX, maxButtonDisplay * imageSize.y), true);
			buttonSize = Vec2f(ImGui::GetContentRegionAvail().x, 0);
			size_t i = 0;
			bool isAMesh = std::is_base_of<Resource::Mesh, T>::value;
			for (const auto& [path, resource] : m_instance->m_resources)
			{
				bool typeChecked = dynamic_pointer_cast<T>(resource) != nullptr;

				std::string name = resource->GetFileInfo().GetFileNameNoExtension();

				if (isAMesh)
				{
					// if is a mesh we want to display the name of the mesh not the name of the model
					name = resource->GetFileInfo().GetFileName();
					name = name.substr(name.find(':') + 1);
				}
				
				if (typeChecked && filter.PassFilter(name.c_str())
					&& resource->ShouldDisplayOnInspector())
				{
					std::string resourceName = SerializeResourceTypeValue(T::GetResourceType());
					ImGui::PushID(static_cast<int>(i++));
					auto cursorPos = ImGui::GetCursorPos();
					if (ImGui::Selectable("##", false, 0, selectableSize))
					{
						result = true;
						outResource = GetOrLoad<T>(path);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(resource->GetFileInfo().GetRelativePath().string().c_str());
					}
					ImGui::SetCursorPos(cursorPos);
					auto thumbnail = GetOrLoad<Texture>(resource->GetThumbnailPath()).lock();
					auto id = thumbnail ? thumbnail->GetID() : 0;
					ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(id)), imageSize);
					ImGui::SameLine();
					ImGui::BeginGroup();
					ImGui::TextUnformatted((resourceName + " | " + name).c_str());
					ImGui::EndGroup();
					/*
					if (ImGui::Button(name.c_str(), buttonSize))
					{
						result = true;
						outResource = GetOrLoad<T>(path);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(resource->GetFileInfo().GetRelativePath().string().c_str());
					}
					*/
					ImGui::PopID();
				}
			}
			ImGui::EndChild();
			ImGui::EndPopup();
		}
		return result;
	}

	template <typename T>
	inline bool Resource::ResourceManager::ResourceField(Weak<T>& outResource, const std::string& fieldName, bool* selected)
	{
		// TODO ? Make a struct to check for rightclick
		ImGui::PushID(fieldName.c_str());
		const std::string resourceName = SerializeResourceTypeValue(T::GetResourceType());
		const std::string label = outResource.lock() ? outResource.lock()->GetName() : "None";

		const Vec2f imageSize = Vec2f(64, 64) * Wrapper::GUI::GetScaleFactor();
		Resource::Texture* texture = nullptr;
		uint32_t id = 0;
		if (auto resource = outResource.lock())
		{
			texture = GetOrLoad<Resource::Texture>(outResource.lock()->GetThumbnailPath()).lock().get();
			if (texture)
				id = texture->GetID();
		}
		auto prevPos = ImGui::GetCursorPos();
		bool result = false;
		if (selected)
			result = *selected;
		if (ImGui::Selectable("##", result, ImGuiSelectableFlags_AllowOverlap, ImVec2(ImGui::GetContentRegionAvail().x, imageSize.y)) && selected)
			*selected = !result;
		if (outResource.lock() && ImGui::IsItemHovered())
			ImGui::SetTooltip(std::to_string(outResource.lock()->GetUUID()).c_str());
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
			{
				auto draggedFiles = GetExplorerDraggedFile();
				if (auto resource = draggedFiles.lock())
				{
					if (auto castResource = std::dynamic_pointer_cast<T>(resource))
					{
						if (!castResource->IsLoaded())
							GetOrLoad<T>(castResource->GetFileInfo().GetFullPath());
						outResource = castResource;
					}
				}
			}
		}
		ImGui::SetCursorPos(prevPos);
		ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(id)), imageSize);
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::TextUnformatted((resourceName + " | " + fieldName).c_str());
		if (ImGui::Button(label.c_str(), Vec2f(ImGui::GetContentRegionAvail().x, 0)))
		{
			ImGui::OpenPopup(fieldName.c_str());
		}
		if (outResource.lock())
		{
			if (ImGui::Button("Find"))
			{
				auto path = outResource.lock()->GetFileInfo().GetFullPath();
				ShowFileInInternExplorer(path);
			}
		}
		ImGui::EndGroup();
		if (ResourceManager::ResourcePopup(fieldName.c_str(), outResource))
		{
			ImGui::PopID();
			return true;
		}
		ImGui::PopID();
		return false;
	}

	inline Weak<Resource::Shader> Resource::ResourceManager::GetUnlitShader()
	{
		if (!m_instance->m_unlitShader.lock())
			m_instance->m_unlitShader = GetOrLoad<Resource::Shader>(ENGINE_RESOURCE_FOLDER_NAME"/shaders/UnlitShader/unlit.shader");
		return m_instance->m_unlitShader;
	}

	inline Weak<Resource::Shader> Resource::ResourceManager::GetLitShader()
	{
		if (!m_instance->m_litShader.lock())
			m_instance->m_litShader = GetOrLoad<Resource::Shader>(ENGINE_RESOURCE_FOLDER_NAME"/shaders/LitShader/lit.shader");
		return m_instance->m_litShader;
	}

	inline Weak<Resource::Material> Resource::ResourceManager::GetDefaultMaterial()
	{
		if (!m_instance->m_defaultMaterial.lock())
			m_instance->m_defaultMaterial = GetOrLoad<Resource::Material>(ENGINE_RESOURCE_FOLDER_NAME"/materials/DefaultMaterial.mat");
		return m_instance->m_defaultMaterial;
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
		if (!m_instance->m_defaultShader.lock())
			m_instance->m_defaultShader = GetLitShader();
		return m_instance->m_defaultShader;
	}
}

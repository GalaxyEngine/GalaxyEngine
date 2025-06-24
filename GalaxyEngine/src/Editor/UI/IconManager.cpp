#include "pch.h"
#include "Editor/UI/IconManager.h"

#include "Resource/ResourceManager.h"

namespace GALAXY 
{
    using namespace Editor::UI;

    ImTextureID IconManager::InfoIcon = nullptr;
    ImTextureID IconManager::WarningIcon = nullptr;
    ImTextureID IconManager::ErrorIcon = nullptr;
    ImTextureID IconManager::SettingsIcon = nullptr;
    ImTextureID IconManager::MenuIcon = nullptr;
    ImTextureID IconManager::AddIcon = nullptr;
    ImTextureID IconManager::RemoveIcon = nullptr;
    ImTextureID IconManager::SearchIcon = nullptr;
    ImTextureID IconManager::CameraComponentIcon = nullptr;
    ImTextureID IconManager::LightComponentIcon = nullptr;
    ImTextureID IconManager::TransformComponentIcon = nullptr;
    ImTextureID IconManager::MeshComponentIcon = nullptr;
    ImTextureID IconManager::ScriptComponentIcon = nullptr;
    
    void IconManager::Initialize()
    {
        auto loadIcon = [](ImTextureID* texture, const Path& fullPath) {
            std::shared_ptr<Resource::Texture> icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(fullPath).lock();
            if (!icon)
                return;
            if (icon->HasBeenSent())
            {
                *texture = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(icon->GetID()));
            }
            else
            {
                icon->EOnLoad.Bind([texture, icon]() {
                    *texture = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(icon->GetID()));
                });
            }
        };
        
        loadIcon(&InfoIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/info.png");
        loadIcon(&WarningIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/warning.png");
        loadIcon(&ErrorIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/error.png");
        loadIcon(&SettingsIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/settings.png");
        loadIcon(&MenuIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/menu.png");
        loadIcon(&AddIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/add.png");
        loadIcon(&RemoveIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/remove.png");
        loadIcon(&SearchIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/search.png");
        loadIcon(&TransformComponentIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/component/transform.png");
        loadIcon(&MeshComponentIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/component/mesh.png");
        loadIcon(&CameraComponentIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/component/camera.png");
        loadIcon(&LightComponentIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/component/light.png");
        loadIcon(&ScriptComponentIcon, ENGINE_RESOURCE_FOLDER_NAME "/icons/component/script.png");
    }
}

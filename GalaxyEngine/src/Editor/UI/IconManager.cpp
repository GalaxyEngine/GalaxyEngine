#include "pch.h"
#include "Editor/UI/IconManager.h"

#include "Resource/ResourceManager.h"

namespace GALAXY 
{
    using namespace Editor::UI;

    uint32_t IconManager::InfoIcon = -1;
    uint32_t IconManager::WarningIcon = -1;
    uint32_t IconManager::ErrorIcon = -1;
    uint32_t IconManager::SettingsIcon = -1;
    uint32_t IconManager::MenuIcon = -1;
    uint32_t IconManager::AddIcon = -1;
    uint32_t IconManager::RemoveIcon = -1;
    uint32_t IconManager::SearchIcon = -1;
    uint32_t IconManager::CameraComponentIcon = -1;
    uint32_t IconManager::LightComponentIcon = -1;
    uint32_t IconManager::TransformComponentIcon = -1;
    uint32_t IconManager::MeshComponentIcon = -1;
    uint32_t IconManager::ScriptComponentIcon = -1;
    
    void IconManager::Initialize()
    {
        auto loadIcon = [](uint32_t* texture, const Path& fullPath) {
            std::shared_ptr<Resource::Texture> icon = Resource::ResourceManager::GetOrLoad<Resource::Texture>(fullPath).lock();
            if (!icon)
                return;
            if (icon->HasBeenSent())
            {
                *texture = icon->GetID();
            }
            else
            {
                icon->EOnLoad.Bind([texture, icon]() {
                    *texture = icon->GetID();
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

#include "pch.h"
#include "Resource/Prefab.h"

#include "Core/SceneHolder.h"
#include "Resource/ResourceManager.h"

namespace GALAXY
{
    void Resource::Prefab::Load()
    {
        if (p_shouldBeLoaded)
            return;
        
        Scene::Load();

        for (const auto& pair : m_waitingObjects)
        {
            InstantiateInternal(pair.second, pair.first);
        }
        m_waitingObjects.clear();
    }

    void Resource::Prefab::Send()
    {
        Scene::Send();
    }

    void Resource::Prefab::Save(const Path& fullPath) const
    {
        Scene::Save(fullPath);
    }

    Weak<Core::GameObject> Resource::Prefab::Instantiate(Weak<Core::GameObject> parent)
    {
        auto object = std::make_shared<Core::GameObject>(p_fileInfo.GetFileNameNoExtension());
        if (!p_loaded)
        {
            auto pair = std::make_pair(object, parent);
            m_waitingObjects.push_back(pair);
        }
        else
        {
            InstantiateInternal(parent, object);
        }

        return object;
    }

    void Resource::Prefab::CreateWith(const Path& fullPath, const std::shared_ptr<Core::GameObject>& gameObject)
    {
        auto prefab = Resource::ResourceManager::AddResource<Resource::Prefab>(fullPath).lock();
        prefab->m_root = gameObject->Clone();

        prefab->Save(fullPath);
    }

    void DisplayGameObject(Shared<Core::GameObject> gameObject)
    {
        if (ImGui::TreeNode(gameObject->GetName().c_str()))
        {
            for (auto& child : gameObject->GetChildren())
            {
                DisplayGameObject(child.lock());
            }
        }
    }

    void Resource::Prefab::ShowInInspector()
    {
#ifdef WITH_EDITOR
        DisplayGameObject(m_root);
#endif
    }

    void Resource::Prefab::InstantiateInternal(Weak<Core::GameObject> parent, Shared<Core::GameObject> gameObject)
    {
        gameObject = m_root->Clone();
        if (parent.lock())
            gameObject->SetParent(parent);
        else
            gameObject->SetParent(Core::SceneHolder::GetCurrentScene()->GetRootGameObject());
    }
}

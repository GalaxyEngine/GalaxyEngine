#pragma once
#include "GalaxyAPI.h"
#include "Scene.h"
#include "Resource/IResource.h"

#include <vector>
#include <utility>

namespace GALAXY 
{
    namespace Core
    {
        class GameObject;
    }
    namespace Resource
    {
        class Prefab : public Scene
        {
            /* TODO:
             * Reflection to check if a variable need to be override, if it was changed
             * Change all game objects links to prefabs when prefab is changing (save or other)
             * Serialize when game object is save into a scene with the uuid of the prefab, and save only the changes
             */
        public:
            explicit Prefab(const Path& fullPath) : Scene(fullPath) {}
            Prefab& operator=(const Prefab& other) = default;
            Prefab(const Prefab&) = default;
            Prefab(Prefab&&) noexcept = default;
            virtual ~Prefab() {}

            void Load() override;
            void Send() override;
            void Save(const Path& fullPath = "") const override;

            // Automaticaly add to the scene
            std::weak_ptr<Core::GameObject> Instantiate(Weak<Core::GameObject> parent = {});

            static void CreateWith(const Path& fullPath, const std::shared_ptr<Core::GameObject>& gameObject);
            
            EDITOR_ONLY void ShowInInspector() override;
        private:
            void InstantiateInternal(Weak<Core::GameObject> parent, Shared<Core::GameObject> gameObject);
            
        private:
            std::vector<std::pair<std::shared_ptr<Core::GameObject>, std::weak_ptr<Core::GameObject>>> m_waitingObjects;
        };
        
    }
    
}

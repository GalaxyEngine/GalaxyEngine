#pragma once
#include "GalaxyAPI.h"
#include "Component/Collider.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API SphereCollider : public Collider
        {
            COMPONENT_SUBCLASS(SphereCollider, Collider)
        public:
            SphereCollider() = default;
            SphereCollider& operator=(const SphereCollider& other) = default;
            SphereCollider(const SphereCollider&) = default;
            SphereCollider(SphereCollider&&) noexcept = default;
            ~SphereCollider() override = default;

            ColliderType GetType() override { return ColliderType::Sphere; }

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
#endif

            void OnStart() override;
            void OnDestroy() override;

#ifdef WITH_EDITOR
            EDITOR_ONLY void OnEditorDraw() override;
#endif

            void SetSize(const float inSize) { m_size = inSize; }

            float GetSize() const { return m_size; }
        private:
            float m_size = 1;

        };
    }
}

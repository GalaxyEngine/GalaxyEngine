#pragma once
#include "GalaxyAPI.h"
#include "Component/Collider.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API SphereCollider : public Collider
        {
        public:
            SphereCollider() = default;
            SphereCollider& operator=(const SphereCollider& other) = default;
            SphereCollider(const SphereCollider&) = default;
            SphereCollider(SphereCollider&&) noexcept = default;
            ~SphereCollider() override = default;

            ColliderType GetType() override { return ColliderType::Sphere; }

            const char* GetComponentName() const override { return "SphereCollider"; }

            void ShowInInspector() override;

            void OnStart() override;
            void OnDestroy() override;

            void OnEditorDraw() override;

            void SetSize(const float inSize) { m_size = inSize; }

            float GetSize() const { return m_size; }
        private:
            float m_size = 1;

        };
    }
}

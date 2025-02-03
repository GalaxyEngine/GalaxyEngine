#pragma once
#include "GalaxyAPI.h"
#include "Component/Collider.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API BoxCollider : public Collider, public std::enable_shared_from_this<BoxCollider>
        {
			COMPONENT_SUBCLASS(BoxCollider, Collider)
        public:
            BoxCollider() = default;
            BoxCollider& operator=(const BoxCollider& other) = default;
            BoxCollider(const BoxCollider&) = default;
            BoxCollider(BoxCollider&&) noexcept = default;
            ~BoxCollider() override = default;

            ColliderType GetType() const override { return ColliderType::Box; }

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
            
            EDITOR_ONLY void OnEditorDraw() override;
#endif
            void OnStart() override;
            void OnDestroy() override;

            void SetSize(const Vec3f& inSize) { m_size = inSize; }

            Vec3f GetSize() const { return m_size; }

            Physic::AABB GetAABB() override;
            Vec3f Support(const Vec3f& direction) override; 
        private:
            Vec3f m_size = Vec3f(1, 1, 1);
            
        };
    }
}

#pragma once
#include "GalaxyAPI.h"
#include "Component/Collider.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API SphereCollider : public Collider, public std::enable_shared_from_this<SphereCollider>
        {
            COMPONENT_SUBCLASS(SphereCollider, Collider)
        public:
            SphereCollider() = default;
            SphereCollider& operator=(const SphereCollider& other) = default;
            SphereCollider(const SphereCollider&) = default;
            SphereCollider(SphereCollider&&) noexcept = default;
            ~SphereCollider() override = default;

            ColliderType GetType() const override { return ColliderType::Sphere; }

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
            
            EDITOR_ONLY void OnEditorDraw() override;
#endif

            void OnStart() override;
            void OnDestroy() override;

            void SetSize(const float inSize) { m_size = inSize; }
            float GetSize() const { return m_size; }

            Physic::AABB GetAABB() override;
            Vec3f Support(const Vec3f& direction) override;

            void Serialize(CppSer::Serializer& serializer) override;
            void Deserialize(CppSer::Parser& parser) override;

            Mat4 GetInverseInertia(float Mass) const override;
        private:
            float m_size = 1;

        };
    }
}

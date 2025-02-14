#pragma once
#include "Collider.h"
#include "GalaxyAPI.h"

namespace GALAXY 
{
    namespace Resource { class Mesh; }
    namespace Component
    {
        class GALAXY_API MeshCollider : public Collider, public std::enable_shared_from_this<MeshCollider>
        {
            COMPONENT_SUBCLASS(MeshCollider, Collider)
        public:
            MeshCollider() = default;
            MeshCollider& operator=(const MeshCollider& other) = default;
            MeshCollider(const MeshCollider&) = default;
            MeshCollider(MeshCollider&&) noexcept = default;
            ~MeshCollider() override = default;

            ColliderType GetType() const override { return ColliderType::Mesh; }
            
#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
            
            EDITOR_ONLY void OnEditorDraw() override;
#endif
            void OnCreate() override;

            void OnStart() override;
            void OnDestroy() override;
            
            void Serialize(CppSer::Serializer& serializer) override;
            void Deserialize(CppSer::Parser& parser) override;

            Physic::AABB GetAABB() override;
            Vec3f Support(const Vec3f& direction) override;

            void SetMesh(const Weak<Resource::Mesh>& mesh);

            void SetConvex(bool convex);
            bool IsConvex() const { return m_isConvex; }
        private:
            Weak<Resource::Mesh> m_mesh;
            Weak<Resource::Mesh> m_convexMesh;

            bool m_isConvex = true;
        };
    }
}

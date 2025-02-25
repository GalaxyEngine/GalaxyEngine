#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"
#include "Physic/AABB.h"

namespace GALAXY 
{
    namespace Component
    {
        class RigidBody;

        enum ColliderType
        {
            Sphere = 0,
            Box,
            Capsule,
            Mesh,

            Count
        };

        class GALAXY_API Collider : public IComponent<Collider>
        {
        public:
            Collider() = default;
            Collider& operator=(const Collider& other) = default;
            Collider(const Collider&) = default;
            Collider(Collider&&) noexcept = default;
            ~Collider() override = default;

            const char* GetComponentName() const override { return "Collider"; }

            virtual ColliderType GetType() const;

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
            
            EDITOR_ONLY void OnEditorDraw() override;
#endif

            void Serialize(CppSer::Serializer& serializer) override;
            void Deserialize(CppSer::Parser& parser) override;

            void OnUpdate() override;

            float GetFriction() const { return p_friction; }
            void SetFriction(float friction) {p_friction = friction;}

            float GetRestitution() const { return p_restitution; }
            void SetRestitution(float value) { p_restitution = value; }

            virtual Physic::AABB GetAABB() { return {}; }
            virtual Vec3f Support(const Vec3f& direction);

            void SetDebugCollide(bool value) { p_debugCollide = value; }
            bool GetDebugCollide() const { return p_debugCollide; }

            virtual Mat4 GetInverseInertia(float Mass) const { return {}; }
            
        protected:
            Weak<RigidBody> p_attachedRigidbody;

            float p_restitution = 0.5f;
            float p_friction = 0.5f;

            bool p_debugCollide = false;

            bool p_drawAABB = false;
        };
    }
}

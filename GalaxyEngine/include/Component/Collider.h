#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

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

            Count
        };

        struct GALAXY_API AABBCollider
        {
            Vec3f Min;
            Vec3f Max;
        };

        class GALAXY_API Collider : public IComponent<Collider>
        {
        public:

            const char* GetComponentName() const override { return "Collider"; }

            Quat m_offsetRot;
            Vec3f m_offsetPos;

            virtual ColliderType GetType();

            virtual AABBCollider GetAABB() { return AABBCollider(); }
        private:
            Weak<RigidBody> m_attachedRigidbody;
        };
    }
}

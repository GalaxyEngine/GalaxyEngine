#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"

namespace GALAXY 
{
    namespace Component
    {
        class Rigidbody;

        enum ColliderType
        {
            Sphere = 0,
            Box,
            Capsule,

            Count
        };

        class GALAXY_API Collider : public IComponent<Collider>
        {
        public:

            const char* GetComponentName() const override { return "Collider"; }

            Quat m_offsetRot;
            Vec3f m_offsetPos;

            virtual ColliderType GetType();
        private:
            Weak<Rigidbody> m_attachedRigidbody;
        };
    }
}

#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API RigidBody : public IComponent<RigidBody>
        {
        public:
            RigidBody() = default;
            RigidBody& operator=(const RigidBody& other) = default;
            RigidBody(const RigidBody&) = default;
            RigidBody(RigidBody&&) noexcept = default;
            ~RigidBody() override = default;

            const char* GetComponentName() const override { return "RigidBody"; }

            void ShowInInspector() override;

            void OnStart() override;
            void OnDestroy() override;
        private:
            float m_mass = 1.0f;
            float m_drag = 0.0f;
            float m_angularDrag = 0.0f;
            
        };
    }
}

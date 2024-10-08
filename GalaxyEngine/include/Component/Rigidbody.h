#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API Rigidbody : public IComponent<Rigidbody>
        {
        public:
            Rigidbody() = default;
            Rigidbody& operator=(const Rigidbody& other) = default;
            Rigidbody(const Rigidbody&) = default;
            Rigidbody(Rigidbody&&) noexcept = default;
            ~Rigidbody() override = default;

            const char* GetComponentName() const override { return "Rigidbody"; }

            EDITOR_ONLY void ShowInInspector() override;

            void OnStart() override;
            void OnDestroy() override;
        private:
            float m_mass = 1.0f;
            float m_drag = 0.0f;
            float m_angularDrag = 0.0f;
            
        };
    }
}

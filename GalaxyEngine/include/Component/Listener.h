#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"

namespace GALAXY 
{
    namespace Wrapper{class Audio;}
    namespace Component
    {
        class GALAXY_API Listener : public IComponent<Listener>
        {
        public:
            Listener() = default;
            Listener& operator=(const Listener& other) = default;
            Listener(const Listener&) = default;
            Listener(Listener&&) noexcept = default;
            ~Listener() override = default;
            
            const char* GetComponentName() const override { return "Listener"; }

            void OnTransformUpdate();
            
            void OnCreate() override;
            void OnDestroy() override;
        private:
            void SetPosition(const Vec3f& position);
            void SetDirection(const Vec3f& direction);
        private:
            friend Wrapper::Audio;
            uint32_t m_listenerIndex = 0;
        };
        
    }
}

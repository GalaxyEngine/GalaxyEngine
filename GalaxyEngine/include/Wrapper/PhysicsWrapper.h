#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Wrapper
    {
        enum class PhysicAPIType
        {
            Jolt,
            PhysX
        };        
        
        class PhysicsWrapper
        {
        public:
            ~PhysicsWrapper() = default;

            static void Initialize(PhysicAPIType type);

            static void Release();
        protected:
            virtual bool InitializeAPI() = 0;
        private:
            static PhysicsWrapper* m_instance; 
        };
    }
    
}

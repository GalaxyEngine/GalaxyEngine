#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Component
    {
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

            virtual ColliderType GetType() { return Count; }
        private:

        };
    }
}

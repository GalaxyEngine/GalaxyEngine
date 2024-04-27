#include "pch.h"
#include "Wrapper/PhysicAPI/JoltPhysics.h"


namespace GALAXY 
{
    static constexpr uint32_t cNumBodies = 10240;
    static constexpr uint32_t cNumBodyMutexes = 0; // Autodetect
    static constexpr uint32_t cMaxBodyPairs = 65536;
    static constexpr uint32_t cMaxContactConstraints = 20480;
    bool Wrapper::PhysicAPI::JoltAPI::InitializeAPI()
    {
        m_physicsSystem.Init(cNumBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_broadPhaseLayerInterface, m_objectVsBroadPhaseLayerFilter, m_objectVsObjectLayerFilter);
        m_physicsSystem.SetPhysicsSettings(JPH::PhysicsSettings());
    }
}

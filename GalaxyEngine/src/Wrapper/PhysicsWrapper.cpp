#include "pch.h"
#include "Wrapper/PhysicsWrapper.h"

#include "Wrapper/PhysicAPI/JoltPhysics.h"
namespace GALAXY 
{
    Wrapper::PhysicsWrapper* Wrapper::PhysicsWrapper::m_instance = nullptr;
    void Wrapper::PhysicsWrapper::Initialize(Wrapper::PhysicAPIType type)
    {
        switch (type)
        {
            case PhysicAPIType::Jolt:
            {
                m_instance = new Wrapper::PhysicAPI::JoltAPI();
                break;
            }
        case PhysicAPIType::PhysX:
            ASSERT(false || "PhysX not yet implemented");
            break;
        default:
            break;
        }
        m_instance->InitializeAPI();
    }

    void Wrapper::PhysicsWrapper::Release()
    {
        delete m_instance;
        m_instance = nullptr;
    }

    void Wrapper::PhysicsWrapper::AddDynamicBody(uint32_t id, Weak<Component::Rigidbody> body)
    {
        // ASSERT(p_dynamicBodies.find(id) == p_dynamicBodies.end());
        // p_dynamicBodies[id] = body;
    }

    void Wrapper::PhysicsWrapper::RemoveDynamicBody(uint32_t id)
    {
        // ASSERT(p_dynamicBodies.find(id) != p_dynamicBodies.end());
        // p_dynamicBodies.erase(id);
    }
}

#include "pch.h"
#include "Wrapper/PhysicsWrapper.h"

#ifdef USE_JOLT
#include "Wrapper/PhysicAPI/JoltPhysics.h"
#endif
#ifdef USE_PHYSX
#endif
#ifdef USE_CUSTOM_PHYSICS
#include "Wrapper/PhysicAPI/CustomPhysics.h"
#endif
namespace GALAXY 
{
    Wrapper::PhysicsWrapper* Wrapper::PhysicsWrapper::p_instance = nullptr;
    void Wrapper::PhysicsWrapper::Initialize(Wrapper::PhysicAPIType type)
    {
        switch (type)
        {
            case PhysicAPIType::Jolt:
            {
#ifdef USE_JOLT
                p_instance = new Wrapper::PhysicAPI::JoltAPI();
#else
                ASSERT(false && "You need to enable Jolt Physics API when compiling using --physic_api=jolt");
#endif
                break;
            }
            case PhysicAPIType::PhysX:
            {
#ifdef USE_PHYSX
                ASSERT(false && "PhysX not yet implemented");
#else
                ASSERT(false && "You need to enable PhysX API when compiling using --physic_api=physx");
#endif
                break;
            }
            case PhysicAPIType::Custom:
            {
#ifdef USE_CUSTOM_PHYSICS
                p_instance = new Wrapper::PhysicAPI::CustomPhysicsAPI();
#else
                ASSERT(false || "You need to enable Custom Physics API when compiling using --physic_api=custom");
#endif
                break;
            }
            default:
                break;
        }
        p_instance->p_collisionLayerManager.AddLayer("Default");
        p_instance->p_collisionLayerManager.AddLayer("Layer 1");
        p_instance->p_collisionLayerManager.AddLayer("Layer 2");
        p_instance->p_collisionLayerManager.AddLayer("Layer 3");
        p_instance->InitializeAPI();
    }

    void Wrapper::PhysicsWrapper::Release()
    {
        delete p_instance;
        p_instance = nullptr;
    }

    void Wrapper::PhysicsWrapper::AddDynamicBody(uint32_t id, Weak<Component::RigidBody> body)
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

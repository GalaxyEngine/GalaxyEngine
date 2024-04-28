#include "pch.h"
#include "Wrapper/PhysicAPI/JoltPhysics.h"

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "Component/BoxCollider.h"
#include "Component/Transform.h"

#include "Component/Rigidbody.h"
#include "Core/Application.h"

#include "Core/GameObject.h"


namespace GALAXY 
{

    static constexpr uint32_t cNumBodies = 10240;
    static constexpr uint32_t cNumBodyMutexes = 0; // Autodetect
    static constexpr uint32_t cMaxBodyPairs = 65536;
    static constexpr uint32_t cMaxContactConstraints = 20480;

    Wrapper::PhysicAPI::JoltAPI::~JoltAPI()
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        delete m_jobSystem;
        delete m_tempAllocator;
        delete m_physicsSystem;
    }

    void Wrapper::PhysicAPI::JoltAPI::Update()
    {
        if (!Core::Application::IsPlayMode())
            return;

        for (auto& body : m_dynamicBodies)
        {
            if (auto rigidbody = dynamic_cast<Component::Rigidbody*>(body.first))
            {
                Component::Transform* transform = rigidbody->GetTransform();
                Vec3f position = transform->GetWorldPosition();
                Quat rotation = transform->GetWorldRotation();
                m_physicsSystem->GetBodyInterface().SetPosition(body.second->GetID(), FromVec3(position), JPH::EActivation::Activate);
                m_physicsSystem->GetBodyInterface().SetRotation(body.second->GetID(), FromQuat(rotation), JPH::EActivation::Activate);
            }
        }
        
        JPH::BodyIDVector bodies; 
        m_physicsSystem->GetBodies(bodies);
        m_physicsSystem->Update(1.f / 60.f, 1, m_tempAllocator, m_jobSystem);
        
        for (auto& body : m_dynamicBodies)
        {
            if (auto rigidbody = dynamic_cast<Component::Rigidbody*>(body.first))
            {
                JPH::RMat44 transform = m_physicsSystem->GetBodyInterface().GetWorldTransform(body.second->GetID());
                JPH::Vec3 position = transform.GetTranslation();
                JPH::Quat rotation = transform.GetRotation().GetQuaternion();
                rigidbody->GetTransform()->SetWorldPosition(ToVec3(position));
                rigidbody->GetTransform()->SetWorldRotation(ToQuat(rotation));
            }
        }
    }

    void Wrapper::PhysicAPI::JoltAPI::CreateRigidbody(Component::Rigidbody* rigidbody)
    {
        Vec3f position = rigidbody->GetTransform()->GetWorldPosition();
        Quat rotation = rigidbody->GetTransform()->GetWorldRotation();
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
        JPH::RefConst<JPH::Shape> boxShape = new JPH::BoxShape(FromVec3(Vec3f::One()));
        JPH::BodyCreationSettings inSettings = JPH::BodyCreationSettings(boxShape, FromVec3(position), FromQuat(rotation), JPH::EMotionType::Dynamic, Layers::MOVING);
        JPH::Body& body = *bodyInterface.CreateBody(inSettings);
        bodyInterface.AddBody(body.GetID(), JPH::EActivation::Activate);

        m_dynamicBodies[rigidbody] = &body;
    }

    void Wrapper::PhysicAPI::JoltAPI::DestroyRigidbody(Component::Rigidbody* rigidbody)
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();

        auto staticBody = m_dynamicBodies.find(rigidbody);
        if (staticBody != m_dynamicBodies.end())
        {
            bodyInterface.RemoveBody(staticBody->second->GetID());
            bodyInterface.DestroyBody(staticBody->second->GetID());
            m_dynamicBodies.erase(staticBody);
        }
    }

    void Wrapper::PhysicAPI::JoltAPI::CreateBoxCollider(Component::BoxCollider* collider)
    {
        Vec3f position = collider->GetTransform()->GetWorldPosition();
        Quat rotation = collider->GetTransform()->GetWorldRotation();
        Vec3f scale = collider->GetTransform()->GetWorldScale();
        Vec3f boxSize = scale * collider->GetSize();
        JPH::RefConst<JPH::Shape> boxShape = new JPH::BoxShape(FromVec3(boxSize));
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();

        auto rigidbody = collider->GetGameObject()->GetComponent<Component::Rigidbody>();
        // if the rigidbody exists, set the shape of the body
        if (auto it = m_dynamicBodies.find(rigidbody.get()); it != m_dynamicBodies.end())
        {
            JPH::BodyID inBodyId = it->second->GetID();
            bodyInterface.SetShape(inBodyId, boxShape, true, JPH::EActivation::Activate);
        }
        // if the rigidbody does not exist, create a new body
        // and if the gameobject has a rigidbody, set the body as dynamic
        else
        {
            const bool hasRigidbody = rigidbody != nullptr;
            JPH::BodyCreationSettings inSettings = JPH::BodyCreationSettings(boxShape, FromVec3(position), FromQuat(rotation), hasRigidbody  ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static, Layers::MOVING);
            JPH::Body& body = *bodyInterface.CreateBody(inSettings);
            bodyInterface.AddBody(body.GetID(), hasRigidbody ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

            if (hasRigidbody)
            {
                m_dynamicBodies[rigidbody.get()] = &body;
            }
            else
            {
                m_staticBodies[collider] = &body;
            }
        }
    }

    void Wrapper::PhysicAPI::JoltAPI::DestroyBoxCollider(Component::BoxCollider* collider)
    {
        JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();

        auto staticBody = m_staticBodies.find(collider);
        if (staticBody != m_staticBodies.end())
        {
            bodyInterface.RemoveBody(staticBody->second->GetID());
            bodyInterface.DestroyBody(staticBody->second->GetID());
            m_staticBodies.erase(staticBody);
        }
        auto dynamicBody = m_dynamicBodies.find(collider);
        if (dynamicBody != m_dynamicBodies.end())
        {
            dynamicBody->second->GetShape()->Release();
        }
    }

    bool Wrapper::PhysicAPI::JoltAPI::InitializeAPI()
    {
        JPH::RegisterDefaultAllocator();

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        
        m_physicsSystem = new JPH::PhysicsSystem();
        m_physicsSystem->Init(cNumBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_broadPhaseLayerInterface, m_objectVsBroadPhaseLayerFilter, m_objectVsObjectLayerFilter);
        m_physicsSystem->SetPhysicsSettings(JPH::PhysicsSettings());

        m_physicsSystem->SetGravity(JPH::Vec3(0, -9.81f, 0));

        m_jobSystem = new JPH::JobSystemThreadPool(2048, 8, std::thread::hardware_concurrency() - 1);

        m_tempAllocator = new JPH::TempAllocatorImpl(1024 * 1024 * 32);

        
        const float scale = 1.f;
        float inSize = 100.f;

        JPH::Body& floor = *m_physicsSystem->GetBodyInterface().CreateBody(JPH::BodyCreationSettings(new JPH::BoxShape(scale * JPH::Vec3(0.5f * inSize, 1.0f, 0.5f * inSize), 0.0f), JPH::RVec3(scale * JPH::Vec3(0.0f, -1.0f, 0.0f)), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING));
        m_physicsSystem->GetBodyInterface().AddBody(floor.GetID(), JPH::EActivation::DontActivate);
        
        PrintLog("Jolt Physics Initialized");
        return true;
    }
}

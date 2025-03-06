#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

#include <random>
#include <unordered_set>

#include "Component/BoxCollider.h"
#include "Component/SphereCollider.h"
#include "Component/MeshCollider.h"
#include "Component/Transform.h"
#include "Component/RigidBody.h"

#include "Core/Application.h"
#include "Core/GameObject.h"

#include "Physic/Plane.h"

#include "Resource/Mesh.h"

#include "Utils/Time.h"

#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64

// Define custom hash function for Vec3f
namespace std
{
    template <>
    struct hash<Vec3f>
    {
        size_t operator()(const Vec3f& v) const noexcept
        {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
        }
    };
}

using namespace Wrapper::PhysicAPI;

namespace GALAXY
{
    Point::Point(const Vec3f& searchDir, Component::Collider* a, Component::Collider* b)
    {
        CalculateSupport(searchDir, a, b);
    }

    void Point::CalculateSupport(const Vec3f& searchDir, Component::Collider* a, Component::Collider* b)
    {
        supB = b->Support(searchDir);
        supA = a->Support(-searchDir);
        point = supB - supA;
    }

    void RigidBodyGroup::AddRigidbody(Shared<Component::RigidBody> rb)
    {
        if (rb->GetGroupIndex() != -1)
            return;
        rigidbodies.push_back(rb);
        rb->SetGroup(index);
    }

    bool RigidBodyGroup::IsSleeping() const
    {
        for (const auto& _rb : rigidbodies)
        {
            auto rb = _rb.lock();
            if (!rb)
                continue;
            if (!rb->IsSleeping())
                return false;
        }
        return true;
    }

    void RigidBodyGroup::WakeUp() const
    {
        for (const auto& _rb : rigidbodies)
        {
            auto rb = _rb.lock();
            if (!rb)
                continue;
            rb->WakeUp();
        }
    }

    Physic::AABB RigidBodyGroup::GetAABB() const
    {
        Physic::AABB aabb;
        aabb.Min = Vec3f(FLT_MAX);
        aabb.Max = Vec3f(FLT_MIN);
        for (auto& _rb : rigidbodies)
        {
            auto rb = _rb.lock();
            if (!rb)
                continue;
            Core::GameObject* gameObject = rb->GetGameObject();
            auto collider = gameObject->GetComponent<Component::Collider>();
            Physic::AABB _aabb = collider->GetAABB();

            aabb.Min.x = std::min(aabb.Min.x, _aabb.Min.x);
            aabb.Min.y = std::min(aabb.Min.y, _aabb.Min.y);
            aabb.Min.z = std::min(aabb.Min.z, _aabb.Min.z);

            aabb.Max.x = std::max(aabb.Max.x, _aabb.Max.x);
            aabb.Max.y = std::max(aabb.Max.y, _aabb.Max.y);
            aabb.Max.z = std::max(aabb.Max.z, _aabb.Max.z);
        }
        return aabb;
    }

    bool CustomPhysicsAPI::InitializeAPI()
    {
        m_dTOffset = 0.f;
        PrintLog("Custom Physics Initialized");
        return true;
    }

    void CustomPhysicsAPI::IntegrateAccel(float dt) const
    {
        for (auto& body : m_objectSet)
        {
            Shared<Component::RigidBody> rigidBody = body.lock();

            if (!rigidBody || !rigidBody->IsEnable())
                continue;

            float inverseMass = rigidBody->GetInverseMass();

            Vec3f vel = rigidBody->GetVelocity();
            Vec3f force = rigidBody->GetForce();
            Vec3f accel = force * inverseMass;

            // Reset static counter if acceleration is significant.
            if (accel.Length() > 0.1f && inverseMass > 0.f)
            {
                rigidBody->WakeUp();
            }

            if (inverseMass > 0)
            {
                accel += rigidBody->GetGravityForce();
            }

            vel += accel * dt;
            rigidBody->SetVelocity(vel);

            Vec3f torque = rigidBody->GetTorque();
            Vec3f angVel = rigidBody->GetAngularVelocity();

            rigidBody->UpdateInertiaTensor();
            Vec3f angAccel = rigidBody->GetInertiaTensor().MultiplyPoint3x4(torque);
            angVel += angAccel * dt;

            rigidBody->SetAngularVelocity(angVel);
        }
    }

    void CustomPhysicsAPI::ResolveCollisions(Component::Collider* collider1,
                                             Component::Collider* collider2,
                                             const CollisionInfo& collisionInfo)
    {
        // Retrieve rigid bodies.
        collider1->InternalUpdateRigidbody();
        collider2->InternalUpdateRigidbody();
        auto bodyA = collider1->GetAttachedRigidbody().lock();
        auto bodyB = collider2->GetAttachedRigidbody().lock();
        if (bodyA == bodyB)
        {
            return;
        }

        if (bodyA && !bodyA->IsEnable())
            bodyA = nullptr;

        if (bodyB && !bodyB->IsEnable())
            bodyB = nullptr;

        if (bodyA && bodyB)
        {
            const int groupA = bodyA->GetGroupIndex();
            const int groupB = bodyB->GetGroupIndex();
            if (groupA != -1 && groupB == -1)
            {
                int index = groupA;
                if (index < m_rigidBodyGroups.size())
                {
                    auto& group = m_rigidBodyGroups[index];
                    group.AddRigidbody(bodyB);
                }
                else
                {
                    // FIX THIS, NEVER SHOULD BE GOING HERE
                    auto& group = AddRigidbodyGroup();
                    group.AddRigidbody(bodyA);
                    group.AddRigidbody(bodyB);
                }
            }
            else if (groupA == -1 && groupB != -1)
            {
                int index = groupB;
                if (index < m_rigidBodyGroups.size())
                {
                    auto& group = m_rigidBodyGroups[index];
                    group.AddRigidbody(bodyA);
                }
                else
                {
                    // FIX THIS, NEVER SHOULD BE GOING HERE
                    auto& group = AddRigidbodyGroup();
                    group.AddRigidbody(bodyA);
                    group.AddRigidbody(bodyB);
                }
            }
            else if (groupA != groupB && groupA != -1 && groupB != -1)
            {
                bool merge = true;
                if (groupA < m_rigidBodyGroups.size())
                {
                    // FIX THIS, NEVER SHOULD BE GOING HERE
                    merge = false;
                    auto& group = AddRigidbodyGroup();
                    group.AddRigidbody(bodyA);
                    group.AddRigidbody(bodyB);
                }
                if (groupB < m_rigidBodyGroups.size())
                {
                    // FIX THIS, NEVER SHOULD BE GOING HERE
                    merge = false;
                    auto& group = AddRigidbodyGroup();
                    group.AddRigidbody(bodyA);
                    group.AddRigidbody(bodyB);
                }
                if (merge)
                    MergeRigidbodyGroup(groupA, groupB);
            }
            else if (groupA == -1 && groupB == -1)
            {
                auto& group = AddRigidbodyGroup();
                group.AddRigidbody(bodyA);
                group.AddRigidbody(bodyB);
            }
            else if (groupA == groupB)
            {
                // Same group, keep it
            }
            else
            {
                ASSERT(false);
            }
        }

        /*
        // Check Group
        for (int i = 0; i < m_rigidBodyGroups.size(); i++)
        {
            
            ASSERT(m_rigidBodyGroups[i].index == i);
            for (int j = 0; j < m_rigidBodyGroups[i].rigidbodies.size(); j++)
            {
                ASSERT(m_rigidBodyGroups[i].rigidbodies[j].lock()->GetGroupIndex() == i);
            }
        }
        */

        if (!bodyA && !bodyB)
            return; // Nothing to resolve if both bodies are static.

        // Retrieve transforms.
        auto transformA = collider1->GetGameObject()->GetTransform();
        auto transformB = collider2->GetGameObject()->GetTransform();

        // Get inverse masses.
        float invMassA = bodyA ? bodyA->GetInverseMass() : 0.f;
        float invMassB = bodyB ? bodyB->GetInverseMass() : 0.f;
        float invMassSum = invMassA + invMassB;
        if (invMassSum == 0.f)
            return; // Both objects are static.

        // ----- Positional Correction -----
        const auto& p = collisionInfo.point;

        float factor = (bodyA && bodyB) ? 0.5f : 1.0f;

        if (bodyA)
        {
            transformA->SetWorldPosition(transformA->GetWorldPosition() -
                (p.depth * factor * (invMassA / invMassSum)) * p.normal);
        }
        if (bodyB)
        {
            transformB->SetWorldPosition(transformB->GetWorldPosition() +
                (p.depth * factor * (invMassB / invMassSum)) * p.normal);
        }

        // ----- Impulse Resolution (Normal Impulse) -----
        // Compute contact offsets (rA and rB) from each center-of-mass to contact point.
        Vec3f rA = p.position - transformA->GetWorldPosition();
        Vec3f rB = p.position - transformB->GetWorldPosition();

        // Get current linear and angular velocities.
        Vec3f velA = bodyA ? bodyA->GetVelocity() : Vec3f::Zero();
        Vec3f velB = bodyB ? bodyB->GetVelocity() : Vec3f::Zero();
        Vec3f angVelA = bodyA ? bodyA->GetAngularVelocity() : Vec3f::Zero();
        Vec3f angVelB = bodyB ? bodyB->GetAngularVelocity() : Vec3f::Zero();

        // Compute the full velocity at the contact point.
        Vec3f fullVelA = velA + angVelA.Cross(rA);
        Vec3f fullVelB = velB + angVelB.Cross(rB);
        Vec3f contactVel = fullVelB - fullVelA;
        float velAlongNormal = contactVel.Dot(p.normal);

        // Do not resolve if objects are separating.
        if (velAlongNormal > 0)
            return;

        // Retrieve inverse inertia tensors.
        auto IinvA = bodyA ? bodyA->GetInverseInertiaTensor() : Mat4::Identity();
        auto IinvB = bodyB ? bodyB->GetInverseInertiaTensor() : Mat4::Identity();

        // Compute rotational terms for the normal impulse.
        Vec3f crossA = rA.Cross(p.normal);
        Vec3f crossB = rB.Cross(p.normal);
        float angularTermA = bodyA ? ((IinvA * crossA).Cross(rA)).Dot(p.normal) : 0.f;
        float angularTermB = bodyB ? ((IinvB * crossB).Cross(rB)).Dot(p.normal) : 0.f;

        // Total effective mass (linear + angular) in the normal direction.
        float effectiveMass = invMassSum + angularTermA + angularTermB;
        effectiveMass = std::max(effectiveMass, 1e-6f); // Prevent division by zero
        if (effectiveMass == 0.f)
            return;

        // Combine restitution values (average).
        float restitution = (collider1->GetRestitution() + collider2->GetRestitution()) * 0.5f;
        float j = -(1.0f + restitution) * velAlongNormal / effectiveMass;
        Vec3f normalImpulse = p.normal * j;

        // Apply the normal impulse to both bodies.
        Vec3f newVelA = velA;
        Vec3f newAngVelA = angVelA;
        Vec3f newVelB = velB;
        Vec3f newAngVelB = angVelB;

        if (bodyA)
        {
            newVelA -= normalImpulse * invMassA;
            newAngVelA += IinvA * (rA.Cross(-normalImpulse));
            bodyA->SetVelocity(newVelA);
            bodyA->SetAngularVelocity(newAngVelA);
        }
        if (bodyB)
        {
            newVelB += normalImpulse * invMassB;
            newAngVelB += IinvB * (rB.Cross(normalImpulse));
            bodyB->SetVelocity(newVelB);
            bodyB->SetAngularVelocity(newAngVelB);
        }

        // ----- Friction Impulse -----
        // Recompute the contact velocities at the contact point after applying the normal impulse.
        fullVelA = newVelA + newAngVelA.Cross(rA);
        fullVelB = newVelB + newAngVelB.Cross(rB);
        contactVel = fullVelB - fullVelA;

        // Compute the tangent (friction) direction.
        Vec3f tangent = contactVel - p.normal * contactVel.Dot(p.normal);
        if (tangent.LengthSquared() > 1e-6f) // Using squared length for efficiency.
        {
            tangent.Normalize();
        }
        else
        {
            // Generate an arbitrary perpendicular tangent if the computed one is too small.
            tangent = p.normal.Cross(Vec3f(1.0f, 0.0f, 0.0f));
            if (tangent.LengthSquared() < 1e-6f) // In case p.normal is parallel to (1,0,0).
            {
                tangent = p.normal.Cross(Vec3f(0.0f, 1.0f, 0.0f));
            }
            tangent.Normalize();
        }

        // Compute effective mass for the friction impulse.
        // For friction, effective mass = invMass + (r x tangent)^T * Iinv * (r x tangent)
        Vec3f rAxt = rA.Cross(tangent);
        Vec3f rBxt = rB.Cross(tangent);
        float angularTermA_tan = bodyA ? (IinvA * rAxt).Dot(rAxt) : 0.f;
        float angularTermB_tan = bodyB ? (IinvB * rBxt).Dot(rBxt) : 0.f;
        float effectiveMassFriction = invMassSum + angularTermA_tan + angularTermB_tan;

        float jt = 0.f;
        if (effectiveMassFriction != 0.f)
        {
            jt = -contactVel.Dot(tangent) / effectiveMassFriction;
        }

        // Compute the combined friction coefficient (average).
        float mu = (collider1->GetFriction() + collider2->GetFriction()) * 0.5f;
        // Clamp the friction impulse magnitude using Coulomb's law.
        float maxFrictionImpulse = fabs(j) * mu;
        if (fabs(jt) > maxFrictionImpulse)
        {
            jt = (jt < 0 ? -maxFrictionImpulse : maxFrictionImpulse);
        }
        Vec3f frictionImpulse = tangent * jt;

        // Apply the friction impulse.
        if (bodyA)
        {
            newVelA -= frictionImpulse * invMassA;
            newAngVelA += IinvA * (rA.Cross(-frictionImpulse));
            bodyA->SetVelocity(newVelA);
            bodyA->SetAngularVelocity(newAngVelA);
        }
        if (bodyB)
        {
            newVelB += frictionImpulse * invMassB;
            newAngVelB += IinvB * (rB.Cross(frictionImpulse));
            bodyB->SetVelocity(newVelB);
            bodyB->SetAngularVelocity(newAngVelB);
        }
    }

    void CustomPhysicsAPI::UpdateConstraints(float constraintDt)
    {
    }

    void CustomPhysicsAPI::IntegrateVelocity(float dt)
    {
        for (auto& _body : m_objectSet)
        {
            auto body = _body.lock();

            if (!body || !body->IsEnable())
                continue;
            Component::Transform* transform = body->GetTransform();

            Vec3f position = transform->GetWorldPosition();
            Vec3f velocity = body->GetVelocity();
            Vec3f angularVelocity = body->GetAngularVelocity();

            Vec3f dPosition = velocity * dt;
            Vec3f dAngle = angularVelocity * dt;

            if (dPosition.Length() < p_sleepThreshold)
            {
                body->AddStaticTime(dt);
            }
            else
            {
                const int groupIndex = body->GetGroupIndex();
                if (groupIndex != -1)
                {
                    if (groupIndex >= m_rigidBodyGroups.size())
                    {
                        // FIX THIS, NEVER SHOULD BE GOING HERE
                    }
                    else
                    {
                        m_rigidBodyGroups[groupIndex].WakeUp();
                    }
                }
                body->WakeUp();
            }

            body->SetIsSleeping(body->GetStaticTime() >= m_staticCountMax);

            if (!body->IsSleeping())
            // if (true)
            {
                position += velocity * dt;

                transform->SetWorldPosition(position);

                velocity *= (1.0f - body->GetDrag() * dt);
                body->SetVelocity(velocity);

                Quat rotation = transform->GetWorldRotation();

                Vec3f a = dAngle * 0.5f;

                rotation = rotation + (Quat(a.x, a.y, a.z, 0) * rotation);
                rotation.Normalize();

                transform->SetWorldRotation(rotation);

                float frameAngularDamping = 1.0f - (body->GetAngularDrag() * dt);
                angularVelocity = angularVelocity * frameAngularDamping;
                body->SetAngularVelocity(angularVelocity);
            }
            else
            {
                body->SetStaticTime(m_staticCountMax);
                body->SetVelocity(Vec3f::Zero());
            }
        }
    }

    //This is the fixed timestep we'd LIKE to have
    const int idealHZ = 144;
    const float idealDT = 1.0f / idealHZ;

    /*
    This is the fixed update we actually have...
    If physics takes too long it starts to kill the framerate, it'll drop the 
    iteration count down until the FPS stabilises, even if that ends up
    being at a low rate. 
    */
    int realHZ = idealHZ;
    float realDT = idealDT;
    int constraintIterationCount = 1;

    void CustomPhysicsAPI::Update()
    {
#ifdef WITH_EDITOR
        if (!Core::Application::IsPlayMode())
            return;
#endif
        float dt = Utils::Time::DeltaTime();

        // Remove expired objects
        m_collisionInfos.clear();
        ClearRigidbodyGroup();
        std::erase_if(m_objectSet, [](const Weak<Component::RigidBody>& body) { return body.expired(); });
        std::erase_if(m_colliderSet, [](const Weak<Component::Collider>& collider) { return collider.expired(); });

        /*
        for (auto _rb : m_objectSet)
        {
            if (auto rb = _rb.lock())
            {
                if (rb->GetTransform()->WasDirty() && rb->IsSleeping)
                {
                    WakeUpRigidBody(rb.get());
                }
            }
        }
        */

        //TODO: Update AABB for broadphases

        Utils::ElapsedTimer timer;
        m_dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!
        while (m_dTOffset >= realDT)
        {
            IntegrateAccel(realDT);

            std::vector<ColliderPair> objects = BroadPhase();

            for (auto& object : objects)
            {
                if (!object.first->IsEnable() || !object.second->IsEnable())
                    continue;
                if (!Physic::CollisionLayerManager::CanCollide(object.first->GetCollisionLayer(),
                                                               object.second->GetCollisionLayer()))
                    continue;
                CollisionInfo info;
                if (GJK(object.first, object.second, info))
                {
#ifdef _DEBUG
                    object.first->SetDebugCollide(true);
                    object.second->SetDebugCollide(true);
#endif
                    object.first->EOnCollide.Invoke(object.first, object.second, info.point);
                    object.second->EOnCollide.Invoke(object.second, object.first, info.point);
                    info.framesLeft = m_numCollisionFrames;
                    ResolveCollisions(object.first, object.second, info);
                    m_collisionInfos.push_back(info);
                }
            }

            float constraintDt = realDT / (float)constraintIterationCount;
            for (int i = 0; i < constraintIterationCount; ++i)
            {
                UpdateConstraints(realDT);
            }

            IntegrateVelocity(realDT); //update positions from new velocity changes

            m_dTOffset -= realDT;
        }

        timer.Stop();
        float updateTime = static_cast<float>(timer.GetElapsedTime().AsSeconds());

        //Uh oh, physics is taking too long...
        if (updateTime > realDT)
        {
            realHZ /= 2;
            realDT *= 2;
            PrintLog("Dropping iteration count due to long physics time...(now %d)", realHZ);
        }
        else if (dt * 2 < realDT)
        {
            //we have plenty of room to increase iteration count!
            int temp = realHZ;
            realHZ *= 2;
            realDT /= 2;

            if (realHZ > idealHZ)
            {
                realHZ = idealHZ;
                realDT = idealDT;
            }
            if (temp != realHZ)
            {
                PrintLog("Raising iteration count due to short physics time...(now %d)", realHZ);
            }
        }
    }

    static std::vector color_for_index = {
        Vec4f(0.87f, 0.12f, 0.98f, 1.0f),
        Vec4f(0.34f, 0.56f, 0.91f, 1.0f),
        Vec4f(0.22f, 0.89f, 0.45f, 1.0f),
        Vec4f(0.76f, 0.32f, 0.54f, 1.0f),
        Vec4f(0.11f, 0.47f, 0.79f, 1.0f),
        Vec4f(0.65f, 0.88f, 0.13f, 1.0f),
        Vec4f(0.91f, 0.34f, 0.27f, 1.0f),
        Vec4f(0.48f, 0.75f, 0.92f, 1.0f),
        Vec4f(0.23f, 0.68f, 0.55f, 1.0f),
        Vec4f(0.56f, 0.12f, 0.77f, 1.0f),
        Vec4f(0.90f, 0.45f, 0.33f, 1.0f),
        Vec4f(0.67f, 0.89f, 0.44f, 1.0f),
        Vec4f(0.15f, 0.64f, 0.82f, 1.0f),
        Vec4f(0.39f, 0.91f, 0.26f, 1.0f),
        Vec4f(0.53f, 0.78f, 0.88f, 1.0f),
        Vec4f(0.82f, 0.29f, 0.64f, 1.0f)
    };


    void CustomPhysicsAPI::DrawDebug()
    {
        return;
        auto instance = Renderer::GetInstance();
        /*
        for (const CollisionInfo& info : m_collisionInfos)
        {
            instance->DrawSimpleWireSphere(info.point.position, 0.1f, 32, Vec4f(1, 0, 0, 1), 10.f);
            instance->DrawLine(info.point.position, info.point.position + info.point.normal * info.point.depth,
                               Vec4f(1, 0, 0, 1), 10.f);
        }
        */

        for (auto& group : m_rigidBodyGroups)
        {
            auto aabb = group.GetAABB();
            Vec4f color = color_for_index[group.index % color_for_index.size()];
            instance->DrawWireCube(aabb.GetCenter(), aabb.GetExtents(), color, 10.f);
        }
    }

    void CustomPhysicsAPI::CreateRigidBody(Weak<Component::RigidBody> rigidbody)
    {
        m_objectSet.insert(rigidbody);
    }

    void CustomPhysicsAPI::DestroyRigidBody(Weak<Component::RigidBody> rigidbody)
    {
        auto object = m_objectSet.find(rigidbody); // Use auto, no reference
        if (object == m_objectSet.end())
        {
            PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
            return;
        }
        m_objectSet.erase(object); // Erase using the iterator
    }

    void CustomPhysicsAPI::CreateBoxCollider(Weak<Component::BoxCollider> collider)
    {
        m_colliderSet.insert(collider);
    }

    void CustomPhysicsAPI::DestroyBoxCollider(Weak<Component::BoxCollider> collider)
    {
        auto object = m_colliderSet.find(collider); // Use auto, no reference
        if (object == m_colliderSet.end())
        {
            PrintError("Could not find collider associated with component 0x%x !", collider);
            return;
        }
        m_colliderSet.erase(object); // Erase using the iterator
    }

    void CustomPhysicsAPI::CreateMeshCollider(Weak<Component::MeshCollider> collider)
    {
        m_colliderSet.insert(collider);
    }

    void CustomPhysicsAPI::DestroyMeshCollider(Weak<Component::MeshCollider> collider)
    {
        m_colliderSet.erase(collider);
    }

    void CustomPhysicsAPI::CreateSphereCollider(Weak<Component::SphereCollider> collider)
    {
        m_colliderSet.insert(collider);
    }

    void CustomPhysicsAPI::DestroySphereCollider(Weak<Component::SphereCollider> collider)
    {
        m_colliderSet.erase(collider);
    }

    void CustomPhysicsAPI::SetDefaultGravity(const Vec3f& value)
    {
        m_defaultGravity = value;
    }

    void CustomPhysicsAPI::AddForce(Weak<Component::RigidBody> rigidbody, const Vec3f& force)
    {
        auto object = m_objectSet.find(rigidbody); // Use auto, no reference
        if (object == m_objectSet.end())
        {
            PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
            return;
        }
        Component::RigidBody* rigidbodyComponent = object->lock().get();
        float mass = rigidbodyComponent->GetMass();
        Vec3f velocity = rigidbodyComponent->GetVelocity();
        rigidbodyComponent->SetVelocity(velocity + force / mass);
    }

    void CustomPhysicsAPI::AddForceAtPosition(const Weak<Component::RigidBody>& weak, const Vec3f& force,
                                              const Vec3f& position)
    {
        auto object = m_objectSet.find(weak); // Use auto, no reference
        if (object == m_objectSet.end())
        {
            PrintError("Could not find rigidbody associated with component 0x%x !", weak);
            return;
        }
        Component::RigidBody* rigidbodyComponent = object->lock().get();

        Component::Transform* transform = rigidbodyComponent->GetTransform();
        if (!transform)
            return;

        Vec3f centerOfMass = transform->GetWorldPosition();
        Vec3f offset = position - centerOfMass;

        AddForce(weak, force);

        Vec3f torque = offset.Cross(force);

        rigidbodyComponent->SetAngularVelocity(rigidbodyComponent->GetAngularVelocity() + torque);
    }

    void CustomPhysicsAPI::AddTorque(const Weak<Component::RigidBody>& weak, const Vec3f& torque)
    {
        auto object = m_objectSet.find(weak); // Use auto, no reference
        if (object == m_objectSet.end())
        {
            PrintError("Could not find rigidbody associated with component 0x%x !", weak);
            return;
        }
        Component::RigidBody* rigidbodyComponent = object->lock().get();
        Vec3f angularVelocity = rigidbodyComponent->GetAngularVelocity();
        rigidbodyComponent->SetAngularVelocity(angularVelocity + torque);
    }

    void CustomPhysicsAPI::WakeUpRigidBody(Component::RigidBody* rigidbody)
    {
        rigidbody->WakeUp();
        if (rigidbody->GetGroupIndex() != -1)
            m_rigidBodyGroups[rigidbody->GetGroupIndex()].WakeUp();
    }

#pragma region GJK Mesh
    Weak<Resource::Mesh> CustomPhysicsAPI::GetConvexMesh(Shared<Resource::Mesh> mesh)
    {
        if (!mesh)
            return {};

        auto it = m_convexMesh.find(mesh->GetUUID());
        if (it != m_convexMesh.end())
            return it->second;

        Shared<Resource::Mesh> convexMesh = std::make_shared<Resource::Mesh>("Convex_" + mesh->GetMeshName());
        m_convexMesh[mesh->GetUUID()] = convexMesh;

        if (mesh->HasBeenSent())
            ComputeConvexVertices(mesh);
        else
            mesh->OnLoad.Bind([this, mesh] { ComputeConvexVertices(mesh); });

        return convexMesh;
    }

    std::vector<Vec3f> CustomPhysicsAPI::ComputeConvexHull(const std::vector<Vec3f>& meshPositionsVert)
    {
        // --- Step 1. Compute extreme vertices using spherical sampling ---
        auto Support = [&](const Vec3f& direction) -> Vec3f
        {
            float bestDot = -std::numeric_limits<float>::infinity();
            Vec3f bestVertex;
            for (const Vec3f& vertex : meshPositionsVert)
            {
                float dot = direction.Dot(vertex);
                if (dot > bestDot)
                {
                    bestDot = dot;
                    bestVertex = vertex;
                }
            }
            return bestVertex;
        };

        const int numTheta = 18; // polar divisions
        const int numPhi = 36; // azimuth divisions
        std::vector<Vec3f> convexHullVertices;
        const float tolerance = 1e-6f;

        for (int i = 0; i <= numTheta; ++i)
        {
            float theta = i * PI / numTheta;
            for (int j = 0; j < numPhi; ++j)
            {
                float phi = j * 2.0f * PI / numPhi;
                Vec3f direction(std::sin(theta) * std::cos(phi),
                                std::sin(theta) * std::sin(phi),
                                std::cos(theta));
                Vec3f point = Support(direction);

                bool isUnique = true;
                for (const Vec3f& v : convexHullVertices)
                {
                    if ((v - point).Length() < tolerance)
                    {
                        isUnique = false;
                        break;
                    }
                }
                if (isUnique)
                    convexHullVertices.push_back(point);
            }
        }

        // --- Step 2. Group the convex-hull vertices into planar faces and triangulate them ---
        Vec3f overallCenter(0, 0, 0);
        for (const Vec3f& v : convexHullVertices)
            overallCenter = overallCenter + v;
        overallCenter = overallCenter / convexHullVertices.size();

        // Structure to hold a face (planar polygon) of the hull
        struct FacePolygon
        {
            Vec3f normal;
            float offset;
            std::vector<int> indices; // indices into convexHullVertices that lie on the same plane
        };
        std::vector<FacePolygon> facePolygons;
        size_t n = convexHullVertices.size();

        // For each combination of 3 vertices, check if they define a hull face.
        // (A face is detected if all other points lie on one side of the plane.)
        for (int i = 0; i < n; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                for (int k = j + 1; k < n; k++)
                {
                    Vec3f edge1 = convexHullVertices[j] - convexHullVertices[i];
                    Vec3f edge2 = convexHullVertices[k] - convexHullVertices[i];
                    Vec3f normal = edge1.Cross(edge2);
                    if (normal.Length() < tolerance)
                        continue; // degenerate triangle
                    normal = normal.GetNormalize();
                    float offset = normal.Dot(convexHullVertices[i]);

                    // Check that every other point is on one side of the plane.
                    bool allPositive = true;
                    bool allNegative = true;
                    for (int m = 0; m < n; m++)
                    {
                        if (m == i || m == j || m == k)
                            continue;
                        float d = normal.Dot(convexHullVertices[m]) - offset;
                        if (d > tolerance)
                            allNegative = false;
                        if (d < -tolerance)
                            allPositive = false;
                    }
                    if (!(allPositive || allNegative))
                        continue; // Not a hull face

                    // Adjust normal so it points outward relative to the overall center.
                    float centerDist = normal.Dot(overallCenter) - offset;
                    if (centerDist > 0)
                    {
                        normal = -normal;
                        offset = -offset;
                    }

                    // Avoid duplicate faces by checking if a similar face (same plane) was already added.
                    bool found = false;
                    for (auto& fp : facePolygons)
                    {
                        if (fabs(fp.normal.Dot(normal) - 1.0f) < tolerance && fabs(fp.offset - offset) < tolerance)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        continue;

                    // Build a new face: collect all vertices that lie in this plane.
                    FacePolygon fp;
                    fp.normal = normal;
                    fp.offset = offset;
                    for (int m = 0; m < n; m++)
                    {
                        float d = fabs(fp.normal.Dot(convexHullVertices[m]) - fp.offset);
                        if (d < tolerance)
                            fp.indices.push_back(m);
                    }
                    // Remove duplicate indices (if any) and sort them.
                    std::sort(fp.indices.begin(), fp.indices.end());
                    fp.indices.erase(std::unique(fp.indices.begin(), fp.indices.end()), fp.indices.end());

                    // Order the vertices around the face.
                    // Compute the face’s centroid.
                    Vec3f faceCentroid(0, 0, 0);
                    for (int idx : fp.indices)
                        faceCentroid = faceCentroid + convexHullVertices[idx];
                    faceCentroid = faceCentroid / fp.indices.size();

                    // Choose a basis for the plane.
                    Vec3f u;
                    if (fabs(fp.normal.x) > fabs(fp.normal.y))
                        u = Vec3f(-fp.normal.z, 0, fp.normal.x).GetNormalize();
                    else
                        u = Vec3f(0, fp.normal.z, -fp.normal.y).GetNormalize();
                    Vec3f v = fp.normal.Cross(u);

                    // Sort the vertex indices by angle around the centroid.
                    std::ranges::sort(fp.indices, [&](int a, int b)
                    {
                        Vec3f pa = convexHullVertices[a] - faceCentroid;
                        Vec3f pb = convexHullVertices[b] - faceCentroid;
                        float angleA = std::atan2(pa.Dot(v), pa.Dot(u));
                        float angleB = std::atan2(pb.Dot(v), pb.Dot(u));
                        return angleA < angleB;
                    });

                    facePolygons.push_back(fp);
                }
            }
        }

        // --- Step 3. Triangulate each face polygon using a fan method ---
        std::vector<Vec3f> triangleVertices; // This will contain groups of 3 vertices per triangle.
        for (const auto& fp : facePolygons)
        {
            if (fp.indices.size() < 3)
                continue;
            // Compute the centroid of the face to check orientation.
            Vec3f polyCentroid(0, 0, 0);
            for (int idx : fp.indices)
                polyCentroid = polyCentroid + convexHullVertices[idx];
            polyCentroid = polyCentroid / fp.indices.size();
            // If the centroid is on the “wrong” side, reverse the ordering.
            bool reverseOrder = (fp.normal.Dot(polyCentroid) - fp.offset > 0);
            std::vector<int> orderedIndices = fp.indices;
            if (reverseOrder)
                std::reverse(orderedIndices.begin(), orderedIndices.end());

            // Fan-triangulate the convex polygon.
            for (size_t i = 1; i < orderedIndices.size() - 1; i++)
            {
                triangleVertices.push_back(convexHullVertices[orderedIndices[0]]);
                triangleVertices.push_back(convexHullVertices[orderedIndices[i]]);
                triangleVertices.push_back(convexHullVertices[orderedIndices[i + 1]]);
            }
        }

        return triangleVertices;
    }


    void CustomPhysicsAPI::ComputeConvexVertices(Shared<Resource::Mesh> mesh)
    {
        Shared<Resource::Mesh> convexMesh = m_convexMesh[mesh->GetUUID()];
        if (!mesh || (convexMesh && convexMesh->HasBeenSent()))
            return;

        std::vector<Vec3f> positions = mesh->GetPositionVertices();
        std::vector<Vec3f> convexVertices;

        if (positions.empty())
        {
            convexMesh->SetMeshPosition(convexVertices);
            return;
        }

        convexVertices = ComputeConvexHull(positions);

        convexMesh->SetMeshPosition(convexVertices);
        PrintLog("Convex mesh created for %s", mesh->GetMeshName().c_str());
    }
#pragma endregion

    bool CustomPhysicsAPI::Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance,
                                   Physic::RaycastHit& hit)
    {
        // TODO : FIX THIS
        bool hitFound = false;
        float closestDistance = maxDistance;
        // Ensure the direction is normalized.
        Vec3f dir = direction.GetNormalize();

        // Lambda to perform a ray-AABB intersection using the slab method.
        auto RayAABBIntersect = [&](const Vec3f& rayOrigin, const Vec3f& rayDir,
                                    const Vec3f& boxMin, const Vec3f& boxMax, float& tOut) -> bool
        {
            float tMin = 0.0f;
            float tMax = maxDistance;

            // X-axis
            if (fabs(rayDir.x) < 1e-8f)
            {
                if (rayOrigin.x < boxMin.x || rayOrigin.x > boxMax.x)
                    return false;
            }
            else
            {
                float t1 = (boxMin.x - rayOrigin.x) / rayDir.x;
                float t2 = (boxMax.x - rayOrigin.x) / rayDir.x;
                if (t1 > t2)
                    std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax)
                    return false;
            }

            // Y-axis
            if (fabs(rayDir.y) < 1e-8f)
            {
                if (rayOrigin.y < boxMin.y || rayOrigin.y > boxMax.y)
                    return false;
            }
            else
            {
                float t1 = (boxMin.y - rayOrigin.y) / rayDir.y;
                float t2 = (boxMax.y - rayOrigin.y) / rayDir.y;
                if (t1 > t2)
                    std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax)
                    return false;
            }

            // Z-axis
            if (fabs(rayDir.z) < 1e-8f)
            {
                if (rayOrigin.z < boxMin.z || rayOrigin.z > boxMax.z)
                    return false;
            }
            else
            {
                float t1 = (boxMin.z - rayOrigin.z) / rayDir.z;
                float t2 = (boxMax.z - rayOrigin.z) / rayDir.z;
                if (t1 > t2)
                    std::swap(t1, t2);
                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);
                if (tMin > tMax)
                    return false;
            }

            tOut = tMin;
            return true;
        };

        // Iterate over each collider in the set.
        for (auto& weakCollider : m_colliderSet)
        {
            Shared<Component::Collider> collider = weakCollider.lock();
            if (!collider)
                continue;

            // Retrieve the collider's AABB.
            Physic::AABB aabb = collider->GetAABB();
            float t = 0.0f;
            if (RayAABBIntersect(origin, dir, aabb.Min, aabb.Max, t))
            {
                if (t < closestDistance)
                {
                    closestDistance = t;
                    hit.collider = collider;
                    hit.distance = t;
                    hit.point = origin + dir * t;

                    // Compute a simple normal based on which AABB face is hit.
                    // (This method assumes the hit point lies very close to one of the faces.)
                    const float epsilon = 1e-4f;
                    if (fabs(hit.point.x - aabb.Min.x) < epsilon)
                        hit.normal = Vec3f(-1, 0, 0);
                    else if (fabs(hit.point.x - aabb.Max.x) < epsilon)
                        hit.normal = Vec3f(1, 0, 0);
                    else if (fabs(hit.point.y - aabb.Min.y) < epsilon)
                        hit.normal = Vec3f(0, -1, 0);
                    else if (fabs(hit.point.y - aabb.Max.y) < epsilon)
                        hit.normal = Vec3f(0, 1, 0);
                    else if (fabs(hit.point.z - aabb.Min.z) < epsilon)
                        hit.normal = Vec3f(0, 0, -1);
                    else if (fabs(hit.point.z - aabb.Max.z) < epsilon)
                        hit.normal = Vec3f(0, 0, 1);
                    else
                        hit.normal = Vec3f::Zero(); // Fallback if no face is clearly hit.

                    hitFound = true;
                }
            }
        }

        hit.hit = hitFound;
        return hitFound;
    }

#pragma region Collision Detection
    struct SAPAABB
    {
        Component::Collider* collider;
        Vec3f Min;
        Vec3f Max;
    };

    std::vector<ColliderPair> CustomPhysicsAPI::BroadPhase() const
    {
        auto colliders = m_colliderSet;
        std::vector<SAPAABB> aabbs;

        // First, gather all AABBs from the colliders
        for (auto& _collider : colliders)
        {
            Shared<Component::Collider> collider = _collider.lock();
            if (collider)
            {
                SAPAABB box;
                Physic::AABB aabb = collider->GetAABB();

                box.Min = aabb.Min;
                box.Max = aabb.Max;
                box.collider = collider.get();

                aabbs.push_back(box);
            }
        }

        // Sort AABBs along one axis (e.g., x-axis)
        std::ranges::sort(aabbs, [](const SAPAABB& a, const SAPAABB& b)
        {
            return a.Min.x < b.Min.x;
        });

        // Perform sweep and prune to find potential collisions
        std::vector<ColliderPair> result;
        for (size_t i = 0; i < aabbs.size(); ++i)
        {
            for (size_t j = i + 1; j < aabbs.size(); ++j)
            {
                // Check if the AABBs overlap
                if (aabbs[i].Max.x < aabbs[j].Min.x)
                {
                    // No overlap, and since the list is sorted, we can break out of the inner loop
                    break;
                }

                if (aabbs[i].Max.y >= aabbs[j].Min.y && aabbs[i].Min.y <= aabbs[j].Max.y &&
                    aabbs[i].Max.z >= aabbs[j].Min.z && aabbs[i].Min.z <= aabbs[j].Max.z)
                {
                    // AABBs overlap, add the colliders to the result
                    result.emplace_back(aabbs[i].collider, aabbs[j].collider);
                }
            }
        }

        return result;
    }

    // Triangle case
    void UpdateSimplex3(const Point& a, Point& b, Point& c, Point& d, int& simpDim, Vec3f& searchDir)
    {
        /* Required winding order:
        //  b
        //  | \
        //  |   \
        //  |    a
        //  |   /
        //  | /
        //  c
        */
        Vec3f n = (b.point - a.point).Cross(c.point - a.point); // Triangle's normal
        Vec3f AO = -a.point; // Direction to origin

        // Determine which feature is closest to origin, make that the new simplex

        simpDim = 2;
        if ((b.point - a.point).Cross(n).Dot(AO) > 0) // Closest to edge AB
        {
            c = a;
            //simp_dim = 2;
            searchDir = (b.point - a.point).Cross(AO).Cross(b.point - a.point);
            return;
        }
        if (n.Cross(c.point - a.point).Dot(AO) > 0) // Closest to edge AC
        {
            b = a;
            //simp_dim = 2;
            searchDir = (c.point - a.point).Cross(AO).Cross(c.point - a.point);
            return;
        }

        simpDim = 3;
        if (n.Dot(AO) > 0) // Above triangle
        {
            d = c;
            c = b;
            b = a;
            //simp_dim = 3;
            searchDir = n;
            return;
        }
        // else below triangle
        d = b;
        b = a;
        //simp_dim = 3;
        searchDir = -n;
    }

    // Tetrahedral case
    bool UpdateSimplex4(const Point& a, Point& b, Point& c, Point& d, int& simpDim, Vec3f& searchDir)
    {
        // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
        // We know a priori that origin is above BCD and below a

        // Get normals of three new faces
        Vec3f ABC = (b.point - a.point).Cross(c.point - a.point);
        Vec3f ACD = (c.point - a.point).Cross(d.point - a.point);
        Vec3f ADB = (d.point - a.point).Cross(b.point - a.point);

        Vec3f AO = -a.point; // dir to origin
        simpDim = 3;

        // Plane-test origin with 3 faces
        /*
        // Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
        // We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this 
        // is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like 
        // this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
        */
        if (ABC.Dot(AO) > 0) // In front of ABC
        {
            d = c;
            c = b;
            b = a;
            searchDir = ABC;
            return false;
        }
        if (ACD.Dot(AO) > 0) // In front of ACD
        {
            b = a;
            searchDir = ACD;
            return false;
        }
        if (ADB.Dot(AO) > 0) // In front of ADB
        {
            c = d;
            d = b;
            b = a;
            searchDir = ADB;
            return false;
        }

        // else inside tetrahedron; enclosed!
        return true;
    }

    void Barycentric(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& p, float& u, float& v, float& w)
    {
        Vec3f v0 = b - a, v1 = c - a, v2 = p - a;
        float d00 = v0.Dot(v0);
        float d01 = v0.Dot(v1);
        float d11 = v1.Dot(v1);
        float d20 = v2.Dot(v0);
        float d21 = v2.Dot(v1);
        float denom = d00 * d11 - d01 * d01;
        if (denom == 0)
            denom == 0.00001f;
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
        u = 1.0f - v - w;
    }

    // Expanding Polytope Algorithm
    // Find minimum translation vector to resolve collision
    void CustomPhysicsAPI::EPA(const Point& a, const Point& b, const Point& c, const Point& d,
                               Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo)
    {
        Point faces[EPA_MAX_NUM_FACES][4]; // Array of faces, each with 3 verts and a normal

        //Init with final simplex from GJK

        //Init with final simplex from GJK
        faces[0][0] = a;
        faces[0][1] = b;
        faces[0][2] = c;
        faces[0][3].point = (b.point - a.point).Cross(c.point - a.point).GetNormalize(); //ABC
        faces[1][0] = a;
        faces[1][1] = c;
        faces[1][2] = d;
        faces[1][3].point = (c.point - a.point).Cross(d.point - a.point).GetNormalize(); //ACD
        faces[2][0] = a;
        faces[2][1] = d;
        faces[2][2] = b;
        faces[2][3].point = (d.point - a.point).Cross(b.point - a.point).GetNormalize(); //ADB
        faces[3][0] = b;
        faces[3][1] = d;
        faces[3][2] = c;
        faces[3][3].point = (d.point - b.point).Cross(c.point - b.point).GetNormalize(); //BDC

        int num_faces = 4;
        int closest_face;

        for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++)
        {
            // Find face that's closest to origin
            float min_dist = faces[0][0].point.Dot(faces[0][3].point);
            closest_face = 0;
            for (int i = 1; i < num_faces; i++)
            {
                float dist = faces[i][0].point.Dot(faces[i][3].point);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    closest_face = i;
                }
            }

            // Search normal to face that's closest to origin
            Vec3f search_dir = faces[closest_face][3].point;
            Point p = Point(search_dir, coll1, coll2);

            if (p.point.Dot(search_dir) - min_dist < EPA_TOLERANCE)
            {
                // Compute barycentric coordinates for the projection of the origin
                Physic::Plane closestPlane = Physic::Plane::PlaneFromTri(
                    faces[closest_face][0].point,
                    faces[closest_face][1].point,
                    faces[closest_face][2].point
                );
                Vec3f projectionPoint = closestPlane.ProjectPointOntoPlane(Vec3f::Zero());
                float u, v, w;
                Barycentric(faces[closest_face][0].point,
                            faces[closest_face][1].point,
                            faces[closest_face][2].point,
                            projectionPoint, u, v, w);

                // Get the support points from each collider in world space using the same barycentrics
                Vec3f worldA = faces[closest_face][0].supA * u +
                    faces[closest_face][1].supA * v +
                    faces[closest_face][2].supA * w;
                Vec3f worldB = faces[closest_face][0].supB * u +
                    faces[closest_face][1].supB * v +
                    faces[closest_face][2].supB * w;

                // Compute a single contact point as the average of the two support points
                Vec3f contactPoint = (worldA + worldB) * 0.5f;
                float penetration = (worldA - worldB).Length();
                Vec3f normal = (worldA - worldB).GetNormalize();

                collisionInfo.AddContactPoint(contactPoint, normal, penetration);
                return;
            }

            Point loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //keep track of edges we need to fix after removing faces
            int num_loose_edges = 0;

            //Find all triangles that are facing p
            for (int i = 0; i < num_faces; i++)
            {
                if (faces[i][3].point.Dot(p.point - faces[i][0].point) > 0) //triangle i faces p, remove it
                {
                    //Add removed triangle's edges to loose edge list.
                    //If it's already there, remove it (both triangles it belonged to are gone)
                    for (int j = 0; j < 3; j++) //Three edges per face
                    {
                        Point current_edge[2] = {faces[i][j], faces[i][(j + 1) % 3]};
                        bool found_edge = false;
                        for (int k = 0; k < num_loose_edges; k++) //Check if current edge is already in list
                        {
                            if (loose_edges[k][1].point == current_edge[0].point && loose_edges[k][0].point ==
                                current_edge[1].point)
                            {
                                loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; //Overwrite current edge
                                loose_edges[k][1] = loose_edges[num_loose_edges - 1][1]; //with last edge in list
                                num_loose_edges--;
                                found_edge = true;
                                k = num_loose_edges; //exit loop because edge can only be shared once
                            }
                        } //endfor loose_edges

                        if (!found_edge)
                        {
                            //add current edge to list
                            // assert(num_loose_edges<EPA_MAX_NUM_LOOSE_EDGES);
                            if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
                            loose_edges[num_loose_edges][0] = current_edge[0];
                            loose_edges[num_loose_edges][1] = current_edge[1];
                            num_loose_edges++;
                        }
                    }

                    //Remove triangle i from list
                    faces[i][0] = faces[num_faces - 1][0];
                    faces[i][1] = faces[num_faces - 1][1];
                    faces[i][2] = faces[num_faces - 1][2];
                    faces[i][3] = faces[num_faces - 1][3];
                    num_faces--;
                    i--;
                } //endif p can see triangle i
            }

            //Reconstruct polytope with p added
            for (int i = 0; i < num_loose_edges; i++)
            {
                // assert(num_faces<EPA_MAX_NUM_FACES);
                if (num_faces >= EPA_MAX_NUM_FACES) break;
                faces[num_faces][0] = loose_edges[i][0];
                faces[num_faces][1] = loose_edges[i][1];
                faces[num_faces][2] = p;
                faces[num_faces][3].point = (loose_edges[i][0].point - loose_edges[i][1].point).Cross(
                    loose_edges[i][0].point - p.point).GetNormalize();

                //Check for wrong normal to maintain CCW winding
                float bias = 0.000001f; //in case dot result is only slightly < 0 (because origin is on face)
                if (faces[num_faces][0].point.Dot(faces[num_faces][3].point) + bias < 0)
                {
                    Point temp = faces[num_faces][0];
                    faces[num_faces][0] = faces[num_faces][1];
                    faces[num_faces][1] = temp;
                    faces[num_faces][3].point = -faces[num_faces][3].point;
                }
                num_faces++;
            }
        }
        PrintLog("EPA did not converge");
        //Return most recent closest point

        Vec3f search_dir = faces[closest_face][3].point;
        Point p = Point(search_dir, coll1, coll2);
        Physic::Plane closestPlane = Physic::Plane::PlaneFromTri(
            faces[closest_face][0].point,
            faces[closest_face][1].point,
            faces[closest_face][2].point);
        Vec3f projectionPoint = closestPlane.ProjectPointOntoPlane(Vec3f::Zero());
        float u, v, w;
        Barycentric(faces[closest_face][0].point,
                    faces[closest_face][1].point,
                    faces[closest_face][2].point,
                    projectionPoint, u, v, w);
        Vec3f worldA = faces[closest_face][0].supA * u +
            faces[closest_face][1].supA * v +
            faces[closest_face][2].supA * w;
        Vec3f worldB = faces[closest_face][0].supB * u +
            faces[closest_face][1].supB * v +
            faces[closest_face][2].supB * w;
        Vec3f contactPoint = (worldA + worldB) * 0.5f;
        float penetration = (worldA - worldB).Length();
        Vec3f normal = (worldA - worldB).GetNormalize();
        collisionInfo.AddContactPoint(contactPoint, normal, penetration);
    }

    // Source : https://github.com/kevinmoran/GJK/blob/master
    bool CustomPhysicsAPI::GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo)
    {
        collisionInfo.a = coll1->GetGameObject();
        collisionInfo.b = coll2->GetGameObject();

        Vec3f coll1Pos = coll1->GetTransform()->GetWorldPosition();
        Vec3f coll2Pos = coll2->GetTransform()->GetWorldPosition();

        Point a, b, c, d;
        Vec3f searchDir = coll1Pos - coll2Pos;

        c.CalculateSupport(searchDir, coll1, coll2);
        searchDir = -c.point;

        b.CalculateSupport(searchDir, coll1, coll2);

        if (b.point.Dot(searchDir) < 0)
            return false; //we didn't reach the origin, won't enclose it

        searchDir = (c.point - b.point).Cross(-b.point).Cross(c.point - b.point);
        if (searchDir == Vec3f::Zero())
        {
            searchDir = (c.point - b.point).Cross(Vec3f::Right());
            if (searchDir == Vec3f::Zero())
                searchDir = (c.point - b.point).Cross(Vec3f::Forward());
        }
        int simp_dim = 2; //simplex dimension

        constexpr int GJK_MAX_NUM_ITERATIONS = 64;
        for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
        {
            a.CalculateSupport(searchDir, coll1, coll2);
            if (a.point.Dot(searchDir) < 0)
                return false; //we didn't reach the origin, won't enclose it

            simp_dim++;
            if (simp_dim == 3)
            {
                UpdateSimplex3(a, b, c, d, simp_dim, searchDir);
            }
            else if (UpdateSimplex4(a, b, c, d, simp_dim, searchDir))
            {
                EPA(a, b, c, d, coll1, coll2, collisionInfo);
                return true;
            }
        }
        return false;
    }

    RigidBodyGroup& CustomPhysicsAPI::AddRigidbodyGroup()
    {
        auto& group = m_rigidBodyGroups.emplace_back();
        group.index = static_cast<int>(m_rigidBodyGroups.size() - 1);

        // std::cout << "Add" << std::endl;
        PrintGroupsState();

        return group;
    }

    void CustomPhysicsAPI::MergeRigidbodyGroup(uint32_t groupAIndex, uint32_t groupBIndex)
    {
        RigidBodyGroup& groupA = m_rigidBodyGroups[groupAIndex];
        RigidBodyGroup& groupB = m_rigidBodyGroups[groupBIndex];

        std::vector<Weak<Component::RigidBody>> rigidbodies = groupB.rigidbodies;
        RemoveRigidbodyGroup(groupB.index);
        for (Weak _rb : rigidbodies)
        {
            Shared<Component::RigidBody> rb = _rb.lock();
            groupA.AddRigidbody(rb);
        }
    }

    void CustomPhysicsAPI::RemoveRigidbodyGroup(uint32_t index)
    {
        RigidBodyGroup& group = m_rigidBodyGroups[index];

        for (auto& _rb : group.rigidbodies)
        {
            auto rb = _rb.lock();
            rb->SetGroup(-1);
        }
        group.rigidbodies.clear();

        m_rigidBodyGroups.erase(m_rigidBodyGroups.begin() + index);
        for (uint32_t i = index; i < m_rigidBodyGroups.size(); i++)
        {
            m_rigidBodyGroups[i].index = i;
            for (auto& _rb : m_rigidBodyGroups[i].rigidbodies)
            {
                auto rb = _rb.lock();
                rb->SetGroup(i);
            }
        }
        // std::cout << "Remove" << std::endl;
        PrintGroupsState();
    }

    void CustomPhysicsAPI::ClearRigidbodyGroup()
    {
        for (auto& group : m_rigidBodyGroups)
        {
            for (auto _rb : group.rigidbodies)
            {
                auto rb = _rb.lock();
                if (!rb)
                    continue;
                rb->SetGroup(-1);
            }
        }
        m_rigidBodyGroups.clear();
    }

    void CustomPhysicsAPI::PrintGroupsState()
    {
        return;
        for (int i = 0; i < m_rigidBodyGroups.size(); i++)
        {
            std::cout << "Group " << i << ": " << std::endl;
            auto rigidbodies = m_rigidBodyGroups[i].rigidbodies;
            for (int j = 0; j < rigidbodies.size(); j++)
            {
                auto rb = rigidbodies[j].lock();
                std::cout << "\t[" << j << "] Group ID: " << rb->GetGroupIndex() << std::endl;
            }
        }
    }
#pragma endregion
}

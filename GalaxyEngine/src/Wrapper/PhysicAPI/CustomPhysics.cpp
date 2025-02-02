#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

#include "Component/BoxCollider.h"
#include "Component/SphereCollider.h"
#include "Component/MeshCollider.h"
#include "Component/Transform.h"

#include "Component/RigidBody.h"
#include "Core/Application.h"

#include "Core/GameObject.h"
#include "Utils/Time.h"

namespace GALAXY
{
    void Wrapper::PhysicAPI::CustomPhysicsAPI::Update()
    {
#ifdef WITH_EDITOR
        if (!Core::Application::IsPlayMode())
            return;
#endif
        float dt = Utils::Time::DeltaTime();

        /*
        for (auto& body : m_objectMap)
        {
            if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
            {
                Component::Transform* transform = rigidbody->GetTransform();
                Vec3f position = transform->GetWorldPosition();
                Quat rotation = transform->GetWorldRotation();
                // body.second.m_position = position;
                // body.second.m_rotation = rotation;
            }
        }
        */

        for (const Weak<Component::RigidBody>& _body : m_objectMap)
        {
            Shared<Component::RigidBody> body = _body.lock();
            Component::Transform* transform = body->GetTransform();

            Vec3f newVelocity = body->GetVelocity() + body->GetGravityForce() * dt;
            Vec3f newPosition = transform->GetWorldPosition() + newVelocity * dt;
            Quat newRotation = transform->GetWorldRotation() * Quat::AngleAxis(
                body->GetAngularVelocity().Length() * dt, body->GetAngularVelocity());
            body->SetVelocity(newVelocity);

            transform->SetWorldPosition(newPosition);
            transform->SetWorldRotation(newRotation);
        }
        /*

        for (auto& body : m_objectMap)
        {
            if (auto rigidbody = dynamic_cast<Component::RigidBody*>(body.first))
            {
                Component::Transform* transform = rigidbody->GetTransform();
                transform->SetWorldPosition(body.second.m_position);
                transform->SetWorldRotation(body.second.m_rotation);
            }
        }
        */
        InternalUpdate();
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateRigidBody(Weak<Component::RigidBody> rigidbody)
    {
        m_objectMap.insert(rigidbody);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyRigidBody(Weak<Component::RigidBody> rigidbody)
    {
        auto object = m_objectMap.find(rigidbody); // Use auto, no reference
        if (object == m_objectMap.end())
        {
            PrintError("Could not find rigidbody associated with component 0x%x !", rigidbody);
            return;
        }
        m_objectMap.erase(object); // Erase using the iterator
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateBoxCollider(Weak<Component::BoxCollider> collider)
    {
        m_colliderMap.insert(collider);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyBoxCollider(Weak<Component::BoxCollider> collider)
    {
        auto object = m_colliderMap.find(collider); // Use auto, no reference
        if (object == m_colliderMap.end())
        {
            PrintError("Could not find collider associated with component 0x%x !", collider);
            return;
        }
        m_colliderMap.erase(object); // Erase using the iterator
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateMeshCollider(Weak<Component::MeshCollider> collider)
    {
        m_colliderMap.insert(collider);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroyMeshCollider(Weak<Component::MeshCollider> collider)
    {
        m_colliderMap.erase(collider);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::CreateSphereCollider(Weak<Component::SphereCollider> collider)
    {
        m_colliderMap.insert(collider);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::DestroySphereCollider(Weak<Component::SphereCollider> collider)
    {
        m_colliderMap.erase(collider);
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::SetDefaultGravity(const Vec3f& value)
    {
        defaultGravity = value;
    }

    bool Wrapper::PhysicAPI::CustomPhysicsAPI::InitializeAPI()
    {
        PrintLog("Custom Physics Initialized");
        return true;
    }

    Wrapper::PhysicAPI::CustomPhysicsAPI::~CustomPhysicsAPI()
    {
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::InternalUpdate()
    {
        std::vector<ColliderPair> objects = BroadPhase();

        for (ColliderPair& pair : objects)
        {
            // PrintLog("Testing pair %ull - %ull", pair.first->GetGameObject()->GetUUID(), pair.second->GetGameObject()->GetUUID());
            if (GJK(pair.first, pair.second))
            {
                pair.first->SetDebugCollide(true);
                pair.second->SetDebugCollide(true);
                // PrintLog("%s collide with %s",
                // pair.first->GetGameObject()->GetName().c_str(),
                // pair.second->GetGameObject()->GetName().c_str());
            }
        }
    }

    struct SAPAABB
    {
        Component::Collider* collider;
        Vec3f Min;
        Vec3f Max;
    };

    std::vector<Wrapper::PhysicAPI::ColliderPair> Wrapper::PhysicAPI::CustomPhysicsAPI::BroadPhase()
    {
        auto colliders = m_colliderMap;
        std::vector<SAPAABB> aabbs;

        // First, gather all AABBs from the colliders
        for (auto& _collider : colliders)
        {
            Shared<Component::Collider> collider = _collider.lock();
            if (collider)
            {
                SAPAABB box;
                Physic::AABB aabb = collider->GetAABB();

                // Assuming SAPAABB has min and max fields for the AABB bounds
                box.Min = aabb.Min;
                box.Max = aabb.Max;
                box.collider = collider.get(); // Store the collider pointer for later reference

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

    // Computes the Minkowski difference support point.
    Vec3f Support(Component::Collider* a, Component::Collider* b, const Vec3f& direction)
    {
        return a->Support(direction) - b->Support(-direction);
    }

    // Updates the simplex and search direction.
    // Returns true if the origin is contained in the simplex.
    bool SimplexContainsOrigin(std::vector<Vec3f>& simplex, Vec3f& direction)
    {
        // Let A be the last added point (at the back).
        const Vec3f A = simplex.back();
        const Vec3f AO = -A;

        // Handle simplex cases based on the number of points.
        if (simplex.size() == 2)
        {
            // --- Line Segment case ---
            // Simplex = {B, A} where A is the newest point.
            const Vec3f B = simplex.front();
            const Vec3f AB = B - A;

            // If the origin is in the AB direction:
            if (AB.Dot(AO) > 0)
            {
                // New direction is perpendicular to AB toward the origin.
                // Using: direction = (AB x AO) x AB.
                direction = AB.Cross(AO).Cross(AB).GetNormalize();
            }
            else
            {
                // Otherwise, the origin is in the direction of A.
                simplex = { A };
                direction = AO.GetNormalize();
            }
            return false;
        }
        else if (simplex.size() == 3)
        {
            // --- Triangle case ---
            // Let the simplex be: C, B, A (A is the newest)
            const Vec3f C = simplex[0];
            const Vec3f B = simplex[1];
            // Edges from A:
            const Vec3f AB = B - A;
            const Vec3f AC = C - A;

            // Compute the triangle's normal (which may not point toward the origin).
            Vec3f ABC = AB.Cross(AC);

            // Compute the perpendicular to AB in the plane of the triangle.
            Vec3f ABPerp = ABC.Cross(AB);
            if (ABPerp.Dot(AO) > 0)
            {
                // Remove C; new simplex is {B, A}
                simplex.erase(simplex.begin());
                direction = ABPerp.GetNormalize();
                return false;
            }

            // Compute the perpendicular to AC in the plane.
            Vec3f ACPerp = AC.Cross(ABC);
            if (ACPerp.Dot(AO) > 0)
            {
                // Remove B; new simplex is {C, A}
                simplex.erase(simplex.begin() + 1);
                direction = ACPerp.GetNormalize();
                return false;
            }

            // If neither region works, then the origin must be in the direction of the triangle's normal.
            // Ensure the normal points toward the origin.
            if (ABC.Dot(AO) > 0)
                direction = ABC.GetNormalize();
            else
                direction = (-ABC).GetNormalize();
            return false;
        }
        else if (simplex.size() == 4)
        {
            // --- Tetrahedron case ---
            // Let the simplex be: D, C, B, A (A is the newest)
            const Vec3f D = simplex[0];
            const Vec3f C = simplex[1];
            const Vec3f B = simplex[2];
            // A is simplex[3]
            // Edges from A:
            const Vec3f AB = B - A;
            const Vec3f AC = C - A;
            const Vec3f AD = D - A;

            // Compute normals for each face of the tetrahedron that contains A.
            Vec3f ABC = AB.Cross(AC);
            if (ABC.Dot(AO) > 0)
            {
                // The origin is outside face ABC.
                simplex = { C, B, A }; // Order: {A (newest), B, C}
                direction = ABC.GetNormalize();
                return false;
            }

            Vec3f ACD = AC.Cross(AD);
            if (ACD.Dot(AO) > 0)
            {
                // The origin is outside face ACD.
                simplex = { D, C, A }; // Order: {A, C, D}
                direction = ACD.GetNormalize();
                return false;
            }

            Vec3f ADB = AD.Cross(AB);
            if (ADB.Dot(AO) > 0)
            {
                // The origin is outside face ADB.
                simplex = { B, D, A }; // Order: {A, D, B}
                direction = ADB.GetNormalize();
                return false;
            }

            // If the origin is not outside any face, then it is inside the tetrahedron.
            return true;
        }

        // Fallback (should never reach here)
        return false;
    }

    bool Wrapper::PhysicAPI::CustomPhysicsAPI::GJK(Component::Collider* a, Component::Collider* b)
    {
        // Initialize the search direction from a's position to b's position.
        Vec3f direction = b->GetTransform()->GetWorldPosition() - a->GetTransform()->GetWorldPosition();
        if (direction.LengthSquared() == 0)
            direction = Vec3f(1, 1, 1); // Fallback direction if both centers coincide.

        // Start with the first support point.
        std::vector<Vec3f> simplex;
        simplex.push_back(Support(a, b, direction));

        // New direction toward the origin.
        direction = -simplex.back();
        
        if (direction.LengthSquared() == 0)
            direction = Vec3f(1, 1, 1); // Fallback direction if both centers coincide.

        constexpr int maxIterations = 100;
        constexpr float epsilon = 1e-6f;

        for (int i = 0; i < maxIterations; ++i)
        {
            Vec3f newSupport = Support(a, b, direction);

            // If the new support point isn't past the origin in the direction, no collision.
            if (newSupport.Dot(direction) < -epsilon)
                return false;

            simplex.push_back(newSupport);
            
            Renderer::GetInstance()->DrawWireCube(newSupport, Vec3f(0.1f));

            if (SimplexContainsOrigin(simplex, direction))
                return true;
        }
        return false;
    }

}

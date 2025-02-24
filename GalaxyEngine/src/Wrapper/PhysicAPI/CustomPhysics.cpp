#include "pch.h"
#include "Wrapper/PhysicAPI/CustomPhysics.h"

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
        size_t operator()(const Vec3f& v) const
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
    
    bool CustomPhysicsAPI::InitializeAPI()
    {
        m_dTOffset = 0.f;
        m_staticCountMax = 25;
        m_staticMaxPosMagn = 0.01f;
        PrintLog("Custom Physics Initialized");
        return true;
    }

    void CustomPhysicsAPI::InternalUpdate()
    {
        m_collisionInfos.clear();
        std::vector<ColliderPair> objects = BroadPhase();

        for (ColliderPair& pair : objects)
        {
            Vec3f mtv = Vec3f::Zero();
            CollisionInfo info;
            // PrintLog("Testing pair %ull - %ull", pair.first->GetGameObject()->GetUUID(), pair.second->GetGameObject()->GetUUID());
            if (GJK(pair.first, pair.second, info))
            {
                pair.first->SetDebugCollide(true);
                pair.second->SetDebugCollide(true);

                m_collisionInfos.push_back(info);

                ResolveCollisions(pair.first, pair.second, info);
            }
        }
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

            if (accel.Length() > 10 && inverseMass > 0.f)
            {
                rigidBody->StaticPositionCount = 0;
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
        // Retrieve rigid bodies from both colliders.
        Shared<Component::RigidBody> bodyA = collider1->GetGameObject()->GetComponent<Component::RigidBody>();
        Shared<Component::RigidBody> bodyB = collider2->GetGameObject()->GetComponent<Component::RigidBody>();


        // Not work, why ?
        // if (bodyA && !bodyA->IsEnable())
            // bodyA = nullptr;

        // if (bodyB && !bodyB->IsEnable())
            // bodyB = nullptr;

        if (!bodyA && !bodyB)
            return;

        // Get transforms (assumes each GameObject provides a GetTransform() method).
        auto transformA = collider1->GetGameObject()->GetTransform();
        auto transformB = collider2->GetGameObject()->GetTransform();

        float aInvMass = bodyA ? bodyA->GetInverseMass() : 0;
        float bInvMass = bodyB ? bodyB->GetInverseMass() : 0;

        Vec3f aVelocity = bodyA ? bodyA->GetVelocity() : Vec3f::Zero();
        Vec3f bVelocity = bodyB ? bodyB->GetVelocity() : Vec3f::Zero();
        
        auto inverseInertiaTensorA = bodyA ? bodyA->GetInverseInertiaTensor() : Mat4::Identity();
        auto inverseInertiaTensorB = bodyB ? bodyB->GetInverseInertiaTensor() : Mat4::Identity();
        
        float totalMass = aInvMass + bInvMass;
        if (totalMass == 0)
            return; // two static objects collided

        auto p = collisionInfo.point;
        
        // ----- Positional Correction -----
        if (bodyA)
        {
            transformA->SetWorldPosition(transformA->GetWorldPosition() -
                (p.normal * p.depth * (aInvMass / totalMass)));
        }
        if (bodyB)
        {
            transformB->SetWorldPosition(transformB->GetWorldPosition() +
                (p.normal * p.depth * (bInvMass / totalMass)));
        }

        // ----- Impulse Resolution -----
        Vec3f relativeA = p.localA;
        Vec3f relativeB = p.localB;

        Vec3f angVelocityA =
            angVelocityA.Cross(relativeA);
        Vec3f angVelocityB =
            angVelocityB.Cross(relativeB);

        Vec3f fullVelocityA = aVelocity + angVelocityA;
        Vec3f fullVelocityB = bVelocity + angVelocityB;

        Vec3f contactVelocity = fullVelocityB - fullVelocityA;

        float impulseForce = contactVelocity.Dot(p.normal);

        // now to work out the effect of inertia ....
        Vec3f inertiaA = inverseInertiaTensorA.MultiplyVector(relativeA.Cross(p.normal)).Cross(relativeA); //?
        Vec3f inertiaB = inverseInertiaTensorB.MultiplyVector(relativeB.Cross(p.normal)).Cross(relativeB); //?
        float angularEffect = (inertiaA + inertiaB).Dot(p.normal);

        //float cRestitution = 0.66f; // disperse some kinetic energy
        float cRestitution = collider1->GetRestitution() + collider2->GetRestitution();

        float j = (-(1.0f + cRestitution) * impulseForce) /
            (totalMass + angularEffect);

        Vec3f fullImpulse = p.normal * j;

        if (bodyA)
        {
            bodyA->SetVelocity(bodyA->GetVelocity() + (-fullImpulse * aInvMass));
            bodyA->SetAngularVelocity(bodyA->GetAngularVelocity() + inverseInertiaTensorA.MultiplyPoint3x4(relativeA.Cross(-fullImpulse)));
        }
        if (bodyB)
        {
            bodyB->SetVelocity(bodyB->GetVelocity() + (fullImpulse * bInvMass));
            bodyB->SetAngularVelocity(bodyB->GetAngularVelocity() + inverseInertiaTensorB.MultiplyPoint3x4(relativeB.Cross(fullImpulse)));
        }        
    }

    void CustomPhysicsAPI::UpdateConstraints(float constraintDt)
    {
        
    }

    void CustomPhysicsAPI::IntegrateVelocity(float dt) const
    {
        float frameLinearDamping = 1.0f - (0.4f * dt);
        for (auto& _body : m_objectSet)
        {
            auto body = _body.lock();
            
            if (!body || !body->IsEnable())
                continue;
            Component::Transform* transform = body->GetTransform();

            Vec3f position = transform->GetWorldPosition();
            Vec3f velocity = body->GetVelocity();

            Vec3f dPosition = velocity * dt;
            float mag_position = (velocity * dt).Length();

            if (dPosition.Length() < m_staticMaxPosMagn)
            {
                body->StaticPositionCount++;
            }

            if (body->StaticPositionCount < m_staticCountMax)
            {
                position += velocity * dt;

                transform->SetWorldPosition(position);

                velocity = velocity * frameLinearDamping;
                body->SetVelocity(velocity);

                Quat rotation = transform->GetWorldRotation();
                Vec3f angularVelocity = body->GetAngularVelocity();

                Vec3f dAngle = angularVelocity * dt;

                Vec3f a = dAngle * 0.5f;

                rotation = rotation + (Quat(a.x, a.y, a.z, 0) * rotation);
                rotation.Normalize();

                transform->SetWorldRotation(rotation);

                float frameAngularDamping = 1.0f - (0.4f * dt);
                angularVelocity = angularVelocity * frameAngularDamping;
                body->SetAngularVelocity(angularVelocity);
            }
            else
            {
                body->StaticPositionCount = m_staticCountMax;
                body->SetVelocity(Vec3f::Zero());
            }
            
        }
        
    }

    //This is the fixed timestep we'd LIKE to have
    const int   idealHZ = 120;
    const float idealDT = 1.0f / idealHZ;

    /*
    This is the fixed update we actually have...
    If physics takes too long it starts to kill the framerate, it'll drop the 
    iteration count down until the FPS stabilises, even if that ends up
    being at a low rate. 
    */
    int realHZ		= idealHZ;
    float realDT	= idealDT;
    int constraintIterationCount = 1;
    
    void CustomPhysicsAPI::Update()
    {
#ifdef WITH_EDITOR
        if (!Core::Application::IsPlayMode())
            return;
#endif
        float dt = Utils::Time::DeltaTime();

        // Remove expired objects
        std::erase_if(m_objectSet, [](const Weak<Component::RigidBody>& body) { return body.expired(); });
        std::erase_if(m_colliderSet, [](const Weak<Component::Collider>& collider) { return collider.expired(); });

        //TODO: Update AABB for broadphases

        Utils::ElapsedTimer timer;
        m_dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!
        while (m_dTOffset >= realDT)
        {
            IntegrateAccel(realDT);
            std::vector<ColliderPair> objects = BroadPhase();

            for (auto& object : objects)
            {
                CollisionInfo info;
                if (GJK(object.first, object.second, info))
                {
                    object.first->SetDebugCollide(true);
                    object.second->SetDebugCollide(false);
                    
                    info.framesLeft = m_numCollisionFrames;
                    ResolveCollisions(object.first, object.second, info);
                    m_collisionInfos.push_back(info);
                }
            }

            float constraintDt = realDT /  (float)constraintIterationCount;
            for (int i = 0; i < constraintIterationCount; ++i) {
                UpdateConstraints(constraintDt);	
            }
            IntegrateVelocity(realDT); //update positions from new velocity changes

            m_dTOffset -= realDT;
        }
        m_collisionInfos.clear();

        timer.Stop();
        float updateTime = static_cast<float>(timer.GetElapsedTime().AsSeconds());

        //Uh oh, physics is taking too long...
        if (updateTime > realDT) {
            realHZ /= 2;
            realDT *= 2;
            PrintLog("Dropping iteration count due to long physics time...(now %d)", realHZ);
        }
        else if(dt*2 < realDT) { //we have plenty of room to increase iteration count!
            int temp = realHZ;
            realHZ *= 2;
            realDT /= 2;

            if (realHZ > idealHZ) {
                realHZ = idealHZ;
                realDT = idealDT;
            }
            if (temp != realHZ) {
                PrintLog("Raising iteration count due to short physics time...(now %d)", realHZ);
            }
        }

        /*
        for (const Weak<Component::RigidBody>& _body : m_objectSet)
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

            body->UpdateInertiaTensor();
        }
        */
    }

    void CustomPhysicsAPI::DrawDebug()
    {
        auto instance = Renderer::GetInstance();
        for (const CollisionInfo& info : m_collisionInfos)
        {
            instance->DrawSimpleWireSphere(info.point.localA, 0.1f, 32, Vec4f(1, 0, 0, 1), 10.f);
            instance->DrawSimpleWireSphere(info.point.localB, 0.1f, 32, Vec4f(0, 0, 1, 1), 10.f);
            instance->DrawLine(info.point.localA, info.point.localA + info.point.normal * info.point.depth,
                               Vec4f(1, 0, 0, 1), 10.f);
            instance->DrawLine(info.point.localB, info.point.localB + info.point.normal * info.point.depth,
                               Vec4f(0, 0, 1, 1), 10.f);
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

    std::vector<Vec3f> CustomPhysicsAPI::ComputeConvexHull(const std::vector<Vec3f>& positions)
    {
        std::vector<Vec3f> convexVertices;
        struct Face
        {
            Vec3f a, b, c;
            Vec3f normal;
            float distance;
            std::vector<Vec3f> outsidePoints;

            Face(const Vec3f& a, const Vec3f& b, const Vec3f& c) : a(a), b(b), c(c)
            {
                Vec3f ab = b - a;
                Vec3f ac = c - a;
                normal = ab.Cross(ac).GetNormalize();
                distance = normal.Dot(a);
            }

            float distanceTo(const Vec3f& p) const
            {
                return normal.Dot(p) - distance;
            }
        };

        // Find initial tetrahedron vertices
        Vec3f A = positions[0];
        for (const Vec3f& p : positions)
            if (p.x > A.x) A = p;

        Vec3f B = A;
        float maxDistSq = 0.0f;
        for (const Vec3f& p : positions)
        {
            float distSq = (p - A).LengthSquared();
            if (distSq > maxDistSq)
            {
                maxDistSq = distSq;
                B = p;
            }
        }

        Vec3f C;
        float maxLineDistSq = 0.0f;
        Vec3f AB = B - A;
        for (const Vec3f& p : positions)
        {
            Vec3f AP = p - A;
            float t = AP.Dot(AB) / AB.LengthSquared();
            t = std::max(0.0f, std::min(1.0f, t));
            Vec3f proj = A + AB * t;
            float distSq = (p - proj).LengthSquared();
            if (distSq > maxLineDistSq)
            {
                maxLineDistSq = distSq;
                C = p;
            }
        }

        Vec3f normal = (B - A).Cross(C - A).GetNormalize();
        float planeDist = normal.Dot(A);
        Vec3f D;
        float maxPlaneDist = 0.0f;
        for (const Vec3f& p : positions)
        {
            float dist = std::abs(normal.Dot(p) - planeDist);
            if (dist > maxPlaneDist)
            {
                maxPlaneDist = dist;
                D = p;
            }
        }

        if (normal.Dot(D) < planeDist)
            normal = -normal;

        std::vector<Face> faces;
        faces.emplace_back(A, B, C);
        faces.emplace_back(A, C, D);
        faces.emplace_back(A, D, B);
        faces.emplace_back(B, D, C);

        std::vector<Vec3f> remainingPoints;
        for (const Vec3f& p : positions)
        {
            if (p != A && p != B && p != C && p != D)
                remainingPoints.push_back(p);
        }

        // Assign outside points to initial faces
        for (Face& face : faces)
        {
            for (auto it = remainingPoints.begin(); it != remainingPoints.end();)
            {
                if (face.distanceTo(*it) > 1e-6f)
                {
                    face.outsidePoints.push_back(*it);
                    it = remainingPoints.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        std::vector<Face> activeFaces = faces;
        while (!activeFaces.empty())
        {
            Face face = activeFaces.back();
            activeFaces.pop_back();

            if (face.outsidePoints.empty())
                continue;

            // Find the furthest point from the face
            Vec3f p = face.outsidePoints[0];
            float maxDist = face.distanceTo(p);
            for (const Vec3f& q : face.outsidePoints)
            {
                float dist = face.distanceTo(q);
                if (dist > maxDist)
                {
                    maxDist = dist;
                    p = q;
                }
            }

            // Create new faces (triangles) with the furthest point
            std::vector<Face> newFaces;
            newFaces.emplace_back(face.a, face.b, p);
            newFaces.emplace_back(face.b, face.c, p);
            newFaces.emplace_back(face.c, face.a, p);

            for (Face& newFace : newFaces)
            {
                for (auto it = face.outsidePoints.begin(); it != face.outsidePoints.end();)
                {
                    if (*it == p)
                    {
                        ++it;
                        continue;
                    }
                    if (newFace.distanceTo(*it) > 1e-6f)
                    {
                        newFace.outsidePoints.push_back(*it);
                        it = face.outsidePoints.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                activeFaces.push_back(newFace);
                faces.push_back(newFace);
            }
        }

        // Instead of gathering unique vertices, we now output triangle vertices in the proper order.
        // Each face (triangle) is added as three consecutive vertices.
        std::vector<Vec3f> sortedVertices;
        for (const Face& face : faces)
        {
            sortedVertices.push_back(face.a);
            sortedVertices.push_back(face.b);
            sortedVertices.push_back(face.c);
        }

        return sortedVertices;
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

    bool CustomPhysicsAPI::Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance, Physic::RaycastHit& hit)
    {
        // TODO : Test this
        bool hitFound = false;
        float closestDistance = maxDistance;
        // Ensure the direction is normalized.
        Vec3f dir = direction.GetNormalize();

        // Lambda to perform a ray-AABB intersection using the slab method.
        auto RayAABBIntersect = [&](const Vec3f& rayOrigin, const Vec3f& rayDir, 
                                    const Vec3f& boxMin, const Vec3f& boxMax, float& tOut) -> bool {
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
                Physic::Plane closestPlane = Physic::Plane::PlaneFromTri(
                    faces[closest_face][0].point, faces[closest_face][1].point,
                    faces[closest_face][2].point); //plane of closest triangle face
                Vec3f projectionPoint = closestPlane.ProjectPointOntoPlane(Vec3f::Zero());
                //projecting the origin onto the triangle(both are in Minkowski space)
                float u, v, w;
                Barycentric(faces[closest_face][0].point, faces[closest_face][1].point, faces[closest_face][2].point,
                            projectionPoint, u, v,
                            w); //finding the barycentric coordinate of this projection point to the triangle

                //The contact points just have the same barycentric coordinate in their own triangles which  are composed by result coordinates of support function 
                Vec3f localA = faces[closest_face][0].supA * u + faces[closest_face][1].supA * v + faces[closest_face][
                    2].supA * w;
                Vec3f localB = faces[closest_face][0].supB * u + faces[closest_face][1].supB * v + faces[closest_face][
                    2].supB * w;
                float penetration = (localA - localB).Length();
                Vec3f normal = (localA - localB).GetNormalize();

                //Convergence (new point is not significantly further from origin)
                localA -= coll1->GetTransform()->GetWorldPosition();
                localB -= coll2->GetTransform()->GetWorldPosition();

                collisionInfo.AddContactPoint(localA, localB, normal, penetration);
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

        Physic::Plane closestPlane = Physic::Plane::PlaneFromTri(faces[closest_face][0].point,
                                                                 faces[closest_face][1].point,
                                                                 faces[closest_face][2].point);
        Vec3f projectionPoint = closestPlane.ProjectPointOntoPlane(Vec3f::Zero());
        float u, v, w;
        Barycentric(faces[closest_face][0].point, faces[closest_face][1].point, faces[closest_face][2].point,
                    projectionPoint, u, v, w);
        Vec3f localA = faces[closest_face][0].supA * u + faces[closest_face][1].supA * v + faces[closest_face][2].supA *
            w;
        Vec3f localB = faces[closest_face][0].supB * u + faces[closest_face][1].supB * v + faces[closest_face][2].supB *
            w;
        float penetration = (localA - localB).Length();
        Vec3f normal = (localA - localB).GetNormalize();

        collisionInfo.AddContactPoint(localA, localB, normal, penetration);
    }

    // Source : https://github.com/kevinmoran/GJK/blob/master
    bool CustomPhysicsAPI::GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo)
    {
        collisionInfo.a = coll1->GetGameObject();
        collisionInfo.b = coll2->GetGameObject();

        Vec3f* mtv = nullptr;

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
#pragma endregion 
}

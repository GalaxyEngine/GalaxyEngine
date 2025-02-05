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
#include "Resource/Mesh.h"
#include "Utils/Time.h"
    
// Define custom hash function for Vec3f
namespace std {
    template <>
    struct hash<Vec3f> {
        size_t operator()(const Vec3f& v) const {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
        }
    };
}

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

    Weak<Resource::Mesh> Wrapper::PhysicAPI::CustomPhysicsAPI::GetConvexMesh(Shared<Resource::Mesh> mesh)
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

    std::vector<Vec3f> Wrapper::PhysicAPI::CustomPhysicsAPI::ComputeConvexHull(const std::vector<Vec3f>& positions)
    {
        std::vector<Vec3f> convexVertices;
        struct Face {
            Vec3f a, b, c;
            Vec3f normal;
            float distance;
            std::vector<Vec3f> outsidePoints;

            Face(const Vec3f& a, const Vec3f& b, const Vec3f& c) : a(a), b(b), c(c) {
                Vec3f ab = b - a;
                Vec3f ac = c - a;
                normal = ab.Cross(ac).GetNormalize();
                distance = normal.Dot(a);
            }

            float distanceTo(const Vec3f& p) const {
                return normal.Dot(p) - distance;
            }
        };

        // Find initial tetrahedron vertices
        Vec3f A = positions[0];
        for (const Vec3f& p : positions)
            if (p.x > A.x) A = p;

        Vec3f B = A;
        float maxDistSq = 0.0f;
        for (const Vec3f& p : positions) {
            float distSq = (p - A).LengthSquared();
            if (distSq > maxDistSq) {
                maxDistSq = distSq;
                B = p;
            }
        }

        Vec3f C;
        float maxLineDistSq = 0.0f;
        Vec3f AB = B - A;
        for (const Vec3f& p : positions) {
            Vec3f AP = p - A;
            float t = AP.Dot(AB) / AB.LengthSquared();
            t = std::max(0.0f, std::min(1.0f, t));
            Vec3f proj = A + AB * t;
            float distSq = (p - proj).LengthSquared();
            if (distSq > maxLineDistSq) {
                maxLineDistSq = distSq;
                C = p;
            }
        }

        Vec3f normal = (B - A).Cross(C - A).GetNormalize();
        float planeDist = normal.Dot(A);
        Vec3f D;
        float maxPlaneDist = 0.0f;
        for (const Vec3f& p : positions) {
            float dist = std::abs(normal.Dot(p) - planeDist);
            if (dist > maxPlaneDist) {
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
        for (const Vec3f& p : positions) {
            if (p != A && p != B && p != C && p != D)
                remainingPoints.push_back(p);
        }

        // Assign outside points to initial faces
        for (Face& face : faces) {
            for (auto it = remainingPoints.begin(); it != remainingPoints.end();) {
                if (face.distanceTo(*it) > 1e-6f) {
                    face.outsidePoints.push_back(*it);
                    it = remainingPoints.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::vector<Face> activeFaces = faces;
        while (!activeFaces.empty()) {
            Face face = activeFaces.back();
            activeFaces.pop_back();

            if (face.outsidePoints.empty())
                continue;

            // Find the furthest point from the face
            Vec3f p = face.outsidePoints[0];
            float maxDist = face.distanceTo(p);
            for (const Vec3f& q : face.outsidePoints) {
                float dist = face.distanceTo(q);
                if (dist > maxDist) {
                    maxDist = dist;
                    p = q;
                }
            }

            // Create new faces (triangles) with the furthest point
            std::vector<Face> newFaces;
            newFaces.emplace_back(face.a, face.b, p);
            newFaces.emplace_back(face.b, face.c, p);
            newFaces.emplace_back(face.c, face.a, p);

            for (Face& newFace : newFaces) {
                for (auto it = face.outsidePoints.begin(); it != face.outsidePoints.end();) {
                    if (*it == p) {
                        ++it;
                        continue;
                    }
                    if (newFace.distanceTo(*it) > 1e-6f) {
                        newFace.outsidePoints.push_back(*it);
                        it = face.outsidePoints.erase(it);
                    } else {
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
        for (const Face& face : faces) {
            sortedVertices.push_back(face.a);
            sortedVertices.push_back(face.b);
            sortedVertices.push_back(face.c);
        }

        return sortedVertices;
    }

    void Wrapper::PhysicAPI::CustomPhysicsAPI::ComputeConvexVertices(Shared<Resource::Mesh> mesh)
    {
        Shared<Resource::Mesh> convexMesh = m_convexMesh[mesh->GetUUID()];
        if (!mesh || (convexMesh && convexMesh->HasBeenSent()))
            return;

        std::vector<Vec3f> positions = mesh->GetPositionVertices();
        std::vector<Vec3f> convexVertices;

        if (positions.empty()) {
            convexMesh->SetMeshPosition(convexVertices);
            return;
        }
        
        convexVertices = ComputeConvexHull(positions);

        convexMesh->SetMeshPosition(convexVertices);
        PrintLog("Convex mesh created for %s", mesh->GetMeshName().c_str());
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

    std::vector<Wrapper::PhysicAPI::ColliderPair> Wrapper::PhysicAPI::CustomPhysicsAPI::BroadPhase() const
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
    
    //Triangle case
    void update_simplex3(Vec3f &a, Vec3f &b, Vec3f &c, Vec3f &d, int &simp_dim, Vec3f &search_dir){
        /* Required winding order:
        //  b
        //  | \
        //  |   \
        //  |    a
        //  |   /
        //  | /
        //  c
        */
        Vec3f n = (b-a).Cross(c - a); //triangle's normal
        Vec3f AO = -a; //direction to origin

        //Determine which feature is closest to origin, make that the new simplex

        simp_dim = 2;
        if((b-a).Cross(n).Dot(AO)>0){ //Closest to edge AB
            c = a;
            //simp_dim = 2;
            search_dir = (b-a).Cross(AO).Cross(b-a);
            return;
        }
        if(n.Cross(c-a).Dot(AO)>0){ //Closest to edge AC
            b = a;
            //simp_dim = 2;
            search_dir = (c-a).Cross(AO).Cross(c-a);
            return;
        }
        
        simp_dim = 3;
        if(n.Dot(AO)>0){ //Above triangle
            d = c;
            c = b;
            b = a;
            //simp_dim = 3;
            search_dir = n;
            return;
        }
        //else //Below triangle
        d = b;
        b = a;
        //simp_dim = 3;
        search_dir = -n;
    }

    //Tetrahedral case
    bool update_simplex4(Vec3f &a, Vec3f &b, Vec3f &c, Vec3f &d, int &simp_dim, Vec3f &search_dir){
        // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
	    //We know a priori that origin is above BCD and below a

        //Get normals of three new faces
        Vec3f ABC = (b-a).Cross(c-a);
        Vec3f ACD = (c-a).Cross(d-a);
        Vec3f ADB = (d-a).Cross(b-a);

        Vec3f AO = -a; //dir to origin
        simp_dim = 3; //hoisting this just cause

        //Plane-test origin with 3 faces
        /*
        // Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
        // We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this 
        // is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like 
        // this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
        */
        if(ABC.Dot(AO)>0){ //In front of ABC
    	    d = c;
    	    c = b;
    	    b = a;
            search_dir = ABC;
    	    return false;
        }
        if(ACD.Dot(AO)>0){ //In front of ACD
    	    b = a;
            search_dir = ACD;
    	    return false;
        }
        if(ADB.Dot(AO)>0){ //In front of ADB
    	    c = d;
    	    d = b;
    	    b = a;
            search_dir = ADB;
    	    return false;
        }

        //else inside tetrahedron; enclosed!
        return true;
    }

    // Source : https://github.com/kevinmoran/GJK/blob/master
    bool Wrapper::PhysicAPI::CustomPhysicsAPI::GJK(Component::Collider* coll1, Component::Collider* coll2)
    {
        Vec3f a,b,c,d;
        Vec3f bWorldPos = coll1->GetTransform()->GetWorldPosition();
        Vec3f aWorldPos = coll2->GetTransform()->GetWorldPosition();

        Vec3f searchDir = bWorldPos - aWorldPos;

        c = coll2->Support(searchDir) - coll1->Support(-searchDir);
        searchDir = -c;

        b = coll2->Support(searchDir) - coll1->Support(-searchDir);

        if (b.Dot(searchDir) < 0)
            return false;

        searchDir = (c - b).Cross(-b).Cross(c - b);
        if (searchDir == Vec3f::Zero())
        {
            searchDir = (c - b).Cross(Vec3f::Right());
            if (searchDir == Vec3f::Zero())
                searchDir = (c - b).Cross(Vec3f::Forward());
        }
        int simp_dim = 2; //simplex dimension

        constexpr int GJK_MAX_NUM_ITERATIONS = 64;
        for(int iterations=0; iterations<GJK_MAX_NUM_ITERATIONS; iterations++)
        {
            a = coll2->Support(searchDir) - coll1->Support(-searchDir);
            if(a.Dot(searchDir)<0)
                return false; //we didn't reach the origin, won't enclose it
    
            simp_dim++;
            if(simp_dim==3){
                update_simplex3(a,b,c,d,simp_dim,searchDir);
            }
            else if(update_simplex4(a,b,c,d,simp_dim,searchDir))
            {
                //TODO
                // if(mtv) *mtv = EPA(a,b,c,d,coll1,coll2);
                return true;
            }
        }
        return false;
    }

}

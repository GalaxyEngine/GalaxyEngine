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

#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
    
// Define custom hash function for Vec3f
namespace std {
    template <>
    struct hash<Vec3f> {
        size_t operator()(const Vec3f& v) const {
            return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1) ^ (hash<float>()(v.z) << 2);
        }
    };
}
using namespace Wrapper::PhysicAPI;
namespace GALAXY
{

    bool CustomPhysicsAPI::InitializeAPI()
    {
        PrintLog("Custom Physics Initialized");
        return true;
    }

    CustomPhysicsAPI::~CustomPhysicsAPI()
    {
    }

    void CustomPhysicsAPI::InternalUpdate()
    {
        std::vector<ColliderPair> objects = BroadPhase();

        for (ColliderPair& pair : objects)
        {
            Vec3f mtv = Vec3f::Zero();
            CollisionPoints points;
            // PrintLog("Testing pair %ull - %ull", pair.first->GetGameObject()->GetUUID(), pair.second->GetGameObject()->GetUUID());
            if (GJK(pair.first, pair.second, points))
            {
                pair.first->SetDebugCollide(true);
                pair.second->SetDebugCollide(true);
                

                ResolveCollisions(pair.first, pair.second, points);
            }
        }
    }
    void CustomPhysicsAPI::ResolveCollisions(Component::Collider* collider1,
                                           Component::Collider* collider2,
                                           const CollisionPoints& collisionPoints)
    {
        auto instance = Renderer::GetInstance();
        m_prevPoints = collisionPoints;
        // Loop through each contact point provided by EPA/GJK.
        for (const CollisionPoint& contact : collisionPoints)
        {
            // Retrieve the rigidbody components if they exist.
            Shared<Component::RigidBody> body1 = collider1->GetGameObject()->GetComponent<Component::RigidBody>();
            Shared<Component::RigidBody> body2 = collider2->GetGameObject()->GetComponent<Component::RigidBody>();
            instance->DrawSimpleWireSphere(contact.point, 0.1f, 32, Vec4f(1, 0, 0, 1), 10.f);
            float depth = contact.normal.Length();
            Vec3f normal = contact.normal.GetNormalize();
            instance->DrawLine(contact.point, contact.point + normal * depth, Vec4f(1, 0, 0, 1), 10.f);
            if (body1)
            {
                // body1->SetGravityForce(Vec3f::Zero());
                // body1->SetVelocity(Vec3f::Zero());
            }
            else if (body2)
            {
                // body2->SetGravityForce(Vec3f::Zero());
                // body2->SetVelocity(Vec3f::Zero());
            }
            continue;
        }
    }

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

    void CustomPhysicsAPI::DrawDebug()
    {
        auto instance = Renderer::GetInstance();
        for (const CollisionPoint& contact : m_prevPoints)
        {
            // instance->DrawSimpleWireSphere(contact.point, 0.1f, 32, Vec4f(1, 0, 0, 1), 10.f);
            // instance->DrawLine(contact.point, contact.point + contact.normal * contact.depth, Vec4f(1, 0, 0, 1), 10.f);
        
            return;
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

#pragma region GJK
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

    void CustomPhysicsAPI::ComputeConvexVertices(Shared<Resource::Mesh> mesh)
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
#pragma endregion 

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
    void UpdateSimplex3(const Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, int& simpDim, Vec3f& searchDir)
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
        Vec3f n = (b - a).Cross(c - a); // Triangle's normal
        Vec3f AO = -a; // Direction to origin

        // Determine which feature is closest to origin, make that the new simplex

        simpDim = 2;
        if ((b - a).Cross(n).Dot(AO) > 0) // Closest to edge AB
        {
            c = a;
            //simp_dim = 2;
            searchDir = (b-a).Cross(AO).Cross(b-a);
            return;
        }
        if (n.Cross(c - a).Dot(AO) > 0) // Closest to edge AC
        {
            b = a;
            //simp_dim = 2;
            searchDir = (c - a).Cross(AO).Cross(c - a);
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
    bool UpdateSimplex4(const Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, int& simpDim, Vec3f& searchDir)
    {
        // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
	    // We know a priori that origin is above BCD and below a

        // Get normals of three new faces
        Vec3f ABC = (b - a).Cross(c - a);
        Vec3f ACD = (c - a).Cross(d - a);
        Vec3f ADB = (d - a).Cross(b - a);

        Vec3f AO = -a; // dir to origin
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

    // Expanding Polytope Algorithm
    // Find minimum translation vector to resolve collision
    CollisionPoints CustomPhysicsAPI::EPA(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d, Component::Collider* coll1, Component::Collider* coll2)
    {
        Vec3f faces[4 * EPA_MAX_NUM_FACES]; // Array of faces, each with 3 verts and a normal
        
        //Init with final simplex from GJK
        faces[0]  = a;
        faces[1]  = b;
        faces[2]  = c;
        faces[3]  = (b-a).Cross(c-a).GetNormalize(); //ABC
        faces[4]  = a;
        faces[5]  = c;
        faces[6]  = d;
        faces[7]  = (c-a).Cross(d-a).GetNormalize(); //ACD
        faces[8]  = a;
        faces[9]  = d;
        faces[10] = b;
        faces[11] = (d-a).Cross(b-a).GetNormalize(); //ADB
        faces[12] = b;
        faces[13] = d;
        faces[14] = c;
        faces[15] = (d-b).Cross(c-b).GetNormalize(); //BDC

        int num_faces = 4;
        int closest_face;

        for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++)
        {
            // Find face that's closest to origin
            float min_dist = faces[0].Dot(faces[3]);
            closest_face = 0;
            for (int i=1; i<num_faces; i++)
            {
                float dist = faces[i * 4].Dot(faces[i * 4 + 3]);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    closest_face = i;
                }
            }

            // Search normal to face that's closest to origin
            Vec3f search_dir = faces[closest_face * 4 + 3]; 
            Vec3f p = coll2->Support(search_dir) - coll1->Support(-search_dir);

            if (p.Dot(search_dir) - min_dist < EPA_TOLERANCE)
            {
                // Convergence (new point is not significantly further from origin)
                return {CollisionPoint(faces[closest_face * 4 + 3] * p.Dot(search_dir))}; // dot vertex with normal to resolve collision along normal!
            }

            Vec3f loose_edges[4 * EPA_MAX_NUM_LOOSE_EDGES + 2]; //keep track of edges we need to fix after removing faces
            int num_loose_edges = 0;

            //Find all triangles that are facing p
            for (int i = 0; i < num_faces; i++)
            {
                if (faces[i * 4 + 3].Dot(p - faces[i * 4]) > 0) // triangle i faces p, remove it
                {
                    // Add removed triangle's edges to loose edge list.
                    // If it's already there, remove it (both triangles it belonged to are gone)
                    for (int j = 0; j < 3; j++) // Three edges per face
                    {
                        Vec3f current_edge[2] = { faces[i * 4 + j], faces[i * 4 + (j + 1) % 3] };
                        bool found_edge = false;
                        for (int k = 0; k < num_loose_edges; k++) // Check if current edge is already in list
                        {
                            if (loose_edges[k * 4 + 1] == current_edge[0] && loose_edges[k * 4] == current_edge[1])
                            {
                                // Edge is already in the list, remove it
                                // THIS ASSUMES EDGE CAN ONLY BE SHARED BY 2 TRIANGLES (which should be true)
                                // THIS ALSO ASSUMES SHARED EDGE WILL BE REVERSED IN THE TRIANGLES (which 
                                // should be true provided every triangle is wound CCW)
                                loose_edges[k * 4]     = loose_edges[(num_loose_edges-1) * 4]; // Overwrite current edge
                                loose_edges[k * 4 + 1] = loose_edges[(num_loose_edges-1) * 4 + 1]; // with last edge in list
                                num_loose_edges--;
                                found_edge = true;
                                k = num_loose_edges; // exit loop because edge can only be shared once
                            }
                        }

                        if (!found_edge) // add current edge to list
                        {
                            // assert(num_loose_edges < EPA_MAX_NUM_LOOSE_EDGES);
                            if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES)
                                break;

                            loose_edges[num_loose_edges * 4]     = current_edge[0];
                            loose_edges[num_loose_edges * 4 + 1] = current_edge[1];
                            num_loose_edges++;
                        }
                    }

                    // Remove triangle i from list
                    faces[i * 4]     = faces[(num_faces-1) * 4];
                    faces[i * 4 + 1] = faces[(num_faces-1) * 4 + 1];
                    faces[i * 4 + 2] = faces[(num_faces-1) * 4 + 2];
                    faces[i * 4 + 3] = faces[(num_faces-1) * 4 + 3];
                    num_faces--;
                    i--;
                }
            }
            
            //Reconstruct polytope with p added
            for (int i = 0; i < num_loose_edges; i++)
            {
                // assert(num_faces<EPA_MAX_NUM_FACES);
                if (num_faces >= EPA_MAX_NUM_FACES)
                    break;
                faces[num_faces * 4] = loose_edges[i * 4];
                faces[num_faces * 4 + 1] = loose_edges[i * 4 + 1];
                faces[num_faces * 4 + 2] = p;
                faces[num_faces * 4 + 3] = (loose_edges[i * 4]-loose_edges[i * 4 + 1]).Cross(loose_edges[i * 4] - p).GetNormalize();

                // Check for wrong normal to maintain CCW winding
                float bias = 0.000001f; //in case dot result is only slightly < 0 (because origin is on face)
                if (faces[num_faces * 4].Dot(faces[num_faces * 4 + 3]) + bias < 0)
                {
                    Vec3f temp = faces[num_faces * 4];
                    faces[num_faces * 4]     = faces[num_faces * 4 + 1];
                    faces[num_faces * 4 + 1] = temp;
                    faces[num_faces * 4 + 3] = -faces[num_faces * 4 + 3];
                }
                num_faces++;
            }
        }
        PrintLog("EPA did not converge");
        //Return most recent closest point
        return {CollisionPoint(faces[closest_face * 4 + 3] * faces[closest_face * 4].Dot(faces[closest_face * 4 + 3]))};
    }

    // Source : https://github.com/kevinmoran/GJK/blob/master
    bool CustomPhysicsAPI::GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionPoints& collisionPoints)
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
                UpdateSimplex3(a,b,c,d,simp_dim,searchDir);
            }
            else if(UpdateSimplex4(a,b,c,d,simp_dim,searchDir))
            {
                collisionPoints = EPA(a,b,c,d,coll1,coll2);
                return true;
            }
        }
        return false;
    }

}

#pragma once

#include "GalaxyAPI.h"
#include "Wrapper/PhysicsWrapper.h"
#include <list>
namespace GALAXY::Resource
{
    class Mesh;
}

namespace GALAXY::Component
{
    class MeshCollider;
}

namespace GALAXY
{
    namespace Wrapper::PhysicAPI
    {
        
        struct ColliderPair
        {
            Component::Collider* first;
            Component::Collider* second;
        };
        
        struct CollisionPoint
        {
            CollisionPoint() {}
            CollisionPoint(Vec3f _point, Vec3f _normal, float _depth) : point(_point), normal(_normal), depth(_depth) {}
            CollisionPoint(Vec3f mtv, Vec3f _point = Vec3f::Zero())
            : point(_point), normal(mtv.GetNormalize()), depth(mtv.Length()) {}
            
            Vec3f point;
            Vec3f normal;
            float depth = 0.f;
        };

        struct EPAVertex {
            Vec3f point;
            Vec3f supA;   
            Vec3f supB;   
        };

        struct EPAFace {
            EPAVertex v[3];
            Vec3f normal;  
            float distance;
        };

        typedef std::vector<CollisionPoint> CollisionPoints;
        
        class CustomPhysicsAPI : public Wrapper::PhysicsWrapper
        {
        public:
            CustomPhysicsAPI() = default;
            ~CustomPhysicsAPI();

            void Update() override;
            void DrawDebug() override;

            void CreateRigidBody(Weak<Component::RigidBody> rigidbody) override;
            void DestroyRigidBody(Weak<Component::RigidBody> rigidbody) override;
            void CreateBoxCollider(Weak<Component::BoxCollider> collider) override;
            void DestroyBoxCollider(Weak<Component::BoxCollider> collider) override;
            void CreateSphereCollider(Weak<Component::SphereCollider> collider) override;
            void DestroySphereCollider(Weak<Component::SphereCollider> collider) override;
            void CreateMeshCollider(Weak<Component::MeshCollider> collider) override;
            void DestroyMeshCollider(Weak<Component::MeshCollider> collider) override;
            void SetDefaultGravity(const Vec3f& value) override;

            void AddForce(Weak<Component::RigidBody> rigidbody, const Vec3f& force) override;

            Weak<Resource::Mesh> GetConvexMesh(Shared<Resource::Mesh> mesh) override;
            static std::vector<Vec3f> ComputeConvexHull(const std::vector<Vec3f>& positions);
            void ComputeConvexVertices(Shared<Resource::Mesh> mesh) override;
        private:
            bool InitializeAPI() override;
            void InternalUpdate();
            void ResolveCollisions(Component::Collider* collider1, Component::Collider* collider2, const CollisionPoints& collisionPoints);
            
            std::vector<ColliderPair> BroadPhase() const;
            static CollisionPoints EPA(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d,
                                            Component::Collider* coll1, Component::Collider* coll2);
            static bool GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionPoints& collisionPoints);

        private:
            std::set<Weak<Component::RigidBody>, WeakPtrCompare<Component::RigidBody>> m_objectSet;
            std::set<Weak<Component::Collider>, WeakPtrCompare<Component::Collider>> m_colliderSet;
            Vec3f m_defaultGravity = Vec3f(0.f, -9.81f, 0.f);

            CollisionPoints m_prevPoints;

            std::unordered_map<Core::UUID, Shared<Resource::Mesh>> m_convexMesh; // Convex mesh with mesh as key
        };
    }
}

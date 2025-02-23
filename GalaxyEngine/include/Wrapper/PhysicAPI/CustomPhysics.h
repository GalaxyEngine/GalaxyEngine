#pragma once

#include "GalaxyAPI.h"
#include "Wrapper/PhysicsWrapper.h"

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
        
        struct ContactPoint
        {
            ContactPoint() {}
            
            Vec3f localA;
            Vec3f localB;
            Vec3f normal;
            float depth = 0.f;
        };

        struct CollisionInfo
        {
            Core::GameObject* a;
            Core::GameObject* b;
            ContactPoint point;
            int framesLeft;

            void AddContactPoint(const Vec3f& localA, const Vec3f& localB, const Vec3f& normal, float depth)
            {
                point.localA = localA;
                point.localB = localB;
                point.normal = normal;
                point.depth = depth;
            }
        };

        struct Point {
            Vec3f point;
            Vec3f supA;   
            Vec3f supB;

            Point() = default;
            Point(const Vec3f& searchDir, Component::Collider* a, Component::Collider* b);

            void CalculateSupport(const Vec3f& searchDir, Component::Collider* a, Component::Collider* b);
        };

        typedef std::vector<ContactPoint> CollisionPoints;
        
        class GALAXY_API CustomPhysicsAPI : public Wrapper::PhysicsWrapper
        {
        public:
            CustomPhysicsAPI() = default;
            ~CustomPhysicsAPI() = default;

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
            void AddForceAtPosition(const Weak<Component::RigidBody>& weak, const Vec3f& force, const Vec3f& position) override;
            void AddTorque(const Weak<Component::RigidBody>& weak, const Vec3f& torque) override;

            Weak<Resource::Mesh> GetConvexMesh(Shared<Resource::Mesh> mesh) override;
            static std::vector<Vec3f> ComputeConvexHull(const std::vector<Vec3f>& positions);
            void ComputeConvexVertices(Shared<Resource::Mesh> mesh) override;

            bool Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance, Physic::RaycastHit& hit) override;
        private:
            bool InitializeAPI() override;
            void InternalUpdate();
            void IntegrateAccel(float dt) const;
            void ResolveCollisions(Component::Collider* collider1, Component::Collider* collider2, const CollisionInfo& collisionInfo);
            void UpdateConstraints(float constraintDt);
            void IntegrateVelocity(float dt) const;
            
            std::vector<ColliderPair> BroadPhase() const;
            static void EPA(const Point& a, const Point& b, const Point& c, const Point& d,
                            Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo);
            static bool GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo);

        private:
            std::set<Weak<Component::RigidBody>, WeakPtrCompare<Component::RigidBody>> m_objectSet;
            std::set<Weak<Component::Collider>, WeakPtrCompare<Component::Collider>> m_colliderSet;
            Vec3f m_defaultGravity = Vec3f(0.f, -9.81f, 0.f);

            std::vector<CollisionInfo> m_collisionInfos;
			float m_dTOffset = 0.f;
			int m_numCollisionFrames = 5;
			float m_staticMaxPosMagn = 0.f;
            int m_staticCountMax = 0;

            std::unordered_map<Core::UUID, Shared<Resource::Mesh>> m_convexMesh; // Convex mesh with mesh as key
        };
    }
}

#pragma once

#include "GalaxyAPI.h"
#include "Physic/AABB.h"
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

            Vec3f position;
            Vec3f normal;
            float depth = 0.f;
        };

        struct CollisionInfo
        {
            Core::GameObject* a;
            Core::GameObject* b;
            ContactPoint point;
            int framesLeft;

            void AddContactPoint(const Vec3f& position, const Vec3f& normal, float depth)
            {
                if (isnan(position.x) || isnan(normal.x))
                {
                    DebugBreak();
                }
                point.position = position;
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

        struct RigidBodyGroup
        {
            std::vector<Weak<Component::RigidBody>> rigidbodies;
            int staticCount = 0;

            int index = -1;

            void AddRigidbody(Shared<Component::RigidBody> rb);

            bool IsSleeping() const;

            void WakeUp() const;

            Physic::AABB GetAABB() const;
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
            void WakeUpRigidBody(Component::RigidBody* rigidbody) override;

            Weak<Resource::Mesh> GetConvexMesh(Shared<Resource::Mesh> mesh) override;
            static std::vector<Vec3f> ComputeConvexHull(const std::vector<Vec3f>& meshPositionsVert);
            void ComputeConvexVertices(Shared<Resource::Mesh> mesh) override;

            bool Raycast(const Vec3f& origin, const Vec3f& direction, float maxDistance, Physic::RaycastHit& hit) override;
        private:
            bool InitializeAPI() override;
            void IntegrateAccel(float dt) const;
            void ResolveCollisions(Component::Collider* collider1, Component::Collider* collider2, const CollisionInfo& collisionInfo);
            void UpdateConstraints(float constraintDt);
            void IntegrateVelocity(float dt);
            
            std::vector<ColliderPair> BroadPhase() const;
            static void EPA(const Point& a, const Point& b, const Point& c, const Point& d,
                            Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo);
            static bool GJK(Component::Collider* coll1, Component::Collider* coll2, CollisionInfo& collisionInfo);

            RigidBodyGroup& AddRigidbodyGroup();
            void MergeRigidbodyGroup(uint32_t groupAIndex, uint32_t groupBIndex);
            void RemoveRigidbodyGroup(uint32_t index);
            RigidBodyGroup& GetRigidbodyGroup(uint32_t index) { return m_rigidBodyGroups[index];}
            void ClearRigidbodyGroup();

            void PrintGroupsState();
        private:
            std::set<Weak<Component::RigidBody>, WeakPtrCompare<Component::RigidBody>> m_objectSet;
            std::set<Weak<Component::Collider>, WeakPtrCompare<Component::Collider>> m_colliderSet;

            std::vector<RigidBodyGroup> m_rigidBodyGroups;
            
            Vec3f m_defaultGravity = Vec3f(0.f, -9.81f, 0.f);

            std::vector<CollisionInfo> m_collisionInfos;
			float m_dTOffset = 0.f;
			int m_numCollisionFrames = 5;
            float m_staticCountMax = 1.5f;

            std::unordered_map<Core::UUID, Shared<Resource::Mesh>> m_convexMesh; // Convex mesh with mesh as key
        };
    }
}

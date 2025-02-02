#pragma once

#include "GalaxyAPI.h"
#include "Wrapper/PhysicsWrapper.h"
#include <list>

namespace GALAXY::Component
{
    class MeshCollider;
}

namespace GALAXY
{
    namespace Wrapper::PhysicAPI
    {
        class InternalRigidbody
        {
        public:
            InternalRigidbody() = default;
            ~InternalRigidbody() = default;

            Quat m_rotation;
            Vec3f m_position;
            Vec3f m_velocity;
            Vec3f m_gravityForce;
            Vec3f m_omega;
            bool m_static = false;
        };

        class InternalCollider
        {
        public:
            InternalCollider() = default;
            ~InternalCollider() = default;

            Quat m_rotation;
            Vec3f m_position;
            Vec3f m_velocity;
            Vec3f m_gravityForce;
            Vec3f m_omega;
            bool m_static = false;
        };

        struct ColliderPair
        {
            Component::Collider* first;
            Component::Collider* second;
        };
        
        class CustomPhysicsAPI : public Wrapper::PhysicsWrapper
        {
        public:
            CustomPhysicsAPI() = default;
            ~CustomPhysicsAPI();

            void Update() override;

            void CreateRigidBody(Weak<Component::RigidBody> rigidbody) override;
            void DestroyRigidBody(Weak<Component::RigidBody> rigidbody) override;
            void CreateBoxCollider(Weak<Component::BoxCollider> collider) override;
            void DestroyBoxCollider(Weak<Component::BoxCollider> collider) override;
            void CreateSphereCollider(Weak<Component::SphereCollider> collider) override;
            void DestroySphereCollider(Weak<Component::SphereCollider> collider) override;
            void CreateMeshCollider(Weak<Component::MeshCollider> collider) override;
            void DestroyMeshCollider(Weak<Component::MeshCollider> collider) override;
            void SetDefaultGravity(const Vec3f& value) override;

        private:
            bool InitializeAPI() override;
            void InternalUpdate();
            std::vector<ColliderPair> BroadPhase();
            static bool GJK(Component::Collider* a, Component::Collider* b);

        private:
            std::set<Weak<Component::RigidBody>, WeakPtrCompare<Component::RigidBody>> m_objectMap;
            std::set<Weak<Component::Collider>, WeakPtrCompare<Component::Collider>> m_colliderMap;
            Vec3f defaultGravity = Vec3f(0.f, -9.81f, 0.f);
        };
    }
}

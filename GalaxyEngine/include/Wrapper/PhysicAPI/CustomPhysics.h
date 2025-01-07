#pragma once

#include "GalaxyAPI.h"
#include "Wrapper\PhysicsWrapper.h"

namespace GALAXY
{
    namespace Wrapper::PhysicAPI
    {
        class InternalRigidBody
        {
        public:
            InternalRigidBody() = default;
            ~InternalRigidBody() = default;

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

        class CustomPhysicsAPI : public Wrapper::PhysicsWrapper
        {
        public:
            CustomPhysicsAPI() = default;
            ~CustomPhysicsAPI();

            void Update() override;

            void CreateRigidBody(Component::RigidBody* rigidbody) override;
            void DestroyRigidBody(Component::RigidBody* rigidbody) override;
            void CreateBoxCollider(Component::BoxCollider* collider) override;
            void DestroyBoxCollider(Component::BoxCollider* collider) override;
            void CreateSphereCollider(Component::SphereCollider* collider) override;
            void DestroySphereCollider(Component::SphereCollider* collider) override;
            void SetDefaultGravity(const Vec3f& value) override;

        private:
            bool InitializeAPI() override;
            void InternalUpdate();
            std::list<std::tuple<Component::BaseComponent*>> BroadPhase();

        private:
            std::unordered_map<Component::BaseComponent*, InternalRigidBody> m_objectMap;
            std::unordered_map<Component::BaseComponent*, InternalCollider> m_colliderMap;
            Vec3f defaultGravity = Vec3f(0,-9.81,0);
        };
    }
}

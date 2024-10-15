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
            bool m_static = false;
        };

        class CustomPhysicsAPI : public Wrapper::PhysicsWrapper
        {
        public:
            CustomPhysicsAPI() = default;
            ~CustomPhysicsAPI();

            void Update() override;

            void CreateRigidbody(Component::Rigidbody* rigidbody) override;
            void DestroyRigidbody(Component::Rigidbody* rigidbody) override;
            void CreateBoxCollider(Component::BoxCollider* collider) override;
            void DestroyBoxCollider(Component::BoxCollider* collider) override;
            void CreateSphereCollider(Component::SphereCollider* collider) override;
            void DestroySphereCollider(Component::SphereCollider* collider) override;
            void SetDefaultGravity(const Vec3f& value) override;

        private:
            bool InitializeAPI() override;
            void InternalUpdate();

        private:
            std::unordered_map<Component::BaseComponent*, InternalRigidBody> m_objectMap;
            Vec3f defaultGravity = Vec3f(0,-1,0);
        };
    }
}

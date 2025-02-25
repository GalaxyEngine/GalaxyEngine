#pragma once
#include "GalaxyAPI.h"
#include "Component/IComponent.h"

namespace GALAXY 
{
    namespace Component
    {
        class GALAXY_API RigidBody : public IComponent<RigidBody>, public std::enable_shared_from_this<RigidBody>
        {
        public:
            RigidBody() = default;
            RigidBody& operator=(const RigidBody& other) = default;
            RigidBody(const RigidBody&) = default;
            RigidBody(RigidBody&&) noexcept = default;
            ~RigidBody() override = default;

            const char* GetComponentName() const override { return "RigidBody"; }

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
#endif

            void Serialize(CppSer::Serializer& serializer) override;
            void Deserialize(CppSer::Parser& parser) override;

            float GetMass() const { return m_mass; }
            void SetMass(float mass) { m_mass = mass; }

            float GetInverseMass() const { return m_mass == 0.0f ? 0.0f : 1.0f / m_mass; }
            void SetInverseMass(float inverseMass) { m_mass = 1.0f / inverseMass; }
            
            float GetDrag() const { return m_drag; }
            void SetDrag(float drag) { m_drag = drag; }
            
            float GetAngularDrag() const { return m_angularDrag; }
            void SetAngularDrag(float angularDrag) { m_angularDrag = angularDrag; }

            void SetVelocity(const Vec3f& velocity) { m_velocity = velocity; }
            Vec3f GetVelocity() const { return m_velocity; }

            void SetAngularVelocity(const Vec3f& angularVelocity) { m_angularVelocity = angularVelocity; }
            Vec3f GetAngularVelocity() const { return m_angularVelocity; }

            void SetGravityForce(const Vec3f& gravityForce) { m_gravityForce = gravityForce; }
            Vec3f GetGravityForce() const { return m_gravityForce; }
            
            Vec3f GetForce() const { return m_force; }
            Vec3f GetTorque() const { return m_torque;}
            
            
            void OnStart() override;
            void OnDestroy() override;

            // ================ Physics ================ //
            void AddForce(const Vec3f& force);
            void AddForceAtPosition(const Vec3f& force, const Vec3f& position);

            void AddTorque(const Vec3f& torque);
            
            Mat4 GetInertiaTensor() const;
            Mat4 GetInverseInertiaTensor() const;

            void UpdateInertiaTensor();
        public:
            int StaticPositionCount = 0;
        private:
            float m_mass = 1.0f;
            float m_drag = 0.4f;
            float m_angularDrag = 0.4f;
            Vec3f m_gravityForce = Vec3f::Up() * -9.81f;

            Vec3f m_velocity;
            Vec3f m_angularVelocity;

            Mat4 m_inverseInertiaTensorLocal = Mat4::Identity();

            Vec3f m_force;
            Vec3f m_torque;
        };
    }
}

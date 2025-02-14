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
            
            void OnStart() override;
            void OnDestroy() override;

            void AddForce(const Vec3f& force);

        private:
            
            float m_mass = 1.0f;
            float m_drag = 0.0f;
            float m_angularDrag = 0.0f;

            Vec3f m_velocity;
            Vec3f m_angularVelocity;

            Vec3f m_gravityForce = Vec3f::Up() * -9.81f;
        };
    }
}

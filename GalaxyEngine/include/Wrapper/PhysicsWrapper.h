#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Component
    {
        class BoxCollider;
        class SphereCollider;
        class CapsuleCollider;
        class WheelCollider;
        class RigidBody;
        class Collider;
    }
    namespace Wrapper
    {
        enum class PhysicAPIType
        {
            Jolt,
            PhysX,
            Custom
        };

        
        class PhysicsWrapper
        {
        public:
            virtual ~PhysicsWrapper() = default;

            static PhysicsWrapper* GetInstance() { return m_instance; }

            static void Initialize(PhysicAPIType type);

            static void Release();

            virtual void Update() = 0;

            virtual void CreateRigidBody(Weak<Component::RigidBody> body) = 0;
            virtual void DestroyRigidBody(Weak<Component::RigidBody> body) = 0;
            virtual void CreateBoxCollider(Weak<Component::BoxCollider> collider) = 0;
            virtual void DestroyBoxCollider(Weak<Component::BoxCollider> collider) = 0;
            virtual void CreateSphereCollider(Weak<Component::SphereCollider> collider) = 0;
            virtual void DestroySphereCollider(Weak<Component::SphereCollider> collider) = 0;

            virtual void SetDefaultGravity(const Vec3f& value) = 0;
        protected:
            void AddDynamicBody(uint32_t id, Weak<Component::RigidBody> body);
            void RemoveDynamicBody(uint32_t id);
            
            virtual bool InitializeAPI() = 0;
        private:
            static PhysicsWrapper* m_instance;

            
        };
    }
    
}

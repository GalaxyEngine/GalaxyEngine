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
        class Rigidbody;
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

            virtual void CreateRigidbody(Component::Rigidbody* body) = 0;
            virtual void DestroyRigidbody(Component::Rigidbody* body) = 0;
            virtual void CreateBoxCollider(Component::BoxCollider* collider) = 0;
            virtual void DestroyBoxCollider(Component::BoxCollider* collider) = 0;
            virtual void CreateSphereCollider(Component::SphereCollider* collider) = 0;
            virtual void DestroySphereCollider(Component::SphereCollider* collider) = 0;

            virtual void SetDefaultGravity(const Vec3f& value) = 0;
        protected:
            void AddDynamicBody(uint32_t id, Weak<Component::Rigidbody> body);
            void RemoveDynamicBody(uint32_t id);
            
            virtual bool InitializeAPI() = 0;
        private:
            static PhysicsWrapper* m_instance;

            
        };
    }
    
}

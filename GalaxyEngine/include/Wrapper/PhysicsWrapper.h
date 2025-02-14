#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
    namespace Resource
    {
        class Mesh;
    }

    namespace Component
    {
        class MeshCollider;
        class BoxCollider;
        class SphereCollider;
        class CapsuleCollider;
        class WheelCollider;
        class RigidBody;
        class Collider;
    }
    namespace Wrapper
    {
        template <typename T>
        struct WeakHash {
            std::size_t operator()(const Weak<T>& weakPtr) const {
                auto sp = weakPtr.lock(); // Convert to shared_ptr
                return std::hash<T*>{}(sp.get()); // Hash raw pointer
            }
        };

        template <typename T>
        struct WeakEqual {
            bool operator()(const Weak<T>& lhs, const Weak<T>& rhs) const {
                return lhs.lock() == rhs.lock(); // Compare the raw pointers
            }
        };

        template <typename T>
        struct WeakPtrCompare {
            bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const {
                return lhs.owner_before(rhs);
            }
        };
        
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
            virtual void CreateMeshCollider(Weak<Component::MeshCollider> collider) = 0;
            virtual void DestroyMeshCollider(Weak<Component::MeshCollider> collider) = 0;

            virtual void AddForce(Weak<Component::RigidBody> rigidbody, const Vec3f& force) {};

            virtual void SetDefaultGravity(const Vec3f& value) = 0;
            virtual Weak<Resource::Mesh> GetConvexMesh(Shared<Resource::Mesh> mesh) {return {};}
            virtual void ComputeConvexVertices(Shared<Resource::Mesh> mesh) {}
        protected:
            void AddDynamicBody(uint32_t id, Weak<Component::RigidBody> body);
            void RemoveDynamicBody(uint32_t id);
            
            virtual bool InitializeAPI() = 0;
        private:
            static PhysicsWrapper* m_instance;

            
        };
    }
    
}

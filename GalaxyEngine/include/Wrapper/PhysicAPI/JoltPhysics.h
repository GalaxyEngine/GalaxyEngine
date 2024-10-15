#pragma once
#include "GalaxyAPI.h"
#include "Wrapper\PhysicsWrapper.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsStepListener.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

namespace GALAXY
{
    namespace Wrapper::PhysicAPI
    {
        // Layer that objects can be in, determines which other objects it can collide with
        // Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
        // layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
        // but only if you do collision testing).
        namespace Layers
        {
            static constexpr uint8_t NON_MOVING = 0;
            static constexpr uint8_t MOVING = 1;
            static constexpr uint8_t NUM_LAYERS = 2;
        };

        /// Class that determines if two object layers can collide
        class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
        {
        public:
            virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
            {
                switch (inObject1)
                {
                case Layers::NON_MOVING:
                    return inObject2 == Layers::MOVING; // Non moving only collides with moving
                case Layers::MOVING:
                    return true; // Moving collides with everything
                default:
                    JPH_ASSERT(false);
                    return false;
                }
            }
        };

        // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
        // a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
        // You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
        // many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
        // your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
        namespace BroadPhaseLayers
        {
            static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
            static constexpr JPH::BroadPhaseLayer MOVING(1);
            static constexpr JPH::uint NUM_LAYERS(2);
        };

        // BroadPhaseLayerInterface implementation
        // This defines a mapping between object and broadphase layers.
        class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
        {
        public:
            BPLayerInterfaceImpl()
            {
                // Create a mapping table from object to broad phase layer
                mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            virtual JPH::uint GetNumBroadPhaseLayers() const override
            {
                return BroadPhaseLayers::NUM_LAYERS;
            }

            virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
            {
                JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
                return mObjectToBroadPhase[inLayer];
            }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
            {
                switch ((JPH::BroadPhaseLayer::Type)inLayer)
                {
                case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
                case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
                default: JPH_ASSERT(false);
                    return "INVALID";
                }
            }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

        private:
            JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
        };

        /// Class that determines if an object layer can collide with a broadphase layer
        class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
        {
        public:
            virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
            {
                switch (inLayer1)
                {
                case Layers::NON_MOVING:
                    return inLayer2 == BroadPhaseLayers::MOVING;
                case Layers::MOVING:
                    return true;
                default:
                    JPH_ASSERT(false);
                    return false;
                }
            }
        };

        // An example contact listener
        /*class MyContactListener : public ContactListener
        {
        public:
            // See: ContactListener
            virtual ValidateResult  OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
            {
                cout << "Contact validate callback" << endl;
    
                // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
                return ValidateResult::AcceptAllContactsForThisBodyPair;
            }
    
            virtual void OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
            {
                cout << "A contact was added" << endl;
            }
    
            virtual void OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
            {
                cout << "A contact was persisted" << endl;
            }
    
            virtual void OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
            {
                cout << "A contact was removed" << endl;
            }
        };*/

        using DynamicBodyMap = std::unordered_map<Component::BaseComponent*, JPH::Body*>;
        using StaticBodyMap = std::unordered_map<Component::BaseComponent*, JPH::Body*>;

        class JoltAPI : public Wrapper::PhysicsWrapper
        {
        public:
            JoltAPI() = default;
            ~JoltAPI();

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

            static inline JPH::Vec3 FromVec3(const Vec3f& inVec) { return JPH::Vec3(inVec.x, inVec.y, inVec.z); }

            static inline Vec3f ToVec3(const JPH::Vec3& inVec)
            {
                return Vec3f(inVec.GetX(), inVec.GetY(), inVec.GetZ());
            }

            static inline JPH::Quat FromQuat(const Quat& inQuat)
            {
                return JPH::Quat(inQuat.x, inQuat.y, inQuat.z, inQuat.w);
            }

            static inline Quat ToQuat(const JPH::Quat& inQuat)
            {
                return Quat(inQuat.GetX(), inQuat.GetY(), inQuat.GetZ(), inQuat.GetW());
            }

        private:
            JPH::PhysicsSystem* m_physicsSystem = nullptr;

            JPH::JobSystem* m_jobSystem = nullptr;
            JPH::TempAllocator* m_tempAllocator = nullptr;

            BPLayerInterfaceImpl m_broadPhaseLayerInterface;
            // The broadphase layer interface that maps object layers to broadphase layers
            ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
            // Class that filters object vs broadphase layers
            ObjectLayerPairFilterImpl m_objectVsObjectLayerFilter; // Class that filters object vs object layers

            DynamicBodyMap m_dynamicBodies;
            StaticBodyMap m_staticBodies;
        };
    }
}

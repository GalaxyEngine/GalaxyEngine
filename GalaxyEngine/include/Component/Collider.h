#pragma once
#include "GalaxyAPI.h"
#include "IComponent.h"
#include "Physic/AABB.h"

namespace GALAXY 
{
    namespace Component
    {
        class RigidBody;

        enum ColliderType
        {
            Sphere = 0,
            Box,
            Capsule,
            Mesh,

            Count
        };

        class GALAXY_API Collider : public IComponent<Collider>
        {
        public:

            const char* GetComponentName() const override { return "Collider"; }

            Quat m_offsetRot;
            Vec3f m_offsetPos;

            virtual ColliderType GetType() const;

#ifdef WITH_EDITOR
            EDITOR_ONLY void ShowInInspector() override;
            
            EDITOR_ONLY void OnEditorDraw() override;
#endif

            void OnUpdate() override;

            virtual Physic::AABB GetAABB() { return {}; }
            virtual Vec3f Support(const Vec3f& direction);

            void SetDebugCollide(bool value) { p_debugCollide = value; }
            bool GetDebugCollide() const { return p_debugCollide; }
        protected:
            Weak<RigidBody> p_attachedRigidbody;

            bool p_debugCollide = false;
            Vec3f p_supportDirection = Vec3f(1, 0, 0);
            Vec3f p_supportPosition;

            bool p_drawAABB = false;
        };
    }
}

#include "pch.h"
#include "Component/RigidBody.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
#ifdef WITH_EDITOR
    void Component::RigidBody::ShowInInspector()
    {
    }
#endif

    void Component::RigidBody::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateRigidBody(this);
    }

    void Component::RigidBody::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyRigidBody(this);
    }
}

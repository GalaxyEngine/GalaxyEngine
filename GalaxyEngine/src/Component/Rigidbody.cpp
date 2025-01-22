#include "pch.h"
#include "Component/RigidBody.h"

#include "Wrapper/PhysicsWrapper.h"

namespace GALAXY 
{
    void Component::RigidBody::ShowInInspector()
    {
#ifdef WITH_EDITOR
#endif
    }

    void Component::RigidBody::OnStart()
    {
        Wrapper::PhysicsWrapper::GetInstance()->CreateRigidBody(this);
    }

    void Component::RigidBody::OnDestroy()
    {
        Wrapper::PhysicsWrapper::GetInstance()->DestroyRigidBody(this);
    }
}
